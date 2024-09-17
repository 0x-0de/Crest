#include "../include/crest.h"
#include "../include/effects.h"
#include "../include/wavutils.h"

static const char* wav_files[5] =
{
    "./tests/audio/arp base.wav",
    "./tests/audio/arp junk in header.wav",
    "./tests/audio/arp missing data.wav",
    "./tests/audio/arp missing format.wav",
    "./tests/audio/arp missing riff.wav"
};

static crest::wav_stream* streams[5];

int main()
{
    crest::init(50);

    crest::audio_source src;
    crest::register_source(&src);

    crest::effect_volume volume(0.25f);

    for(int i = 0; i < 5; i++)
    {
        streams[i] = new crest::wav_stream(wav_files[i]);
    }

    bool active = true;
    int current_index = 0;

    src.add_transform(&volume);

    while(active)
    {
        if(!src.is_playing())
        {
            src.add_stream(streams[current_index]);
            delete streams[current_index];
            current_index++;
        }
        if(current_index == 5)
        {
            active = false;
        }
    }

    crest::remove_source(&src);
    crest::terminate();

    return 0;
}