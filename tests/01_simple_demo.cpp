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

    src.add_stream(&stream);
    src.add_transform(&volume);

    while(src.is_playing()) {}

    crest::remove_source(&src);
    crest::terminate();

    return 0;
}