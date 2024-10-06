#include <iostream>

#include <chrono>
#include <thread>

#include "../include/crest.h"
#include "../include/effects.h"
#include "../include/vorbisutils.h"

int main()
{
    crest::init(50);

    crest::audio_source src;
    crest::register_source(&src);

    crest::effect_volume volume(0.25f);

    crest::vorbis_stream stream("./tests/audio/jingle.ogg");

    src.add_transform(&volume);

    while(true)
    {
        std::cout << "?" << std::endl;
        src.add_stream(&stream);
        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(500));
    }

    return 0;
}