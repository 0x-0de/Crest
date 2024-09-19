#ifndef _CREST_FLACUTILS_H_
#define _CREST_FLACUTILS_H_

#include "crest.h"

#include <fstream>
#include <string>
#include <vector>

struct FLAC_frame
{
    UINT64 subframe_file_offset;
    UINT64 frame_offset;

    UINT16 blocksize;
    UINT32 sample_rate;
    UINT8 channel_assignment_hint;
    UINT8 sample_size;
    UINT64 number;
};

typedef long long int64;
typedef unsigned long long uint64;

class bit_reader
{
    public:
        bit_reader(std::string filepath);
        ~bit_reader();

        uint64 get_bit_index() const;
        std::string get_filepath() const;
        uint64 get_file_size() const;

        void increment_bit_index(uint64 amount);
        void jump_to_next_byte();

        bool readable() const;

        uint64 read_as_int(uint64 length, bool is_signed);

        bool read_bit();
        bool* read_bits(uint64 length);

        void read_buffer();

        uint64 read_unary(bool end_bit);
        uint64 read_utf8();

        void set_bit_index(uint64 index);
    private:
        std::string filepath;
        std::ifstream* reader;
        bool can_read;

        char* buffer;
        uint64 bit_index, buffer_bit_index, byte_length;
};

namespace crest
{
    class CREST_LIB flac_stream : public stream
    {
        public:
            flac_stream(std::string filepath);
            ~flac_stream();

            stream* copy();
            float* pull(UINT32 request, UINT32* length, bool* eof);
            void reset();
        private:
            std::string filepath;
            unsigned int read_frame_index, read_sample_index;
            
            bool can_read_file;

            bool initialize();
            bool read_frame(unsigned int frame_index, int** decoded_pcm, int* decoded_pcm_length);

            std::vector<FLAC_frame> frames;
            bit_reader* reader;
    };
}

#endif