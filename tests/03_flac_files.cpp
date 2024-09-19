#include <iostream>

#include "../include/crest.h"
#include "../include/effects.h"
#include "../include/flacutils.h"

static const char* flac_files[64] =
{   
    //Subset cases - these are all valid and should function properly.
    "./tests/audio/flac/subset/01 - blocksize 4096.flac",
    "./tests/audio/flac/subset/02 - blocksize 4608.flac",
    "./tests/audio/flac/subset/03 - blocksize 16.flac",
    "./tests/audio/flac/subset/04 - blocksize 192.flac",
    "./tests/audio/flac/subset/05 - blocksize 254.flac",
    "./tests/audio/flac/subset/06 - blocksize 512.flac",
    "./tests/audio/flac/subset/07 - blocksize 725.flac",
    "./tests/audio/flac/subset/08 - blocksize 1000.flac",
    "./tests/audio/flac/subset/09 - blocksize 1937.flac",
    "./tests/audio/flac/subset/10 - blocksize 2304.flac",
    "./tests/audio/flac/subset/11 - partition order 8.flac",
    "./tests/audio/flac/subset/12 - qlp precision 15 bit.flac",
    "./tests/audio/flac/subset/13 - qlp precision 2 bit.flac",
    "./tests/audio/flac/subset/14 - wasted bits.flac",
    "./tests/audio/flac/subset/15 - only verbatim subframes.flac",
    "./tests/audio/flac/subset/16 - partition order 8 containing escaped partitions.flac",
    "./tests/audio/flac/subset/17 - all fixed orders.flac",
    "./tests/audio/flac/subset/18 - precision search.flac",
    "./tests/audio/flac/subset/19 - samplerate 35467Hz.flac",
    "./tests/audio/flac/subset/20 - samplerate 39kHz.flac",
    "./tests/audio/flac/subset/21 - samplerate 22050Hz.flac",
    "./tests/audio/flac/subset/22 - 12 bit per sample.flac",
    "./tests/audio/flac/subset/23 - 8 bit per sample.flac",
    "./tests/audio/flac/subset/24 - variable blocksize file created with flake revision 264.flac",
    "./tests/audio/flac/subset/25 - variable blocksize file created with flake revision 264, modified to create smaller blocks.flac",
    "./tests/audio/flac/subset/26 - variable blocksize file created with CUETools.Flake 2.1.6.flac",
    "./tests/audio/flac/subset/27 - old format variable blocksize file created with Flake 0.11.flac",
    "./tests/audio/flac/subset/28 - high resolution audio, default settings.flac",
    "./tests/audio/flac/subset/29 - high resolution audio, blocksize 16384.flac",
    "./tests/audio/flac/subset/30 - high resolution audio, blocksize 13456.flac",
    "./tests/audio/flac/subset/31 - high resolution audio, using only 32nd order predictors.flac",
    "./tests/audio/flac/subset/32 - high resolution audio, partition order 8 containing escaped partitions.flac",
    "./tests/audio/flac/subset/33 - samplerate 192kHz.flac",
    "./tests/audio/flac/subset/34 - samplerate 192kHz, using only 32nd order predictors.flac",
    "./tests/audio/flac/subset/35 - samplerate 134560Hz.flac",
    "./tests/audio/flac/subset/36 - samplerate 384kHz.flac",
    "./tests/audio/flac/subset/37 - 20 bit per sample.flac",
    "./tests/audio/flac/subset/38 - 3 channels (3.0).flac",
    "./tests/audio/flac/subset/39 - 4 channels (4.0).flac",
    "./tests/audio/flac/subset/40 - 5 channels (5.0).flac",
    "./tests/audio/flac/subset/41 - 6 channels (5.1).flac",
    "./tests/audio/flac/subset/42 - 7 channels (6.1).flac",
    "./tests/audio/flac/subset/43 - 8 channels (7.1).flac",
    "./tests/audio/flac/subset/44 - 8-channel surround, 192kHz, 24 bit, using only 32nd order predictors.flac",
    "./tests/audio/flac/subset/45 - no total number of samples set.flac",
    "./tests/audio/flac/subset/46 - no min-max framesize set.flac",
    "./tests/audio/flac/subset/47 - only STREAMINFO.flac",
    "./tests/audio/flac/subset/48 - Extremely large SEEKTABLE.flac",
    "./tests/audio/flac/subset/49 - Extremely large PADDING.flac",
    "./tests/audio/flac/subset/50 - Extremely large PICTURE.flac",
    "./tests/audio/flac/subset/51 - Extremely large VORBISCOMMENT.flac",
    "./tests/audio/flac/subset/52 - Extremely large APPLICATION.flac",
    "./tests/audio/flac/subset/53 - CUESHEET with very many indexes.flac",
    "./tests/audio/flac/subset/54 - 1000x repeating VORBISCOMMENT.flac",
    "./tests/audio/flac/subset/55 - file 48-53 combined.flac",
    "./tests/audio/flac/subset/56 - JPG PICTURE.flac",
    "./tests/audio/flac/subset/57 - PNG PICTURE.flac",
    "./tests/audio/flac/subset/58 - GIF PICTURE.flac",
    "./tests/audio/flac/subset/59 - AVIF PICTURE.flac",
    "./tests/audio/flac/subset/60 - mono audio.flac",
    "./tests/audio/flac/subset/61 - predictor overflow check, 16-bit.flac",
    "./tests/audio/flac/subset/62 - predictor overflow check, 20-bit.flac",
    "./tests/audio/flac/subset/63 - predictor overflow check, 24-bit.flac",
    "./tests/audio/flac/subset/64 - rice partitions with escape code zero.flac"
};

static crest::flac_stream* streams[64];

int main()
{
    crest::init(50);

    crest::audio_source src;
    crest::register_source(&src);

    crest::effect_volume volume(0.25f);

    for(int i = 0; i < 64; i++)
    {
        std::cout << "Reading stream " << (i + 1) << "..." << std::endl;
        streams[i] = new crest::flac_stream(flac_files[i]);
    }

    bool active = true;
    int current_index = 0;

    src.add_transform(&volume);

    while(active)
    {
        if(!src.is_playing())
        {
            if(current_index == 64)
            {
                active = false;
            }
            else
            {
                std::cout << "Playing stream " << (current_index + 1) << "..." << std::endl;
                src.add_stream(streams[current_index]);
                delete streams[current_index];
                current_index++;
            }
        }
    }

    crest::remove_source(&src);
    crest::terminate();

    return 0;
}