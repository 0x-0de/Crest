#include "vorbisutils.h"

#include <iostream>

#include "../lib/stb_vorbis.c"

crest::vorbis_stream::vorbis_stream(std::string filepath) : stream(), filepath(filepath)
{
    can_read_file = initialize();
    usable = can_read_file;
}

crest::vorbis_stream::~vorbis_stream()
{
    if(can_read_file)
    {
        stb_vorbis_close((stb_vorbis*) v);
    }
}

bool crest::vorbis_stream::initialize()
{
    buffer_offset = 0;

    int error;
    v = stb_vorbis_open_filename(filepath.c_str(), &error, NULL);

    if(v == NULL)
    {
        std::cerr << "[ERR] Cannot open file: " << filepath << std::endl;
        return false;
    }

    format.channels = ((stb_vorbis*) v)->channels;
    format.sample_rate = ((stb_vorbis*) v)->sample_rate;
    format.bit_depth = 16;

    buffer_is_initialized = false;
    end_of_file = false;
    return true;
}

crest::stream* crest::vorbis_stream::copy()
{
    return new vorbis_stream(filepath);
}

float* crest::vorbis_stream::pull(UINT32 request, UINT32* length, bool* eof)
{
    if(!buffer_is_initialized)
    {
        end_of_file = !decode_next_frame();
    }

    float* data = new float[request * format.channels];
    UINT32 data_offset = 0;

    for(unsigned int i = 0; i < request; i++)
    {
        if(buffer_offset == buffer_size)
        {
            end_of_file = !decode_next_frame();
            buffer_offset = 0;
        }

        if(end_of_file) break;

        for(int i = 0; i < 2; i++)
        {
            data[data_offset] = buffer[i][buffer_offset];
            data_offset++;
        }
        buffer_offset++;
    }

    if(end_of_file)
    {
        for(UINT32 i = data_offset; i < request * format.channels; i++)
        {
            data[i] = 0;
        }
    }

    *eof = end_of_file;
    return data;
}

void crest::vorbis_stream::reset()
{
    if(can_read_file) stb_vorbis_close((stb_vorbis*) v);
    can_read_file = initialize();
}

bool crest::vorbis_stream::decode_next_frame()
{
    int channels;

    buffer_size = stb_vorbis_get_frame_float((stb_vorbis*) v, &channels, &buffer);
    //else std::cout << "Length: " << current_data_buffer_size << ", " << "Channels: " << channels << std::endl;

    buffer_is_initialized = true;
    return buffer_size != 0;
}