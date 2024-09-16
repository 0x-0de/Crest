# Crest
A free and lightweight audio library for C++ developers.

This project serves as a replacement for my other audio library, **fcal**. Crest currently only supports Windows devices, with WASAPI capabilities. Linux support is hopefully coming in the future.

## Current Capabilities
- Audio playback with WASAPI for Windows devices (Vista or later).
- Built-in decoders for .wav files, .flac files, and .ogg vorbis files (thanks to stb_vorbis)
- Simple volume and balance controls.
- Audio stream and transform interfaces to allow developers to write their own audio generators and one-to-one transforms.

## Planned Features
- Support for Ubuntu devices.
- Audio input capabilities.

## Compiling and Linking

### Windows
The necessary header files are located in the ```include``` folder. You could also just use the ones from the ```src``` folder, they're the exact same.

Run the provided makefile with ```make build``` to build the .dll for Windows devices, or use the .dll provided in the ```bin``` folder. Remember to link with ```-lcrest```.

For building tests, run ```make test_[#]``` (replace ```[#]``` with ```01```, ```02```, or whatever two-figure number corresponds to which test) after running ```make build```.

### Linux
Not supported yet.

## Attributions
Crest makes use of the vorbis decoder provided with ```stb_vorbis.c```. This file is public-domain, and found here: https://github.com/nothings/stb/tree/master

## License
Crest uses the zlib license. For more information, check the ```LICENSE.md``` file.