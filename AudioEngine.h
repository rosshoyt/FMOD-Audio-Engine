#pragma once
///
/// @file AudioEngine.h
/// 
/// This audio engine is an FMOD wrapper with provides sound file loading, 2D/3D audio
/// playback, audio file looping, FMOD Soundbank loading, FMOD Event playback, and more.
///
/// @author Ross Hoyt
/// @dependencies FMOD Studio & Core
/// 
#include <FMOD/fmod_studio.hpp>
#include <FMOD/fmod.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// FMOD Error Handling
void ERRCHECK_fn(FMOD_RESULT result, const char* file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

class AudioEngine {
public:
    /**
    * Initializes Audio Engine Studio and Core systems
    * FMOD's Distance factor is set to 1.0f by default (1 meter/ 3D game unit)
    */
    AudioEngine();

    /**
    * Method which should be called every frame of the game loop
    */
    void update();

    /**
    * Loads a 2D sound which will playback in tradidional stereo mode (IE, not positional audio)
    * The file is read into the cache to prepare for later playback.
    * Only reads file and creates the sound if it has not already been added to the cache.
    * To play the sound later, use method playSoundFile()
    */
    void loadSoundFile(const char* filepath,bool loop);
    
    /**
    * Plays a sound file using FMOD's low level audio system. If the sound file has not been
    * previously loaded using loadSoundFile(), a console message is displayed
    *
    * @var filename - relative path to file from project directory. (Can be .OGG, .WAV, .MP3,
    *                 or any other FMOD-supported audio format)
    */
    void playSoundFile(const char* filepath);

    /**
    * Loads a 3D sound. The position of the sound is set while calling method play3DSoundFile().
    * The file is read into the cache to prepare for later playback.
    * Only reads file and creates the sound if it has not already been added to the cache.
    * To play the sound later, use method play3DSoundFile()
    */
    void load3DSoundFile(const char* filepath, bool loop);

    /**
    * Plays a 3D sound file using FMOD's low level audio system. If the sound file has not been
    * previously loaded using load3DSoundFile(), a console message is displayed
    *
    * @var filename - relative path to file from project directory. (Can be .OGG, .WAV, .MP3,
    *                 or any other FMOD-supported audio format)
    */
    void play3DSoundFile(const char* filepath, float x, float y, float z);
   
    /**
    * Sets the posision of a looping 3D sound that has already been loaded and is playing back.
    */
    void update3DSoundPosition(const char* filepath, float x, float y, float z);

    /**
    * Sets the position of the listener in the 3D scene.
    * @var posX, posY, posZ - 3D translation of listener
    * @var forwardX, forwardY, forwardZ - forward angle character is looking in
    * @var upX, upY, upZ - up which must be perpendicular to forward vector
    */
    void set3DListenerPosition(float posX,     float posY,     float posZ,
                               float forwardX, float forwardY, float forwardZ,
                               float upX,      float upY,      float upZ);

    /**
    * Loads an FMOD Studio soundbank
    */
    void loadFMODStudioBank(const char* filepath);
    
    /**
    * Loads an FMOD Studio Event. The Soundbank that this event is in must have been loaded before
    * calling this method.
    */
    void loadFMODStudioEvent(const char* eventName, std::vector<std::pair<const char*, float>> paramsValues = { });
    
    /**
    * Sets the parameter of an FMOD Soundbank Event Instance.
    */
    void setFMODEventParamValue(const char* eventName, const char* parameterName, float value);
    
    /**
    * Plays the specified instance of an event
    * TODO support playback of multiple event instances
    * TODO Fix playback
    */
    void playEvent(const char* eventName, int instanceIndex = 0);
    
    /**
    * Stops the specified instance of an event, if it is playing.
    */
    void stopEvent(const char* eventName, int instanceIndex = 0);
 
private:    
    /**
    * Checks if a sound file is in the soundCache
    */
    bool soundIsCached(const char* filepath);

    /**
    * Gets a sound from the cache without checking if it has been added.
    * Only use if certain sound is in cache, as one will be created at provided filepath
    * key value (but it won't be initialized or work)
    */
    FMOD::Sound* getSound(const char* filepath);

    /**
    * Prints debug info about an FMOD event description
    */
    void printEventInfo(FMOD::Studio::EventDescription* eventDescription);


    // FMOD Studio API system, which can play FMOD sound banks (*.bank)
    FMOD::Studio::System* studioSystem = nullptr;       
    
    // FMOD's low-level audio system which plays audio files and is obtained from Studio System)
    FMOD::System* lowLevelSystem = nullptr;          
    
    // Max FMOD::Channels for the audio engine 
    static const unsigned int MAX_AUDIO_CHANNELS = 1024; 
    
    // Units per meter.  I.e feet would = 3.28.  centimeters would = 100.
    const float DISTANCEFACTOR = 1.0f;  
    
    // Listener position, initialized to default value
    FMOD_VECTOR listenerpos = { 0.0f, 0.0f, -1.0f * DISTANCEFACTOR };
    
    // Listener forward vector, initialized to default value
    FMOD_VECTOR forward     = { 0.0f, 0.0f, 1.0f };
    
    // Listener upwards vector, initialized to default value
    FMOD_VECTOR up          = { 0.0f, 1.0f, 0.0f };

    /*
    * Map which caches FMOD Low-Level sounds, created with loadSoundFile() and load3DSoundFile(). 
    * Key is the relative file path of each unique sound asset. 
    * Value is the FMOD::Sound* to be played back.
    * TODO Refactor to use numeric UID as key
    */
    std::map<std::string, FMOD::Sound*> soundCache;   
    
    /*
    * Map which stores the current playback channels of any looping sound that is created with 
    * loadSoundFile() and load3DSoundFile().
    * Key is the relative file path of each the sound asset.
    * Value is the FMOD::Channel* the FMOD::Sound* is playing back on.
    */
    std::map<std::string, FMOD::Channel*> channelMap;

    /*
    * Map which stores the soundbanks loaded with loadFMODStudioBank()
    */
    std::map<std::string, FMOD::Studio::Bank*> soundBanks;
    
    /*
    * Map which stores event descriptions created during loadFMODStudioEvent()
    */
    std::map<std::string, FMOD::Studio::EventDescription*> eventDescriptions;
    
    /*
    * Map which stores event instances created during loadFMODStudioEvent()
    */
    std::map<std::string, FMOD::Studio::EventInstance*> eventInstances;
};
