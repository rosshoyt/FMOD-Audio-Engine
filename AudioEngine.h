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
#include <list>
#include <map>

// FMOD Error Handling Function
void ERRCHECK_fn(FMOD_RESULT result, const char* file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)


struct SoundInfo {
    
    std::string uniqueID; 

    const char* filePath;
    
    bool isLoop, is3D, isLoaded;
    
    float x, y, z;

    // convienience method to set the 3D coordinates of the sound.
    void set3DCoords(float x, float y, float z) {
        this->x = x, this->y = y, this->z = z;
    }

    SoundInfo(const char* filePath = "", bool isLoop = false, bool is3D = false, float x = 0.0f, float y = 0.0f, float z = 0.0f) 
        : filePath(filePath), isLoop(isLoop), is3D(is3D), x(x), y(y), z(z) 
    {
        uniqueID = filePath;
    }

    // TODO  implement sound instancing
    // int instanceID = -1; 

};

class AudioEngine {
public:

    /**
    * Audio Engine constructor. method init() must be called before use of the Audio Engine.
    */
    AudioEngine();

    /**
    * Initializes Audio Engine Studio and Core systems
    * FMOD's Distance factor is set to 1.0f by default (1 meter/ 3D game unit)
    */
    void init(); // TODO
    /**
    * Method which should be called every frame of the game loop
    */
    void update();
    
    /**
    * Loads a mono or stereo audio file,  which will playback in tradidional stereo mode (IE, not positional audio)
    * The file is read into the cache to prepare for later playback.
    * Only reads file and creates the sound if it has not already been added to the cache.
    * To play the sound later, use method playSound()
    */
    void loadSound(SoundInfo soundInfo);

    /**
    * Plays a sound file using FMOD's low level audio system. If the sound file has not been
    * previously loaded using loadSoundFile(), a console message is displayed
    *
    * @var filename - relative path to file from project directory. (Can be .OGG, .WAV, .MP3,
    *                 or any other FMOD-supported audio format)
    */
    void playSound(SoundInfo soundInfo);
    
    void stopSound(SoundInfo soundInfo);

    /**
    * Updates the position of a looping 3D sound that has already been loaded and is playing back.
    * The SoundInfo object's position coordinates will be used for the new sound position, so
    * SoundInfo::set3DCoords(x,y,z) should be called before this method to set the new desired location.
    */
    void update3DSoundPosition(SoundInfo soundInfo); 
    
    

    bool soundIsPlaying(SoundInfo soundInfo); 
   

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
    * TODO Fix
    */
    void stopEvent(const char* eventName, int instanceIndex = 0);
 
private:  

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
    
    /**
    * Checks if a sound file is in the soundCache
    */
    bool soundLoaded(SoundInfo soundInfo); 


    /**
    * Sets the 3D position of a sound 
    */
    void set3dChannelPosition(SoundInfo soundInfo, FMOD::Channel* channel);
    

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
    
    // Listener position, initialized to default value
    FMOD_VECTOR listenerpos = { 0.0f, 0.0f, -1.0f * DISTANCEFACTOR };
    
    // Listener forward vector, initialized to default value
    FMOD_VECTOR forward     = { 0.0f, 0.0f, 1.0f };
    
    // Listener upwards vector, initialized to default value
    FMOD_VECTOR up          = { 0.0f, 1.0f, 0.0f };

   

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
