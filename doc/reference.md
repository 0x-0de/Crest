# Crest | Reference
Crest is a relatively minimalistic framework, meant primarily not to cover but instead *simplify* a lot of audio-related use cases, and cover the "backend" stuff. The main way developers will interact with Crest is through implementing interface objects. The two interface objects that you'll primarily be working with are ```stream``` and ```stream_transform``` objects.

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

More examples of working Crest projects can be found within the ```tests``` folder, and it's recommended to take a look at those for additional working examples of projects.

## Global functions
All declarations for these functions, and the structures and classes listed below this section are found within ```include/crest.h```

```using namespace crest;``` - All accessible functions and structures in Crest are found within the ```crest``` namespace. From here on out, assume every function and class name is prefixed with ```crest::```.

```void init(UINT32 ms)```: Initializes WASAPI, the crest audio buffer, and the processing and playback threads. ```ms``` determines the length of the audio buffer in milliseconds. The true size of the audio buffer will be determined by the sample rate of the user's audio playback sample rate per millisecond multiplied by the length of the requested buffer in milliseconds multiplied by the number of channels the user's audio playback device is using.

```void terminate()```: Terminates WASAPI and Crest's audio threads, and frees any memory associated with the audio buffer.

```void register_source(audio_source* src)```: Registers an ```audio_source``` object with Crest, meaning that Crest will begin "listening" to the audio source and playing back data from any ```stream``` that is added to it.

```void remove_source(audio_source* src)```: Removes an ```audio_source``` from the pool of sources that Crest will listen to. Note that any remaining streams within a removed source will not be cleared automatically and will thus serve as dead weight without being cleared manually, memory-wise.

```audio_format get_client_audio_format()```: Returns the current ```audio_format``` being used by the current playback device.

## ```audio_format```
The ```audio_format``` structure contains information about the audio format of a given object that contains, records, or plays audio data. Each ```stream``` object has an associated ```audio_format```, and the client itself has an audio format accessible with ```get_client_audio_format()``` (covered above). An ```audio_format``` structure contains the following variables:

- ```sample_rate``` - The number of individual samples, or frames, that play within a second. In most cases it's frames, but custom implemented streams may use this variable differently.
- ```bit_depth``` - The bit depth of the audio data. In most cases this variable will not matter, as all ```stream``` objects produce an array of floats, which have a constant bit depth of 32 in C/++. However, this variable is important in converting these floats to the final bit depth that is required by the client audio playback device (which Crest does automatically).
- ```channels``` - The number of audio channels that the audio data is expected to cover. For audio file decoders/readers, mono files will produce only 1 channel, while stereo files will typically produce 2 channels.
- ```num_samples``` - This is an additional variable used for denoting the total number of samples a ```stream``` will produce. In most cases, this variable can be ignored.

## ```stream```
The ```stream``` interface is perhaps the most important and most useful structure in Crest. ```stream``` objects generate audio, in the form of a linear array of floating-point samples allocated on the heap. They can either generate this audio through synthesis, or through pulling audio data from sources like audio files or other ```stream```s.

There are currently 3 existing implementations of ```stream``` objects in Crest, not including the ones that may be found in the ```tests``` of this library. Each of these library-built ```stream``` objects are for the latter use case: pulling data out of compressed and uncompressed audio files. Each ```stream``` has it's own ```audio_format``` structure, and it's paramount that this structure is defined upon initalizing a ```stream```.

```wav_stream``` pulls data out of uncompressed ```.wav``` files. The declarations for this ```stream``` implementation are found within ```include/wavutils.h```.

```flac_stream``` pulls data out of losslessly compressed ```.flac``` files using a custom decoder. The declarations for this ```stream``` implementation are found within ```include/flacutils.h```.

```vorbis_stream``` pulls data out of a lossy compressed ```.ogg``` vorbis file using the decoder provided with the ```stb_vorbis.c``` file. The declarations for this ```stream``` implementation are found within ```include/vorbisutils.h```.

Because ```stream``` is an abstract class, each implementation of the interface will require the following functions to be implemented:

- ```stream* stream::copy()``` - Returns a pointer to a copy of the ```stream```. Every time a ```stream``` is added to an ```audio_source```, this function is called to add a dynamically-allocated copy of the ```stream``` to the source, which will then be deleted when the ```stream``` ends or otherwise expires.

- ```void stream::reset()``` - "Resets" the ```stream```. If the usage of the ```stream``` involves, for example, reading a file, then this function would presumably reset the reader and return it to the start of the file. This function is used when the ```stream::pull(...)``` function produces a positive ```terminate``` result and the ```CREST_STREAMFLAG_LOOP``` flag is set to true. Otherwise, this function can and should be used at your discretion.

- ```float* stream::pull(UINT32 request_frames, UINT32* length, bool* terminate)``` - This is the function responsible for generating the audio data. Implementations of this function can vary wildly depending on your use case, but the result must be a dynamically-allocated array of floats, with each float representing an audio sample. Channels should be interleaved such that, in a 2-channel output, a sample from channel 1 in a given frame is followed immediately by the corresponding sample for channel 2 in that frame.
  - ```request_frames``` determines the total amount of audio data, **in audio frames**, that the ```audio_source``` or the user is requesting to retrieve from the ```stream```.
  - ```length``` is then updated with the actual size of the data, **in audio samples** (number of frames * number of channels). This number should correspond with the true number of floats in the array.
  - The ```terminate``` reference determines whether an ```audio_source``` should remove the ```stream``` after this round of data has been pulled. In the case of reading audio files, the ```terminate``` field will most likely be updated to ```true``` if the built-in reader reaches the end of the file during the current pull operation.

Here are the other public functions of a ```stream``` class that do not need implementations, as well as it's constructor and destructor:

```stream::stream()``` does nothing other than initialize all ```stream``` flags to ```false```. It's recommended to still initialize subclasses with this constructor to prevent uninitialized flags.

```stream::~stream()``` is a virtual destructor that is currently empty.

```void stream::add_transform(stream_transform* transform)``` - Adds a ```stream_transform``` object to the stream, which is applied to the audio data obtained with the ```pull``` function after all necessary sample rate and channel conversions have been applied.

```bool stream::get_flag(UINT16 flag) const``` - Returns the value of one of the ```stream``` flags, corresponding with the ```flag``` parameter.

```audio_format stream::get_format() const``` - Returns the ```audio_format``` struct being used by the stream.

```float* stream::pull_data(UINT32 request_frames, UINT32* length, bool* terminate)``` - This can be thought of as the *"parent"* function to the subclass's implementation of the ```pull``` function. The ```pull_data``` function is used by ```audio_source```s when pulling data out of ```stream```s, and essentially calls the ```pull``` function, and then converts the resulting data from the ```audio_format``` specified in the ```stream``` to the ```audio_format``` that the client uses, and then finally applies any ```stream_transform```s that are being stored within the ```stream``` to the resulting converted data.

```void stream::set_flag(UINT16 flag, bool val)``` - Sets the ```stream``` flag ```flag``` to the value ```val```. Currently, there is only 1 ```stream``` flag:
- ```CREST_STREAMFLAG_LOOP``` - ```audio_source```s will reset the ```stream``` after the ```terminate``` field is positive, rather than deleting it. Used with the current implementations of the file reader streams.

Finally, here are the protected fields of ```stream``` that will probably need to be accessed when implementing a ```stream``` subclass:

```audio_format format``` - The audio format of the stream. This format determines, importantly, the sample rate and number of channels produced by the audio stream, which in turn determines how the sample rate and channel conversions in ```pull_data``` will function, so it's very important to define this structure when initializing the ```stream``` subclass.

```std::vector<stream_transform*> transforms``` - A vector containing the list of ```stream_transform``` objects which this stream is currently using.

```bool flags[1]``` - The values of all the flags that this stream is currently using.

## ```stream_transform```
The ```stream_transform``` object allows you to define an additional layer of audio processing that occurs after the data has been pulled and converted to the client format. These transformations to the data have to be one-to-one, meaning that the size of the array of data must be preserved before and after the transformation.

There are currently two simple ```stream_transform``` implementations in the library, to give you an idea of how they work:

- ```effect_volume``` - Adjusts the volume/gain of a signal by multiplying each sample by a volume constant.

- ```effect_balance``` - Similar to ```effect_volume```, but there are now two constants which determine the volume in the left and right channels of a 2-channel stereo output.

The declarations for both of these ```stream_transform```s can be found in ```include/effects.h```.

Like ```stream```s, ```stream_transform```s are abstract classes, but there is only one function that a subclass implementation must implement:

- ```void stream_transform::transform(float* data, UINT32 length, audio_format format)``` - Transforms the array ```data```, which has a length ```length``` and an audio format defined by ```format```.

Additionally, here are the public functions of any ```stream_transform``` object:

- ```stream_transform::stream_transform()``` - Sets the ```change_buffer``` to 0.

- ```stream_transform::~stream_transform()``` - Empty destructor.

- ```float stream_transform::get_change_buffer_length() const``` - Returns the ```change_buffer``` for the transform.

- ```void stream_transform::set_change_buffer_length(float value)``` - Sets the ```change_buffer``` for the stream to ```value```.

The ```change_buffer``` is currently the only protected value for a ```stream_transform```. How it's used, or whether it's used at all, is up to the developer, but in the two implemented ```stream_transform``` objects it's used to determine how quickly a change in the volume/balance levels can occur during real-time playback, in seconds.

For example, a change buffer of 2 means that setting the volume with ```effect_volume::set_volume(float value)``` will cause a shift in the volume from the current value to the new value that will take 2 seconds of playback.

## ```audio_source```
The ```audio_source``` acts as a bridge between user-created instances of ```stream```s and Crest's audio threads. Once they're registered with Crest using ```register_source```, Crest will begin listening for any ```stream```s added to the ```audio_source```, and begin processing them to be outputted to the OS's sound buffer.

Unlike ```stream```s and ```stream_transform```s, the ```audio_source``` class is concrete.

- ```audio_source::audio_source()``` - Empty constructor.

- ```audio_source::~audio_source()``` - Removes and deletes all streams that are currently tied to the ```audio_source``` object.

- ```void audio_source::add_stream(stream* stream)``` - Copies the stream ```*stream``` and adds it to the ```streams``` vector. Assuming no errors with the stream, the ```audio_source``` should begin playing the stream afterward.

- ```bool audio_source::contains_stream(stream* stream)``` - Returns true if ```streams``` contains a copy of ```stream```.

- ```UINT32 audio_source::get_number_of_streams() const``` - Returns the number of ```stream```s in the ```streams``` vector.

- ```bool is_playing() const``` - Returns true if the ```streams``` vector has at least 1 active stream. Note that this means that even if the final output of the ```audio_source``` is digital silence, this will still return true if the former case is true.

- ```void audio_source::remove_streams(stream* stream)``` - Removes and deletes all instances (copies) of ```stream``` from the ```streams``` vector.

The ```audio_source``` class's main job is to stores a vector of ```stream```s, and pull data from the streams automatically at the request of the Crest processing thread.