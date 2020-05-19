### FMOD VS19 Setup Overview - (see FMOD-VS19-Setup.docx for detailed directions)
##### This guide works best with a Visual Studio 19 Project created following CPSC5910's SetupOpenGL.pdf

###### 1. Obtain FMOD’s .dll files, .lib files, and header files. These are included in this repository, or can optionally be obtained by downloading/installing the FMOD Studio API from https://www.fmod.com/download

###### 2. Create a new sub-directory in your VS Project’s Include folder, titled ‘FMOD’ [e.g: /Include/FMOD/]. Place the FMOD header files inside this new FMOD sub-directory.	

###### 3. Determine the libraries you need to add to your project, based on your VS Project’s existing build configuration. Choose either x64 or x86 (Win32) dependencies; you cannot use both in the same VS project. Then, place the chosen libraries directly into your project’s library directory (/Lib).

###### 4. Add the new dependencies to Debug and Release Configurations of VS: 'Project Properties/Configuration Properties/Linker/Input/Additional Dependencies'

###### 5. Tell Visual Studio where to find your Debugging Environment. 

###### 6. (Optional) Add the AudioEngine.h and AudioEngine.cpp files to your Visual Studio Project, and #include “AudioEngine.h”.
