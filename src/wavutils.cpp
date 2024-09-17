#include "wavutils.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include "conv.h"

bool goto_byte_seq(const char* seq, std::ifstream& reader)
{
    int length = strlen(seq);
    char buffer[length + 1];
    buffer[length] = 0;

    reader.read(buffer, length);

    if(strcmp(buffer, seq) == 0)
    {
        return true;
    }
    else
    {
        while(strcmp(buffer, seq) != 0)
        {
            if(reader.eof()) return false;

            for(int i = 0; i < length - 1; i++)
            {
                buffer[i] = buffer[i + 1];
            }

            reader.read(buffer + length - 1, 1);
        }
    }

    return true;
}

crest::wav_stream::wav_stream(std::string filepath) : stream(), filepath(filepath)
{
    can_read_file = initialize();
    usable = can_read_file;
}

bool crest::wav_stream::initialize()
{
    std::ifstream reader(filepath, std::ios::binary);

    if(!reader.is_open())
    {
        std::cerr << "[CREST] Cannot open .wav file: " << filepath << std::endl;
        std::cerr << "   Are you sure this file exists?" << std::endl;
        return false;
    }

    char buffer[5];
    reader.read(buffer, 4);
    buffer[4] = 0;

    if(strcmp(buffer, "RIFF") != 0)
    {
        std::cerr << "[CREST] Failed to read .wav file: " << filepath << std::endl;
        std::cerr << "   File doesn't have a RIFF header. Are you sure this is a .wav file?" << std::endl;
        return false;
    }

    reader.read(buffer, 4); //RIFF size, we're ignoring this.
    reader.read(buffer, 4);

    if(strcmp(buffer, "WAVE") != 0)
    {
        std::cerr << "[CREST] Failed to read .wav file: " << filepath << std::endl;
        std::cerr << "   File doesn't have a WAVE header. Are you sure this is a .wav file?" << std::endl;
        return false;
    }

    if(!goto_byte_seq("fmt ", reader))
    {
        std::cerr << "[CREST] Failed to read .wav file: " << filepath << std::endl;
        std::cerr << "   File's format metadata section is either misplaced or missing." << std::endl;
        return false;
    }

    reader.read(buffer, 4); //Size of the 'fmt ' chunk. Skipped.
    reader.read(buffer, 2);

    if(buffer[0] != 1 || buffer[1] != 0)
    {
        std::cerr << "[CREST] Failed to read .wav file: " << filepath << std::endl;
        std::cerr << "   File uses an unsupported audio format (only mode 1, integer, is supported)." << std::endl;
        return false;
    }

    reader.read(buffer, 2);
    format.channels = conv_bytes_to_int(buffer, 2, false);;

    reader.read(buffer, 4);
    format.sample_rate = conv_bytes_to_int(buffer, 4, false);;

    reader.read(buffer, 4); //SR*BD*C/8, don't care.
    reader.read(buffer, 2); //BD*C/8, don't care.

    reader.read(buffer, 2);
    format.bit_depth = conv_bytes_to_int(buffer, 2, false);;
    
    if(!goto_byte_seq("data", reader))
    {
        std::cerr << "[CREST] Failed to read .wav file: " << filepath << std::endl;
        std::cerr << "   File's data section is either misplaced or missing." << std::endl;
        return false;
    }

    reader.read(buffer, 4);
    data_size = conv_bytes_to_int(buffer, 4, false);
    data_offset = reader.tellg();

    read_offset = 0;

    reader.close();
    return true;
}

crest::stream* crest::wav_stream::copy()
{
    stream* s = new wav_stream(filepath);

    for(int i = 0; i < 1; i++)
    {
        s->set_flag(i, s->get_flag(i));
    }

    for(UINT16 i = 0; i < transforms.size(); i++)
    {
        s->add_transform(transforms[i]);
    }

    return s;
}

float* crest::wav_stream::pull(UINT32 request, UINT32* length, bool* eof)
{
    if(!can_read_file) return nullptr;
    
    std::ifstream reader(filepath, std::ios::binary);
    reader.seekg(data_offset + read_offset, reader.beg);

    unsigned int data_length = request * format.channels;
    float* data = new float[data_length];

    *length = data_length;
    *eof = false;

    for(unsigned int i = 0; i < data_length; i++)
    {
        data[i] = 0;
    }

    int bytes_per_sample = format.bit_depth / 8;
    unsigned int data_raw_size = data_length * bytes_per_sample;

    if(read_offset + data_raw_size >= data_size)
    {
        *eof = true;
        data_raw_size -= ((read_offset + data_raw_size) - data_size);
        *length -= ((read_offset + data_raw_size) - data_size) / 4;
    }

    char* raw_data = new char[data_raw_size];

    reader.read(raw_data, data_raw_size);
    reader.close();

    unsigned int data_index = 0;
    for(unsigned int i = 0; i < data_raw_size; i += bytes_per_sample)
    {
        data[data_index] = conv_bytes_to_float(raw_data + i, bytes_per_sample);
        data_index++;
    }

    delete[] raw_data;

    read_offset += data_raw_size;
    return data;
}

void crest::wav_stream::reset()
{
    read_offset = 0;
}

void crest::wav_stream::dft()
{
    UINT32 num_samples = data_size / (format.channels * format.bit_depth / 8);

    UINT32 ds;
    bool eof;
    float* data = pull(num_samples, &ds, &eof);

    std::cout << data_size << ", " << ds << std::endl;

    if(!eof)
    {
        std::cerr << "Arrgh! You didn't read the whole file!" << std::endl;
    }

    for(double i = 0; i < format.sample_rate / 2; i++) //Current frequency. We sample up to the Nyquist limit (sampling frequency / 2).
    {
        double frequency_a = 0, frequency_b = 0;
        UINT32 ind = 0;
        for(UINT32 j = 0; j < num_samples * 2 - 1; j++) //Current sample.
        {
            float d = (j & 1) ? (data[ind] + data[ind + format.channels]) / 2 : data[ind];
            double exp = (-6.2831853 * (double) (i * j)) / (double) format.sample_rate;
            frequency_a += d * std::cos(exp) * 2;
            frequency_b -= d * std::sin(exp) * 2;

            ind += (j & 1) ? 0 : format.channels;
        }
        frequency_a /= num_samples * 2;
        frequency_b /= num_samples * 2;
        double magnitude = std::sqrt(std::pow(frequency_a, 2) + std::pow(frequency_b, 2));
        if(i < 1000) std::cout << i << ": " << magnitude << std::endl;
    }

    delete[] data;
}