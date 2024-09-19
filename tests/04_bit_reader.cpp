#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#define BIT_READER_BUFFER_SIZE 1024

typedef long long int64;
typedef unsigned long long uint64;

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

The example reads the file ./tests/audio/flac/subset/01 - blocksize 4096.flac. See the instructions
located in the empty folder to gain access to the file.
*/

class bit_reader
{
    public:
        bit_reader(std::string filepath);
        ~bit_reader();

        uint64 get_bit_index() const;
        uint64 get_file_size() const;

        void increment_bit_index(uint64 amount);
        void jump_to_next_byte();

        uint64 read_as_int(uint64 length, bool is_signed);

        bool read_bit();
        bool* read_bits(uint64 length);

        void read_buffer();

        uint64 read_unary(bool end_bit);
        uint64 read_utf8();

        void set_bit_index(uint64 index);
    private:
        std::ifstream* reader;
        bool can_read;

        char* buffer;
        uint64 bit_index, buffer_bit_index, byte_length;
};

bit_reader::bit_reader(std::string filepath)
{
    reader = new std::ifstream(filepath.c_str(), std::ios::binary);

    if(!reader->good())
    {
        std::cerr << "[ERR] Cannot read file: " << filepath << std::endl;
        can_read = false;
    }
    else
    {
        reader->seekg(0, reader->end);
        byte_length = reader->tellg();
        reader->seekg(0, reader->beg);

        buffer = new char[BIT_READER_BUFFER_SIZE];

        bit_index = 0;
        buffer_bit_index = 0;

        can_read = true;

        read_buffer();
    }
}

bit_reader::~bit_reader()
{
    delete[] buffer;
}

void bit_reader::increment_bit_index(uint64 amount)
{
    bit_index += amount;
    buffer_bit_index += amount;

    if(buffer_bit_index >= (BIT_READER_BUFFER_SIZE << 3))
    {
        read_buffer();
        buffer_bit_index &= ((BIT_READER_BUFFER_SIZE << 3) - 1);
    }
}

void bit_reader::jump_to_next_byte()
{
    uint64 bit = bit_index & 7;
    uint64 jump = bit == 0 ? 0 : (8 - bit);

    increment_bit_index(jump);
}

bool bit_reader::read_bit()
{
    if(buffer_bit_index == (BIT_READER_BUFFER_SIZE << 3))
    {
        read_buffer();
    }

    uint64 bit = bit_index & 7;
    uint64 byte = buffer_bit_index >> 3;

    bit_index++;
    buffer_bit_index++;

    return ((buffer[byte] >> (7 - bit)) & 1);
}

uint64 bit_reader::get_bit_index() const
{
    return bit_index;
}

uint64 bit_reader::get_file_size() const
{
    return byte_length;
}

bool* bit_reader::read_bits(uint64 length)
{
    bool* buff = new bool[length];
    for(uint64 i = 0; i < length; i++)
    {
        buff[i] = read_bit();
    }
    return buff;
}

void bit_reader::read_buffer()
{
    uint64 start = ((bit_index >> 3) / BIT_READER_BUFFER_SIZE) * BIT_READER_BUFFER_SIZE;
    reader->seekg(start, reader->beg);

    uint64 length = std::min(BIT_READER_BUFFER_SIZE, (signed) (byte_length - (bit_index >> 3)));
    reader->read(buffer, length);
    buffer_bit_index = 0;
}

uint64 bit_reader::read_as_int(uint64 length, bool is_signed)
{
    int64 result = 0;
    for(int i = (signed) length - 1; i > -1; i--)
        result |= (int) read_bit() << i;

    if(is_signed)
    {
        int64 cap = ((int64) 1 << (length - 1));
        int64 correction = ((int64) 1 << length) * -1;

        if(result >= cap)
        {
            result += correction;
        }
    }

    return result;
}

uint64 bit_reader::read_unary(bool end_bit)
{
    uint64 result = 0;
    bool b = read_bit();

    while(b != end_bit)
    {
        result++;
        b = read_bit();
    }

    return result;
}

uint64 bit_reader::read_utf8()
{
    uint64 size = read_unary(0);

    uint64 result = 0;

    if(size == 0)
    {
        result = read_as_int(7, false);
        return result;
    }

    for(uint64 i = 0; i < size; i++)
    {
        result <<= 6;
        if(i == 0)
        {
            unsigned char n = read_as_int((8 - size) - 1, false);
            result += n;
        }
        else
        {
            unsigned char h = read_as_int(2, false);
            unsigned char n = read_as_int(6, false);

            if(h == 2) result += n;
        }
    }

    return result;
}

void bit_reader::set_bit_index(uint64 index)
{
    bit_index = index;
    buffer_bit_index = index & ((BIT_READER_BUFFER_SIZE << 3) - 1);
}

int main()
{
    bit_reader reader("./tests/audio/flac/subset/01 - blocksize 4096.flac");
    unsigned int flac_sign = reader.read_as_int(32, true);
    int last_metadata = reader.read_bit();
    int block_type = reader.read_as_int(7, false);
    int block_length = reader.read_as_int(24, false);
    int min_blocksize = reader.read_as_int(16, false);
    int max_blocksize = reader.read_as_int(16, false);
    int min_framesize = reader.read_as_int(24, false);
    int max_framesize = reader.read_as_int(24, false);
    int sample_rate = reader.read_as_int(20, false);
    int channels = reader.read_as_int(3, false);
    int bit_depth = reader.read_as_int(5, false);
    int total_samples = reader.read_as_int(36, false);

    std::cout << "FLAC sign: " << flac_sign << " (should be 1716281667)." << std::endl;
    std::cout << "Last metadata block? " << last_metadata << " (should be 0)." << std::endl;
    std::cout << "Block type: " << block_type << " (should be 0)." << std::endl;
    std::cout << "   Block length: " << block_length << " (should be 34)." << std::endl;
    std::cout << "Min blocksize: " << min_blocksize << " (should be 4096)." << std::endl;
    std::cout << "Max blocksize: " << max_blocksize << " (should be 4096)." << std::endl;
    std::cout << "Min framesize: " << min_framesize << " (should be 2445)." << std::endl;
    std::cout << "Max framesize: " << max_framesize << " (should be 9278)." << std::endl;
    std::cout << "Sample rate: " << sample_rate << " (should be 44100)." << std::endl;
    std::cout << "Channels: " << channels + 1 << " (should be 2)." << std::endl;
    std::cout << "Bit depth: " << bit_depth + 1 << " (should be 16)." << std::endl;
    std::cout << "Total samples: " << total_samples << " (should be 308700)." << std::endl;

    return 0;
}