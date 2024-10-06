#include "crest.h"

#include "comdef.h"

#include "mmdeviceapi.h"
#include "audioclient.h"
#include "timeapi.h"
#include "initguid.h"
#include "Functiondiscoverykeys_devpkey.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#define CREST_WINDOWS_SLEEP_RESOLUTION 5

//#define CREST_OUTPUT_OUTLIER_BUFFER_DIFFERENCES

#define VERIFY(hr) if(!check_result(hr)) return hr

bool check_result(HRESULT result)
{
    if(FAILED(result))
    {
        _com_error err(result);
        std::cout << err.ErrorMessage() << std::endl;
        return false;
    }

    return true;
}

static bool active = false;
static bool paused = false;
static bool halted = false;

static bool thread_is_paused = false;
static bool should_halt = false;

static IMMDeviceEnumerator* device_enumerator;
static IMMDevice* device;
static IAudioClient* audio_client;
static IAudioRenderClient* render_client;
static crest::audio_format client_format;

static UINT32 req_buffer_ms;
static UINT32 buffer_frame_size;

static std::thread* audio_thread;
static std::thread* processing_thread;

static BYTE* audio_data;
static UINT32 audio_data_lid, audio_data_remove;
static UINT32 client_frame_size;
static bool ready_to_process;

HRESULT init_wasapi_device();
HRESULT halt_playback(bool release_all);

class CREST_WASAPI_DeviceNotificationListener : public IMMNotificationClient
{
    //IUnknown methods. Just using the https://learn.microsoft.com/en-us/windows/win32/coreaudio/audio-session-events implemention of these.

    LONG ref;

    public:
        CREST_WASAPI_DeviceNotificationListener()
        {
            ref = 0;
        }

        virtual ~CREST_WASAPI_DeviceNotificationListener() {}

        ULONG STDMETHODCALLTYPE AddRef()
        {
            return InterlockedIncrement(&ref);
        }

        ULONG STDMETHODCALLTYPE Release()
        {
            ULONG ulRef = InterlockedDecrement(&ref);
            if(0 == ulRef)
            {
                delete this;
            }
            return ulRef;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
        {
            //According to the implementation, this is should be 'IID_IUnknown == riid'. Changed it to this for the time being... not sure if it works.
            if(__uuidof(IUnknown) == riid)
            {
                AddRef();
                *ppvInterface = (IUnknown*) this;
            }
            else if(__uuidof(IMMNotificationClient) == riid)
            {
                AddRef();
                *ppvInterface = (IMMNotificationClient*) this;
            }
            else
            {
                *ppvInterface = NULL;
                return E_NOINTERFACE;
            }
            return S_OK;
        }

        //IMMNotificationClient methods.

        HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR device_id)
        {
            /*
            IMMDevice* sw_device;
            IPropertyStore* property_store;

            device_enumerator->GetDevice(device_id, &sw_device);

            sw_device->OpenPropertyStore(STGM_READ, &property_store);

            PROPVARIANT var_name;
            PropVariantInit(&var_name);

            HRESULT hr;

            hr = property_store->GetValue(PKEY_Device_FriendlyName, &var_name);
            VERIFY(hr);

            if(var_name.vt != VT_EMPTY)
            {
                std::wcout << var_name.pwszVal << std::endl;
            }
            */

            if(role != 0) return S_OK;

            paused = true;

            while(!thread_is_paused) {} //Waiting for the audio thread to pause...
            //std::cout << "All threads paused. Commencing..." << std::endl;

            HRESULT hr;

            //std::cout << "Stopping the audio client..." << std::endl;

            should_halt = true;
            while(!halted) {}

            device_enumerator->GetDevice(device_id, &device);

            //std::cout << "Initializing the new endpoint..." << std::endl;

            hr = init_wasapi_device();
            VERIFY(hr);

            //std::cout << "Restarting the stream..." << std::endl;

            hr = audio_client->Start();
            VERIFY(hr);

            //std::cout << "Success! Unpausing the threads." << std::endl;

            should_halt = false;
            paused = false;

            return S_OK;
        }

        HRESULT OnDeviceAdded(LPCWSTR device_id)
        {
            return S_OK;
        }

        HRESULT OnDeviceRemoved(LPCWSTR device_id)
        {
            return S_OK;
        }

        HRESULT OnDeviceStateChanged(LPCWSTR device_id, DWORD new_state)
        {
            std::cout << device_id << ", " << new_state << std::endl;
            return S_OK;
        }

        HRESULT OnPropertyValueChanged(LPCWSTR device_id, const PROPERTYKEY key)
        {
            return S_OK;
        }
};

static CREST_WASAPI_DeviceNotificationListener crest_dnl;

#ifdef CREST_OUTPUT_OUTLIER_BUFFER_DIFFERENCES
    static std::ofstream* debug_writer;
#endif

static std::vector<crest::audio_source*> sources;

crest::stream::stream()
{
    usable = true;

    for(UINT32 i = 0; i < 1; i++)
        flags[i] = false;
}

crest::stream::~stream() {}

void crest::stream::add_transform(crest::stream_transform* transform)
{
    transforms.push_back(transform);
}

bool crest::stream::get_flag(UINT16 flag) const
{
    return flags[flag];
}

crest::audio_format crest::stream::get_format() const
{
    return format;
}

bool crest::stream::is_usable() const
{
    return usable;
}

float* crest::stream::pull_data(UINT32 request_frames, UINT32* length, bool* terminate)
{
    float sample_rate_ratio = (float) format.sample_rate / (float) client_format.sample_rate;
    unsigned int actual_request = request_frames * sample_rate_ratio + 1;

    UINT8 stream_channels = format.channels;
    UINT8 client_channels = client_format.channels;

    UINT32 samples = request_frames * client_channels;

    float* data = new float[samples];
    float* unconverted_data = pull(actual_request, length, terminate);
    
    if(flags[CREST_STREAMFLAG_LOOP] && *terminate)
    {
        reset();
        unsigned int offset = *length;

        unsigned int new_length = actual_request - *length / client_channels;
        float* loop = pull(new_length, length, terminate);

        for(unsigned int i = 0; i < new_length * client_channels; i++)
        {
            unconverted_data[i + offset] = loop[i];
        }
    }

    //Convert channels.

    float* channel_data;

    if(stream_channels != client_channels)
    {
        channel_data = new float[samples];
        for(UINT32 i = 0; i < actual_request; i++)
        {
            for(UINT8 j = 0; j < client_channels; j++)
            {
                channel_data[i * client_channels + j] = unconverted_data[i * stream_channels + (j % stream_channels)];
            }
        }
        delete[] unconverted_data;
    }
    else
    {
        channel_data = unconverted_data;
    }

    //Convert sample rate.

    float point = 0;
    for(UINT32 i = 0; i < samples; i += client_channels)
    {
        for(UINT8 j = 0; j < client_channels; j++)
        {
            int s = std::floor(point) * client_channels;
            int e = s + client_channels;

            float start = channel_data[s + j];
            float end = channel_data[e + j];

            float p = point - std::floor(point);
            data[i + j] = (end - start) * p + start;
        }
        point += sample_rate_ratio;
    }

    *length = samples;

    for(UINT32 i = 0; i < transforms.size(); i++)
    {
        transforms[i]->transform(data, *length, client_format);
    }

    delete[] channel_data;
    return data;
}

void crest::stream::set_flag(UINT16 flag, bool val)
{
    flags[flag] = val;
}

crest::stream_transform::stream_transform()
{
    change_buffer = 0;
}

crest::stream_transform::~stream_transform() {}

void crest::stream_transform::set_change_buffer_length(float value)
{
    change_buffer = value;
}

crest::audio_source::audio_source()
{
    
}

crest::audio_source::~audio_source()
{
    while(streams.size() > 0)
    {
        streams.erase(streams.begin());
    }
}

void crest::audio_source::add_stream(stream* s)
{
    if(!s->is_usable())
    {
        std::cerr << "[CREST] Cannot add unusable stream to audio_source." << std::endl;
        return;
    }
    
    originals.push_back(s);

    stream* str = s->copy();
    streams.push_back(str);
}

void crest::audio_source::add_transform(stream_transform* t)
{
    transforms.push_back(t);
}

bool crest::audio_source::contains_stream(stream* stream)
{
    for(UINT32 i = 0; i < originals.size(); i++)
    {
        if(originals[i] == stream) return true;
    }

    return false;
}

UINT32 crest::audio_source::get_number_of_streams() const
{
    return streams.size();
}

bool crest::audio_source::is_playing() const
{
    return streams.size() > 0;
}

void crest::audio_source::remove_streams(stream* s)
{
    for(UINT32 i = 0; i < streams.size(); i++)
    {
        if(originals[i] == s)
        {
            delete streams[i];
            originals.erase(originals.begin() + i);
            streams.erase(streams.begin() + i);
            i--;
        }
    }
}

void crest::audio_source::read(FLOAT** data, UINT32 request_frames)
{
    UINT32 samples = request_frames * client_format.channels;
    *data = new FLOAT[samples];

    for(UINT32 i = 0; i < samples; i++)
        (*data)[i] = 0;

    for(UINT64 i = 0; i < streams.size(); i++)
    {
        UINT32 length;
        bool end_stream;

        float* read = streams[i]->pull_data(request_frames, &length, &end_stream);

        if(length != samples)
        {
            std::cout << "[CREST] Warning: length of pulled audio data (" << length << ") does not match requested size (" << samples << ")." << std::endl;
        }

        for(UINT32 j = 0; j < samples; j++)
        {
            (*data)[j] += read[j];
        }
        delete[] read;
        if(end_stream)
        {
            delete streams[i];
            streams.erase(streams.begin() + i);
            originals.erase(originals.begin() + i);
            i--;
        }
    }

    for(UINT32 i = 0; i < transforms.size(); i++)
    {
        transforms[i]->transform(*data, samples, client_format);
    }
}

//Writing the next audio output.
void write_audio_to_buffer(BYTE* data, UINT32 buffer_size)
{
    //if(buffer_size != 480 && buffer_size != 960)
    //std::cout << buffer_size << std::endl;

    UINT32 channels = client_format.channels;
    UINT32 sample_size = client_format.bit_depth;
    
    UINT32 samples = buffer_size * channels;

    FLOAT* float_buffer = new FLOAT[samples];

    for(UINT32 i = 0; i < samples; i++)
    {
        float_buffer[i] = 0;
    }
    
    for(UINT32 i = 0; i < sources.size(); i++)
    {
        FLOAT* source_data;
        sources[i]->read(&source_data, buffer_size);
        for(UINT32 j = 0; j < samples; j++)
        {
            float_buffer[j] += source_data[j];
        }
        delete[] source_data;
    }

    #ifdef CREST_OUTPUT_OUTLIER_BUFFER_DIFFERENCES
        for(UINT32 i = 2; i < samples; i++)
        {
            float diff = float_buffer[i] - float_buffer[i - 2];
            if(abs(diff) > 0.1f)
                (*debug_writer) << diff << '\n';
        }
    #endif

    for(UINT32 i = 0; i < samples; i++)
    {
        UINT32 val;
        switch(sample_size)
        {
            case 8:
                val = float_buffer[i] * (1 << 8);
                data[i] = val;
                break;
            case 16:
                val = float_buffer[i] * (1 << 16);
                data[i * 2] = val;
                data[i * 2 + 1] = val >> 8;
                break;
            case 24:
                val = float_buffer[i] * (1 << 24);
                data[i * 3] = val;
                data[i * 3 + 1] = val >> 8;
                data[i * 3 + 2] = val >> 16;
                break;
            case 32:
                memcpy(data + i * 4, float_buffer + i, 4);
                break;
            default:
                break;
        }
    }

    delete[] float_buffer;
}

HRESULT init_wasapi_device()
{
    HRESULT hr;

    //We can then set up our audio client by calling "activate" on this function.
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &audio_client);
    VERIFY(hr);

    //Our audio client has a specific format which we can record with a WAVEFORMATEX struct. This contains information about the format of an audio stream,
    //such as the number of channels being used, number of samples being played per second (sample rate), number of bits per sample (bit depth), and more.
    WAVEFORMATEX* format;
    hr = audio_client->GetMixFormat(&format);
    VERIFY(hr);

    //Initializes the audio stream.
    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, req_buffer_ms * 10000, 0, format, NULL);
    VERIFY(hr);

    client_format.sample_rate = format->nSamplesPerSec;
    client_format.bit_depth = format->wBitsPerSample;
    client_format.channels = format->nChannels;
    client_format.num_samples = 0;

    //Returns the maximum size of the allocated buffer.
    hr = audio_client->GetBufferSize(&buffer_frame_size);
    VERIFY(hr);

    if(active)
    {
        delete[] audio_data;
    }

    client_frame_size = client_format.bit_depth / 8 * client_format.channels;
    audio_data = new BYTE[buffer_frame_size * client_frame_size];

    audio_data_lid = 0;
    audio_data_remove = 0;

    //std::cout << "   Buffer size: " << buffer_frame_size << " frames" << std::endl;
    //std::cout << "     Requested: " << req_buffer_ms << "ms" << std::endl;
    //std::cout << "   Sample rate: " << client_format.sample_rate << std::endl;

    //The audio render client enables a client to actually write to a rendering endpoint buffer.
    hr = audio_client->GetService(__uuidof(IAudioRenderClient), (void**) &render_client);
    VERIFY(hr);

    return S_OK;
}

HRESULT halt_playback(bool release_all)
{
    HRESULT hr;

    //Stop the audio stream.
    hr = audio_client->Stop();
    VERIFY(hr);

    //Release all COM resources.
    if(release_all) render_client->Release();
    audio_client->Release();
    device->Release();

    return S_OK;
}

//Initializing Windows' WASAPI, which is used for audio input and output.
HRESULT wasapi_init()
{    
    HRESULT hr;

    //Creating the IMMDeviceEnumerator object.
    //The IMMDeviceEnumerator is used for providing multimedia device resources, although for the moment it only enumerates audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &device_enumerator);
    VERIFY(hr);

    //Registering an instance of the CREST_WASAPI_DeviceNotificationListener class.
    hr = device_enumerator->RegisterEndpointNotificationCallback(&crest_dnl);

    //Using the IMMDeviceEnumerator, we can locate the default audio endpoint.
    //We set the IMMDevice to this endpoint.
    hr = device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    VERIFY(hr);

    return init_wasapi_device();
}

void process_queue()
{
    UINT32 process_step_frames = 100;
    while(active)
    {
        if(!ready_to_process || paused) continue;

        //Remove any already in-use data.
        if(audio_data_remove != 0)
        {
            if(audio_data_remove > audio_data_lid)
            {
                std::cout << "Removing " << audio_data_remove << " frames of data (current lid is " << audio_data_lid << ")." << std::endl;
                for(UINT32 i = 0; i < audio_data_lid; i++)
                {
                    UINT32 frame_offset = i * client_frame_size;
                    for(UINT32 j = 0; j < client_frame_size; j++)
                    {
                        audio_data[frame_offset + j] = 0;
                    }
                }
                audio_data_lid = 0;
            }
            else
            {
                for(UINT32 i = 0; i < audio_data_lid - audio_data_remove; i++)
                {
                    UINT32 frame_offset_to = i * client_frame_size;
                    UINT32 frame_offset_from = (i + audio_data_remove) * client_frame_size;
                    for(UINT32 j = 0; j < client_frame_size; j++)
                    {
                        audio_data[frame_offset_to + j] = audio_data[frame_offset_from + j];
                    }
                }
                audio_data_lid -= audio_data_remove;
            }

            audio_data_remove = 0;
        }
        
        //Write new data.
        if(audio_data_lid < buffer_frame_size)
        {
            UINT32 distance_to_cap = buffer_frame_size - audio_data_lid;
            UINT32 actual_process_step = std::min(process_step_frames, distance_to_cap);

            //std::cout << "New data write. Current lid: " << audio_data_lid << ", total size: " << buffer_frame_size << ", processing " << actual_process_step << " frames." << std::endl;

            write_audio_to_buffer(audio_data + audio_data_lid * client_frame_size, actual_process_step);
            audio_data_lid += actual_process_step;
        }
    }
}

//Opens and maintains the audio rendering thread, used by WASAPI.
HRESULT thread_open()
{
    ready_to_process = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(req_buffer_ms * 2)); //Giving the processing thread a head start.

    //Starting the audio client, this will begin playback.
    HRESULT hr = audio_client->Start();
    VERIFY(hr);

    BYTE* data;
    UINT32 used_buffer_size;

    MMRESULT result = timeBeginPeriod(CREST_WINDOWS_SLEEP_RESOLUTION);
    if(result == TIMERR_NOCANDO)
    {
        std::cout << "Timer cannot be set to minimum. Using default..." << std::endl;
    }

    while(active)
    {
        if(paused)
        {
            thread_is_paused = true;
            if(should_halt && !halted)
            {
                halt_playback(false);
                halted = true;
            }
            continue;
        }
        else
        {
            thread_is_paused = false;
            halted = false;
        }

        //Get size of remaining buffer space.
        hr = audio_client->GetCurrentPadding(&used_buffer_size);
        VERIFY(hr);
        unsigned int remaining_buffer_size = buffer_frame_size - used_buffer_size;

        //Getting a pointer to the data, so that we can write to it.
        hr = render_client->GetBuffer(remaining_buffer_size, &data);
        VERIFY(hr);

        //Continue writing to that space.
        UINT32 requested_bytes = remaining_buffer_size * client_frame_size;
        for(UINT32 i = 0; i < requested_bytes; i++)
            data[i] = audio_data[i];
        audio_data_remove = remaining_buffer_size;

        //Release it to the audio client.
        hr = render_client->ReleaseBuffer(remaining_buffer_size, 0);
        VERIFY(hr);

        Sleep(req_buffer_ms / 2);
    }

    timeEndPeriod(CREST_WINDOWS_SLEEP_RESOLUTION);

    hr = halt_playback(true);
    VERIFY(hr);

    device_enumerator->UnregisterEndpointNotificationCallback(&crest_dnl);
    device_enumerator->Release();

    return hr;
}

//Initializes and opens the audio buffer.
void crest::init(UINT32 buffer_ms)
{
    paused = false;

    CoInitialize(NULL);

    /*
    TIMECAPS t;
    timeGetDevCaps(&t, 8);

    std::cout << t.wPeriodMin << ", " << t.wPeriodMax << std::endl;
    */

    req_buffer_ms = buffer_ms;

    ready_to_process = false;

    #ifdef CREST_OUTPUT_OUTLIER_BUFFER_DIFFERENCES
        debug_writer = new std::ofstream("test.txt");
    #endif

    if(FAILED(wasapi_init()))
    {
        return;
    }

    active = true;
    audio_thread = new std::thread(thread_open);
    processing_thread = new std::thread(process_queue);
}

void crest::terminate()
{
    active = false;
    ready_to_process = false;
    paused = false;

    #ifdef CREST_OUTPUT_OUTLIER_BUFFER_DIFFERENCES
        delete debug_writer;
    #endif

    processing_thread->join();
    delete processing_thread;

    audio_thread->join();
    delete audio_thread;

    delete[] audio_data;
}

void crest::register_source(crest::audio_source* source)
{
    sources.push_back(source);
}

void crest::remove_source(crest::audio_source* source)
{
    for(unsigned int i = 0; i < sources.size(); i++)
    {
        if(sources[i] == source)
        {
            sources.erase(sources.begin() + i);
            break;
        }
    }
}

crest::audio_format crest::get_client_audio_format()
{
    return client_format;
}