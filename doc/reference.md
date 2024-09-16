# Crest | Reference
Crest is a relatively minimalistic framework, meant primarily to simplify a lot of use cases, and cover the "backend" stuff. The main way developers will interact with Crest is through implementing interface objects. The two interface objects that you'll primarily be working with are ```stream``` and ```stream_transform``` objects.

```stream``` objects serve as audio generators for the Crest framework. The example implementations in Crest don't "generate" audio, rather they pull audio data out of audio files, like .wav, .flac, or .ogg vorbis files, but they can certainly be used for synthesizing audio as well.

```stream_transform``` objects simply perform a one-to-one transform on a linear stream of audio. The example ```stream_transform``` objects simply alter the volume and 2-channel balance of stream objects.

Crest also uses ```audio_source``` objects as a bridge between the ```audio_stream``` objects and the sound card. ```audio_stream``` objects can be added to ```audio_source``` objects, and, if the sources have been registered with Crest, streams will then proceed to output audio.

Here is a working example:

```
#include "../include/crest.h"
#include "../include/vorbisutils.h"

int main()
{
    crest::init(50);

    crest::audio_source src;
    crest::register_source(&src);

    crest::vorbis_stream stream("[path-to-file]");

    src.add_stream(&stream);

    while(src.is_playing()) {}

    crest::remove_source(&src);
    crest::terminate();

    return 0;
}
```

This code is available as ```tests/01_simple_demo.cpp``` and can be compiled with a working audio filepath with ```make test_01```.

