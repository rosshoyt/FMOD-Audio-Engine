/// 
/// @file AudioEngine.cpp
///
#include "AudioEngine.h"
#include <FMOD/fmod_errors.h>
#include <iostream>

AudioEngine::AudioEngine() : soundCache(), channelMap(), soundBanks(), eventDescriptions(), eventInstances() {
    ERRCHECK(FMOD::Studio::System::create(&studioSystem));
    ERRCHECK(studioSystem->getCoreSystem(&lowLevelSystem));
    ERRCHECK(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0));
    ERRCHECK(lowLevelSystem->set3DSettings(1.0, DISTANCEFACTOR, 1.0f));
    ERRCHECK(studioSystem->initialize(MAX_AUDIO_CHANNELS, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
}

void AudioEngine::update() {
    ERRCHECK(studioSystem->update()); // also updates the low level system
}

void AudioEngine::loadSoundFile(const char* filepath, bool loop) {
    if (!soundIsCached(filepath)) {
        std::cout << "Audio Engine: Loading Sound File " << filepath << "\n";
        FMOD::Sound* sound;
        ERRCHECK(lowLevelSystem->createSound(filepath, FMOD_2D, 0, &sound));
        ERRCHECK(sound->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));
        soundCache.insert({ filepath,  sound });
    }
    else 
        std::cout << "Audio Engine: Sound File was already loaded!\n";
}

void AudioEngine::playSoundFile(const char* filepath) {
    if (soundIsCached(filepath)) {
        // channel for sound to play on
        FMOD::Channel* channel;
        ERRCHECK(lowLevelSystem->playSound(getSound(filepath), 0, false, &channel));
    }
    else
        std::cout << "AudioEngine: Trying to play a sound that wasn't loaded!\n";
}

void AudioEngine::load3DSoundFile(const char* filepath, bool loop) {
    if (!soundIsCached(filepath)) {
        std::cout << "Loading 3D Sound File " << filepath << "\n";
        FMOD::Sound* sound;
        ERRCHECK(lowLevelSystem->createSound(filepath, FMOD_3D, 0, &sound));
        // TODO allow user to set custom 3D Min Max Distance
        ERRCHECK(sound->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR));
        ERRCHECK(sound->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));
        // TODO create separate cache for 3D sounds
        soundCache.insert({ filepath,  sound });
    }
    else
        std::cout << "AudioEngine: 3D Sound File was already loaded!\n";
}

void AudioEngine::play3DSoundFile(const char* filepath, float x, float y, float z) {
    if (soundIsCached(filepath)) {
        FMOD_VECTOR position = { x * DISTANCEFACTOR, y, z };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
        FMOD::Channel* channel;
        lowLevelSystem->playSound(getSound(filepath), 0, true, &channel);
        channel->set3DAttributes(&position, &velocity);
        channel->setPaused(false);
    }
    else
        std::cout << "AudioEngine: Trying to play a 3DSound that wasn't loaded!\n";
}

void AudioEngine::update3DSoundPosition(const char* filepath, float x, float y, float z) {
    if (channelMap.count(filepath) > 0) {
        FMOD::Channel* channel = channelMap[filepath];
        FMOD_VECTOR position = { x * DISTANCEFACTOR, y * DISTANCEFACTOR, z * DISTANCEFACTOR };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
        ERRCHECK(channel->set3DAttributes(&position, &velocity));
    }
    else
        std::cout << "AudioEngine: Trying to update 3D Sound Position for a sound that wasn't loaded!\n";
}

void AudioEngine::set3DListenerPosition(float posX, float posY, float posZ, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ) {
    listenerpos = { posX,     posY,     posZ };
    forward = { forwardY, forwardX, forwardZ };
    up = { upY,      upX,      upZ };
    ERRCHECK(lowLevelSystem->set3DListenerAttributes(0, &listenerpos, 0, &forward, &up));
}

void AudioEngine::loadFMODStudioBank(const char* filepath) {
    std::cout << "AudioEngine: Loading FMOD Studio Sound Bank " << filepath << '\n';
    FMOD::Studio::Bank* bank = NULL;
    ERRCHECK(studioSystem->loadBankFile(filepath, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
    soundBanks.insert({ filepath, bank });
}

void AudioEngine::loadFMODStudioEvent(const char* eventName, std::vector<std::pair<const char*, float>> paramsValues) { // std::vector<std::map<const char*, float>> perInstanceParameterValues) {
    std::cout << "AudioEngine: Loading FMOD Studio Event " << eventName << '\n';
    FMOD::Studio::EventDescription* eventDescription = NULL;
    ERRCHECK(studioSystem->getEvent(eventName, &eventDescription));
    // Create an instance of the event
    FMOD::Studio::EventInstance* eventInstance = NULL;
    ERRCHECK(eventDescription->createInstance(&eventInstance));
    // DEBUG TODO remove
    printEventInfo(eventDescription);
    for (const auto& parVal : paramsValues) {
        std::cout << "Setting Event Instance Parameter " << parVal.first << "to value: " << parVal.second << '\n';
        // Set the parameter values of the event instance
        ERRCHECK(eventInstance->setParameterByName(parVal.first, parVal.second));
    }
    eventInstances.insert({ eventName, eventInstance });
    eventDescriptions.insert({ eventName, eventDescription });
}

void AudioEngine::setFMODEventParamValue(const char* eventName, const char* parameterName, float value) {
    if (eventInstances.count(eventName) > 0) {
        ERRCHECK(eventInstances[eventName]->setParameterByName(parameterName, value));
    }
    else
        std::cout << "Event " << eventName << " was not in event instance cache, can't set param \n";

}

void AudioEngine::playEvent(const char* eventName, int instanceIndex) {
    std::cout << "Before Playing Event, checking state of Event Description " << eventName << '\n';
    printEventInfo(eventDescriptions[eventName]);
    auto eventInstance = eventInstances[eventName];
    if (eventInstances.count(eventName) > 0) {
        std::cout << "Playing Event " << eventName << '\n';
        auto eventInstance = eventInstances[eventName];
        ERRCHECK(eventInstance->start());
    }
    else
        std::cout << "Event " << eventName << " was not in event instance cache, cannot play \n";
}

void AudioEngine::stopEvent(const char* eventName, int instanceIndex) {
    if(eventInstances.count(eventName) > 0)
        ERRCHECK(eventInstances[eventName]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
    else
        std::cout << "Event " << eventName << " was not in event instance cache, cannot stop \n";
}


// Private definitions 

bool AudioEngine::soundIsCached(const char* filepath) {
    return soundCache.count(filepath) > 0;
}

FMOD::Sound* AudioEngine::getSound(const char* filepath) {
    return soundCache[filepath];
}

// Error checking/debugging function defintions

void ERRCHECK_fn(FMOD_RESULT result, const char* file, int line)
{
    if (result != FMOD_OK)
        std::cout << "FMOD ERROR: AudioEngine.cpp [Line " << line << "] " << result << "  - " << FMOD_ErrorString(result) << '\n';
}

void AudioEngine::printEventInfo(FMOD::Studio::EventDescription* eventDescription) {
    
    int params;
    bool is3D, isOneshot;
    ERRCHECK(eventDescription->getParameterDescriptionCount(&params));
    ERRCHECK(eventDescription->is3D(&is3D));
    ERRCHECK(eventDescription->isOneshot(&isOneshot));

    std::cout << "FMOD EventDescription has " << params << " parameter descriptions, "
        << (is3D ? " is " : " isn't ") << " 3D, "
        << (isOneshot ? " is " : " isn't ") << " oneshot, "
        << (eventDescription->isValid() ? " is " : " isn't ") << " valid.\n";
}