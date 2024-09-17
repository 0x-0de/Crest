#include "../include/crest.h"
#include "../include/effects.h"
#include "../include/wavutils.h"

static const char* wav_files[5] =
{
    "./tests/audio/wav/arp base.wav",             //Base case - no errors or uncommon things in the .wav file.
    "./tests/audio/wav/arp junk in header.wav",   //Uncommon case - Header of the .wav file contains a JUNK chunk, which should be ignored.
    "./tests/audio/wav/arp missing data.wav",     //Faulty case - .wav file is missing 'data' section, or the header indicating the beginning of said data section. File is unusable.
    "./tests/audio/wav/arp missing format.wav",   //Faulty case - .wav file is missing format header, or the signifier for it. File is unusable.
    "./tests/audio/wav/arp missing riff.wav"      //Faulty case - .wav file is missing RIFF tag. File is unusable.
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