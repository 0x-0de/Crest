#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#define BIT_READER_BUFFER_SIZE 1024

void print_bits(bool* bits, unsigned int length)
{
    for(int i = (signed) length - 1; i > -1; i--)
    {
        if((i & 7) == 7) std::cout << " ";
        std::cout << bits[i];
    }
    std::cout << std::endl;
}

/*
This test is mainly for a revised bit reader which I hope to implement in the current .FLAC decoder.
The current (as of writing) bit reader has proven to be too slow to process high-resolution audio files.

The example reads the file ./tests/audio/flac/subset/01 - blocksize 4096.flac.


*/

class bit_reader
{
    public:
        bit_reader(std::string filepath);
        ~bit_reader();

        void read_buffer();

        bool read_bit();
        bool* read_bits(unsigned int length);

        unsigned int read_as_int(unsigned int length);
    private:
        std::ifstream* reader;
        bool can_read;

        char* buffer;
        unsigned int bit_index, buffer_bit_index;
        unsigned int byte_length;
};

bit_reader::bit_reader(std::string filepath)
{
    reader = new std::ifstream(filepath.c_str(), std::ios::binary);

    if(!reader->good())
    {
        std::cerr << "[ERR] Cannot read file: " << filepath << std::endl;
        can_read = false;
    }

    reader->seekg(0, reader->end);
    byte_length = reader->tellg();
    reader->seekg(0, reader->beg);

    buffer = new char[BIT_READER_BUFFER_SIZE];

    bit_index = 0;
    buffer_bit_index = 0;

    can_read = true;

    read_buffer();
}

bit_reader::~bit_reader()
{
    delete[] buffer;
}

bool bit_reader::read_bit()
{
    if(buffer_bit_index == (BIT_READER_BUFFER_SIZE << 3))
    {
        read_buffer();
    }

    unsigned int bit = bit_index & 7;
    unsigned int byte = buffer_bit_index >> 3;

    bit_index++;
    buffer_bit_index++;

    return ((buffer[byte] >> (7 - bit)) & 1);
}

bool* bit_reader::read_bits(unsigned int length)
{
    bool* buff = new bool[length];
    for(unsigned int i = 0; i < length; i++)
    {
        buff[i] = read_bit();
    }
    return buff;
}

void bit_reader::read_buffer()
{
    unsigned int length = std::min(BIT_READER_BUFFER_SIZE, (signed) (byte_length - (bit_index >> 3)));
    reader->read(buffer, length);
    buffer_bit_index = 0;
}

unsigned int bit_reader::read_as_int(unsigned int length)
{
    unsigned int result = 0;
    for(int i = (signed) length - 1; i > -1; i--)
        result |= (int) read_bit() << i;
    return result;
}

int main()
{
    bit_reader reader("./tests/audio/flac/subset/01 - blocksize 4096.flac");
    unsigned int test = reader.read_as_int(32);

    std::cout << test << std::endl;

    //delete[] test;

    return 0;
}