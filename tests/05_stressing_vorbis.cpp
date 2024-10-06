#include <iostream>

#include <chrono>
#include <thread>

#include "../include/crest.h"
#include "../include/effects.h"
#include "../include/vorbisutils.h"
#include "../include/wavutils.h"

int main()
{
    unsigned int src_1_timer = 0, src_2_timer = 0;

    crest::init(50);

    crest::audio_source src_1;
    crest::register_source(&src_1);

    crest::audio_source src_2;
    crest::register_source(&src_2);

    crest::effect_volume volume(0.25f);

    crest::vorbis_stream stream_1("./tests/audio/jingle.ogg");
    crest::vorbis_stream stream_2("./tests/audio/triwave.ogg");

    src_1.add_transform(&volume);
    src_2.add_transform(&volume);

    while(true)
    {
        if(src_1_timer == 100)
        {
            src_1.add_stream(&stream_1);
            src_1_timer = 0;
        }
        if(src_2_timer == 10)
        {
            src_2.add_stream(&stream_2);
        }

        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));

        src_1_timer += 1;
        src_2_timer += 1;
    }

    crest::remove_source(&src_1);
    crest::remove_source(&src_2);

    return 0;
}