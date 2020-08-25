#pragma once
///
/// @file AudioEngine.h
/// 
/// FMOD wrapper which loads sound files (.wav, .mp3, .ogg etc) and FMOD soundbanks (.bank files)
/// and supports looping or one-shot playback in stereo, as well as customizable 3D positional audio
/// Implements the FMOD Studio and FMOD Core API's to allow audio file-based implementations,
/// alongside/in addition to use of FMOD Studio Sound Banks.
///
/// @author Ross Hoyt
/// @dependencies FMOD Studio & Core
/// 
#include <FMOD/fmod_studio.hpp>
#include <FMOD/fmod.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "SoundInfo.h"

/**
 * Error Handling Function for FMOD Errors
 * @param result - the FMOD_RESULT generated during every FMOD 
 */
void ERRCHECK_fn(FMOD_RESULT result, const char* file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

/**
 * Class that handles the process of loading and playing sounds by wrapping FMOD's functionality.
 * Deals with all FMOD calls so that FMOD-specific code does not need to be used outside this class.
 * Only one AudioEngine should be constructed for an application.
 */
class AudioEngine {
public:
    /**
     * Default AudioEngine constructor. 
     * AudioEngine::init() must be called before using the Audio Engine 
     */
    AudioEngine();

    /**
     * Initializes Audio Engine Studio and Core systems to default values. 
     */
    void init();

    /**
     * Method that is called to deactivate the audio engine after use.
     */
    void deactivate();

    /**
    * Method which should be called every frame of the game loop
    */
    void update();
    
    /**
     * Loads a sound from disk using provided settings
     * Prepares for later playback with playSound()
     * Only reads the audio file and loads into the audio engine
     * if the sound file has already been added to the cache
     */
    void loadSound(SoundInfo soundInfo);

    /**
    * Plays a sound file using FMOD's low level audio system. If the sound file has not been
    * previously loaded using loadSoundFile(), a console message is displayed
    *
    * @param filename - relative path to file from project directory. (Can be .OGG, .WAV, .MP3,
    *                 or any other FMOD-supported audio format)
    */
    void playSound(SoundInfo soundInfo);
    
    /**
     * Stops a looping sound if it's currently playing.
     */
    void stopSound(SoundInfo soundInfo);

    /**
     * Method that updates the volume of a soundloop that is playing. This can be used to create audio 'fades'
     * where the volume ramps up or down to the provided new volume
     * @param fadeSampleLength the length in samples of the intended volume sample. If less than 64 samples, the default 
     *                         FMOD fade out is used
     */
    void updateSoundLoopVolume(SoundInfo &soundInfo, float newVolume, unsigned int fadeSampleLength = 0);

   

    /**
    * Updates the position of a looping 3D sound that has already been loaded and is playing back.
    * The SoundInfo object's position coordinates will be used for the new sound position, so
    * SoundInfo::set3DCoords(x,y,z) should be called before this method to set the new desired location.
    */
    void update3DSoundPosition(SoundInfo soundInfo); 
      
    /**
     * Checks if a looping sound is playing.
     */
    bool soundIsPlaying(SoundInfo soundInfo); 
   

    /**
     * Sets the position of the listener in the 3D scene.
     * @param posX, posY, posZ - 3D translation of listener
     * @param forwardX, forwardY, forwardZ - forward angle character is looking in
     * @param upX, upY, upZ - up which must be perpendicular to forward vector
     */
    void set3DListenerPosition(float posX,     float posY,     float posZ,
                               float forwardX, float forwardY, float forwardZ,
                               float upX,      float upY,      float upZ);

    /**
    * Utility method that returns the length of a SoundInfo's audio file in milliseconds
    * If the sound hasn't been loaded, returns 0
    */
    unsigned int getSoundLengthInMS(SoundInfo soundInfo);

    /**
     * Loads an FMOD Studio soundbank 
     * TODO Fix
     */
    void loadFMODStudioBank(const char* filePath);
    
    /**
     * Loads an FMOD Studio Event. The Soundbank that this event is in must have been loaded before
     * calling this method.
     * TODO Fix
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
 
    /**
     * Sets the volume of an event.
     * @param volume0to1 - volume of the event, from 0 (min vol) to 1 (max vol)
     */
    void setEventVolume(const char* eventName, float volume0to1 = .75f);

    /**
     * Checks if an event is playing.
     */
    bool eventIsPlaying(const char* eventName, int instance = 0);

    /**
     * Mutes all sounds for the audio engine
     */
	void muteAllSounds();

    /**
     * Unmutes all sounds for the audio engine
     */
	void unmuteAllSound();

    /**
     * Returns true if the audio engine is muted, false if not
     */
	bool isMuted();

    // The audio sampling rate of the audio engine
    static const int AUDIO_SAMPLE_RATE = 44100;

private:  

    /**
     * Checks if a sound file is in the soundCache
     */
    bool soundLoaded(SoundInfo soundInfo);

    /**
     * Sets the 3D position of a sound
     */
    void set3dChannelPosition(SoundInfo soundInfo, FMOD::Channel* channel);

    /**
     * Initializes the reverb effect
     */
    void initReverb();

    /**
     * Prints debug info about an FMOD event description
     */
    void printEventInfo(FMOD::Studio::EventDescription* eventDescription);

    // FMOD Studio API system, which can play FMOD sound banks (*.bank)
    FMOD::Studio::System* studioSystem = nullptr;       
    
    // FMOD's low-level audio system which plays audio files and is obtained from Studio System
    FMOD::System* lowLevelSystem = nullptr;          

    // Max FMOD::Channels for the audio engine 
    static const unsigned int MAX_AUDIO_CHANNELS = 1024; 
    
    // Units per meter.  I.e feet would = 3.28.  centimeters would = 100.
    const float DISTANCEFACTOR = 1.0f;  
 
    // Listener head position, initialized to default value
    FMOD_VECTOR listenerpos = { 0.0f, 0.0f, -1.0f * DISTANCEFACTOR };
    
    // Listener forward vector, initialized to default value
    FMOD_VECTOR forward     = { 0.0f, 0.0f, 1.0f };
    
    // Listener upwards vector, initialized to default value
    FMOD_VECTOR up          = { 0.0f, 1.0f, 0.0f };

    // Main group for low level system which all sounds go though
    FMOD::ChannelGroup* mastergroup = 0;

    // Low-level system reverb TODO add multi-reverb support
	FMOD::Reverb3D* reverb;

	// Reverb origin position
	FMOD_VECTOR revPos = { 0.0f, 0.0f, 0.0f };

	// reverb min, max distances
	float revMinDist = 10.0f, revMaxDist = 50.0f;

    // flag tracking if the Audio Engin is muted
    bool muted = false;

    /*
     * Map which caches FMOD Low-Level sounds
     * Key is the SoundInfo's uniqueKey field.
     * Value is the FMOD::Sound* to be played back.
     * TODO Refactor to use numeric UID as key
     */
    std::map<std::string, FMOD::Sound*> sounds;

    /*
     * Map which stores the current playback channels of any playing sound loop
     * Key is the SoundInfo's uniqueKey field.
     * Value is the FMOD::Channel* the FMOD::Sound* is playing back on.
     */
    std::map<std::string, FMOD::Channel*> loopsPlaying;

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
