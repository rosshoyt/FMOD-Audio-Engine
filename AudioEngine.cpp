/// 
/// @file AudioEngine.cpp
///
#include "AudioEngine.h"
#include <FMOD/fmod_errors.h>
#include <iostream>

AudioEngine::AudioEngine() : sounds(), loopsPlaying(), soundBanks(), eventDescriptions(), eventInstances() {}

void AudioEngine::init() {
    ERRCHECK(FMOD::Studio::System::create(&studioSystem));
    ERRCHECK(studioSystem->getCoreSystem(&lowLevelSystem));
    ERRCHECK(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0));
    ERRCHECK(lowLevelSystem->set3DSettings(1.0, DISTANCEFACTOR, 1.0f));
    ERRCHECK(studioSystem->initialize(MAX_AUDIO_CHANNELS, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));

}

void AudioEngine::update() {
    ERRCHECK(studioSystem->update()); // also updates the low level system
}

void AudioEngine::loadSound(SoundInfo soundInfo) {
    if (!soundLoaded(soundInfo)) {
        std::cout << "Audio Engine: Loading Sound from file " << soundInfo.filePath << '\n';
        FMOD::Sound* sound;
        ERRCHECK(lowLevelSystem->createSound(soundInfo.filePath, soundInfo.is3D ? FMOD_3D : FMOD_2D, 0, &sound));
        ERRCHECK(sound->setMode(soundInfo.isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));
        ERRCHECK(sound->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR));
        sounds.insert({ soundInfo.uniqueID, sound });
    }
    else
        std::cout << "Audio Engine: Sound File was already loaded!\n";
}

void AudioEngine::playSound(SoundInfo soundInfo) {
    if (soundLoaded(soundInfo)) {
        FMOD::Channel* channel;
        // start play in 'paused' state
        ERRCHECK(lowLevelSystem->playSound(sounds[soundInfo.uniqueID], 0, true /* start paused */, &channel));

        if (soundInfo.is3D)
            set3dChannelPosition(soundInfo, channel);
        
        if (soundInfo.isLoop) // add to channel map of sounds currently playing, to stop later
            loopsPlaying.insert({ soundInfo.uniqueID, channel });

        // start audio playback
        ERRCHECK(channel->setPaused(false));
         
    }
    else
        std::cout << "Audio Engine: Can't play, sound was not loaded yet from " << soundInfo.filePath << '\n';

}

void AudioEngine::stopSound(SoundInfo soundInfo) {
    if (soundIsPlaying(soundInfo)) {
        loopsPlaying[soundInfo.uniqueID]->stop();
        loopsPlaying.erase(soundInfo.uniqueID);
    }
    else
        std::cout << "Audio Engine: Can't stop a sound that's not playing!\n";
}

bool AudioEngine::soundIsPlaying(SoundInfo soundInfo) {
    return soundInfo.isLoop && loopsPlaying.count(soundInfo.uniqueID);
}


void AudioEngine::update3DSoundPosition(SoundInfo soundInfo) {
    if (soundIsPlaying(soundInfo)) 
        set3dChannelPosition(soundInfo, loopsPlaying[soundInfo.uniqueID]);
    else
        std::cout << "Audio Engine: Can't update sound position!\n";

}


void AudioEngine::set3DListenerPosition(float posX, float posY, float posZ, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ) {
    listenerpos = { posX,     posY,     posZ };
    forward =     { forwardX, forwardY, forwardZ };
    up =          { upX,      upY,      upZ };
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
bool AudioEngine::soundLoaded(SoundInfo soundInfo) {
    return sounds.count(soundInfo.uniqueID) > 0;
}
void AudioEngine::set3dChannelPosition(SoundInfo soundInfo, FMOD::Channel* channel) {
    FMOD_VECTOR position = { soundInfo.x * DISTANCEFACTOR, soundInfo.y * DISTANCEFACTOR, soundInfo.z * DISTANCEFACTOR };
    FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f }; // TODO Add dopplar (velocity) support
    ERRCHECK(channel->set3DAttributes(&position, &velocity));
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