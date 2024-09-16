#include "../include/crest.h"
#include "../include/vorbisutils.h"

int main()
{
    crest::init(50);

    crest::audio_source src;
    crest::register_source(&src);

    crest::vorbis_stream stream("./tests/audio/revival 15.ogg");

    src.add_stream(&stream);

    while(src.is_playing()) {}

    crest::remove_source(&src);
    crest::terminate();

    return 0;
}