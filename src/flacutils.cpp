#include "flacutils.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#define PRINT_FLAC_DEBUG false
//#define MEASURE_FLAC_PERFORMANCE_LPC

char flip_byte(char a)
{
    char b = 0;
    for(int i = 0; i < 8; i++)
    {
        if((a & (1 << (7 - i))))
            b |= (1 << i);
    }
    return b;
}

/*

//-------------------------------------------------------------------------------
//Bitreader class and functions.
//-------------------------------------------------------------------------------

bit_reader::bit_reader(const char* filepath, UINT16 cache_size) : filepath(filepath), cache_size(cache_size)
{
    reader = fopen(filepath, "rb");

    UINT16 s = 1;
    for(int i = 0; i < 16; i++)
    {
        if(cache_size < s)
        {
            std::cout << "[INF] Bit reader cache needs to be a power of 2. Setting to next smallest power: " << (s >> 1) << " (was " << cache_size << ")." << std::endl;
            this->cache_size = (s >> 1);
            break;
        }
        else if(cache_size == s) break;

        s <<= 1;
    }

    if(reader == NULL)
    {
        std::cerr << "[ERR] Bit reader couldn't open " << filepath << std::endl;
    }
    else
    {
        fseek(reader, 0, SEEK_END);
        filesize = ftell(reader) * 8;
        fseek(reader, 0, SEEK_SET);

        bit_index = 0;

        cache = nullptr;
    }

    cache_bit_size = this->cache_size << 3;
}

bit_reader::~bit_reader()
{
    fclose(reader);
    if(cache != nullptr) clear_cache();
}

void bit_reader::clear_cache()
{
    if(cache != nullptr) delete[] cache;
    cache = nullptr;
}

void bit_reader::read_cache()
{
    int cache_index = (bit_index / cache_bit_size);
    int byte_index = cache_index * cache_size;
    
    fseek(reader, byte_index, SEEK_SET);

    cache = new char[cache_size];

    int remaining = (filesize >> 3) - byte_index;
    if(remaining > cache_size) remaining = cache_size;
    fread(cache, sizeof(char), remaining, reader);
}

void bit_reader::set_bit_index(UINT64 index)
{
    bit_index = index;
    clear_cache();
}

void bit_reader::increment_bit_index(INT64 value)
{
    bit_index += value;
    clear_cache();
}

void bit_reader::jump_to_next_byte()
{
    int bit_offset = bit_index % 8;
    if(bit_offset != 0)
    {
        bit_index += 8 - bit_offset;
    }
    clear_cache();
}

inline char bit_reader::read_next_byte()
{
    int byte_index = (bit_index & (cache_bit_size - 1)) >> 3;
    return cache[byte_index];
}

bool bit_reader::read_bit()
{
    UINT8 bit_offset = bit_index & 7;

    if((bit_index & (cache_bit_size - 1)) == 0)
    {
        clear_cache();
        read_cache();
    }

    char byte = read_next_byte();

    UINT8 b = 7 - bit_offset;
    bool bit = (byte >> b) & 1;

    bit_index++;

    return bit;
}

void bit_reader::read_bits(bool* buffer, UINT8 length)
{
    UINT16 current_bit = 0;
    UINT8 bit_offset = bit_index & 7;

    if((bit_index & (cache_bit_size - 1)) == 0)
        clear_cache();

    if(cache == nullptr)
    {
        read_cache();
    }

    char byte = read_next_byte();

    //TODO: Maybe optimize this loop so it goes in bytes?
    while(current_bit < length)
    {
        if(bit_offset == 8)
        {
            int byte_index = bit_index >> 3;
            if(byte_index % cache_size == 0)
            {
                clear_cache();
                read_cache();
            }
            byte = read_next_byte();
            bit_offset = 0;
        }

        UINT8 b = 7 - bit_offset;
        buffer[current_bit] = (byte >> b) & 1;

        bit_offset++;
        bit_index++;

        current_bit++;
    }
}

INT64 bit_reader::read_as_int(const UINT8 bit_length, bool is_signed)
{
    /*
    Loop over bit_length/8 + 1 amount of bytes.
    Mask the start and end bytes with bit masks, then shift and add to sum.
    For the start byte, shift down to where the bit offset becomes the 1-digit.
    For each subsequent byte, shift up the add to byte offset, then add 8 for each additional after the last.
    For the ending byte, mask it by doing an & operation to 0 out the remaining digits in the byte.
    Return sum.
    */

    /*
    INT64 num = 0;

    UINT8 bit_offset = bit_index & 7;
    UINT8 bit_count = 0;

    UINT8 first_byte_shift = bit_offset;
    bool first_byte = true;
    bool last_byte = false;
    bool is_negative = false;

    UINT8 bit_next;

    if(bit_length == 14) std::cout << "Bit offset: " << (int) bit_offset << std::endl;

    while(bit_count < bit_length)
    {
        if(bit_index >= cache_bit_size)
            clear_cache();

        if(cache == nullptr)
        {
            read_cache();
        }

        UINT8 byte = read_next_byte();

        bit_next = 8;
        if(bit_count + bit_next >= bit_length)
        {
            bit_next = bit_length - bit_count;
            last_byte = true;
        }

        if(first_byte)
        {
            byte >>= first_byte_shift;
            bit_next = 8 - first_byte_shift;
        }
        if(last_byte)
        {
            byte &= ((1 << bit_next) - 1);
            is_negative = 1 & (byte >> bit_next);
        }

        bit_count += bit_next;
        num |= ((UINT64) byte) << (bit_length - bit_count);
        bit_index += bit_next;

        if(bit_length == 14)
        std::cout << num << ", " << (((UINT64) byte) << (bit_length - bit_count)) << ", " << (UINT64) byte << ", " << (bit_length - bit_count) << std::endl;

        first_byte = false;
    }

    if(bit_length == 14) std::cout << num << std::endl;

    if(is_signed)
    {
        if(is_negative)
        {
            INT64 cap = (1 << (bit_length - 1));
            INT64 correction = (1 << bit_length) * -1;
            if(num > cap) num += correction;
        }
    }
    */

/*

    bool buffer[bit_length];
    read_bits(buffer, bit_length);

    INT64 num = 0;

    for(UINT8 i = 0; i < bit_length; i++)
    {
        int index = (bit_length - i) - 1;
        num |= buffer[i] << index;
    }

    if(is_signed)
    {
        int cap = (1 << (bit_length - 1));
        int correction = (1 << bit_length) * -1;

        if(num >= cap)
        {
            num += correction;
        }
    }
    
    return num;
}

UINT64 bit_reader::read_unary(bool end_bit)
{
    UINT64 result = 0;
    bool b = read_bit();

    while(b != end_bit)
    {
        result++;
        b = read_bit();
    }

    return result;
}

UINT64 bit_reader::read_utf8()
{
    UINT64 size = read_unary(0);

    UINT64 result = 0;

    if(size == 0)
    {
        result = read_as_int(7, false);
        return result;
    }

    for(UINT64 i = 0; i < size; i++)
    {
        result <<= 6;
        if(i == 0)
        {
            UINT8 n = read_as_int((8 - size) - 1, false);
            result += n;
        }
        else
        {
            UINT8 h = read_as_int(2, false);
            UINT8 n = read_as_int(6, false);

            if(h == 2) result += n;
        }
    }

    return result;
}

*/

#define BIT_READER_BUFFER_SIZE 1024

/*
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
*/

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
    }
}

void bit_reader::jump_to_next_byte()
{
    uint64 bit = bit_index & 7;
    uint64 jump = bit == 0 ? 0 : (8 - bit);

    increment_bit_index(jump);
}

bool bit_reader::readable() const
{
    return can_read;
}

bool bit_reader::read_bit()
{
    if(buffer_bit_index >= (BIT_READER_BUFFER_SIZE << 3))
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

std::string bit_reader::get_filepath() const
{
    return filepath;
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

    buffer_bit_index &= ((BIT_READER_BUFFER_SIZE << 3) - 1);
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

    read_buffer();
}

//-------------------------------------------------------------------------------
//Decoding.
//-------------------------------------------------------------------------------

bool read_flac_tag(bit_reader* reader)
{
    int tag = reader->read_as_int(32, false);
    return tag == 1716281667;
}

bool read_metadata_block_streaminfo(bit_reader* reader, UINT32 length, crest::audio_format* format)
{
    reader->read_as_int(16, false); //min_blocksize
    reader->read_as_int(16, false); //max_blocksize

    reader->read_as_int(24, false); //min_framesize
    reader->read_as_int(24, false); //max_framesize

    format->sample_rate = reader->read_as_int(20, false);

    format->channels = reader->read_as_int(3, false) + 1;
    format->bit_depth = reader->read_as_int(5, false) + 1;

    format->num_samples = reader->read_as_int(36, false);

    reader->increment_bit_index(128);

    return true;
}

bool read_metadata_block(bit_reader* reader, crest::audio_format* format, bool* last)
{
    if(PRINT_FLAC_DEBUG) std::cout << std::hex << reader->get_bit_index() / 8 << std::endl;

    *last = reader->read_as_int(1, false);
    int type = reader->read_as_int(7, false);

    UINT32 length = reader->read_as_int(24, false);

    switch(type)
    {
        case 0:
            read_metadata_block_streaminfo(reader, length, format);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            reader->increment_bit_index(length * 8);
            break;
        default:
            std::cerr << "[ERR] Unknown metadata block type: " << type << std::endl;
            return false;
    }

    return true;
}

bool read_metadata(bit_reader* reader, crest::audio_format* format)
{
    bool last = false;
    while(!last)
    {
        if(!read_metadata_block(reader, format, &last))
        {
            return false;
        }
    }
    return true;
}

bool read_residual_constant(bit_reader* reader, FLAC_frame* frame, int* values)
{
    int value = reader->read_as_int(frame->sample_size, true);
    for(unsigned int i = 0; i < frame->blocksize; i++)
    {
        if(values != nullptr) values[i] = value;
    }
    return true;
}

bool read_residual_verbatim(bit_reader* reader, FLAC_frame* frame, int* values)
{
    for(unsigned int i = 0; i < frame->blocksize; i++)
    {
        if(values != nullptr) values[i] = reader->read_as_int(frame->sample_size, true);
        else reader->increment_bit_index(frame->sample_size);
    }
    return true;
}

bool read_residual_lpc(bit_reader* reader, FLAC_frame* frame, int order, int shift, int* warmup_samples, int* coefficients, int* values)
{
    int method = reader->read_as_int(2, false);

    if(method > 1)
    {
        std::cerr << "Residual coding method is reserved." << std::endl;
        return false;
    }

    #ifdef MEASURE_FLAC_PERFORMANCE_LPC
        double setup_length = 0;
        double division_length = 0;
        double value_length = 0;
    #endif

    int partition_order = reader->read_as_int(4, false);
    int partitions = (1 << partition_order);

    int index = 0;
    for(int i = 0; i < order; i++)
    {
        if(values != nullptr) values[index] = warmup_samples[i];
        index++;
    }
    
    for(int i = 0; i < partitions; i++)
    {
        int parameter = reader->read_as_int(method == 1 ? 5 : 4, false);
        if(parameter == (method == 1 ? 31 : 15))
        {
            std::cerr << "Escape code used." << std::endl;
            return false;
        }

        int samples = (i == 0) ? frame->blocksize / partitions - order : frame->blocksize / partitions;

        for(int j = 0; j < samples; j++)
        {
            #ifdef MEASURE_FLAC_PERFORMANCE_LPC
                auto start = std::chrono::steady_clock::now();
            #endif

            int quotient = reader->read_unary(1);
            int remainder;

            if(parameter == 0) remainder = 0;
            else remainder = reader->read_as_int(parameter, false);

            #ifdef MEASURE_FLAC_PERFORMANCE_LPC
                auto end = std::chrono::steady_clock::now();

                std::chrono::duration<double, std::micro> elapsed = end - start;
                setup_length += elapsed.count();

                start = std::chrono::steady_clock::now();
            #endif

            int residual = (quotient << parameter) + remainder;
            bool negative = residual & 1;

            //Zig-zag encoded.
            residual += negative;
            residual *= negative ? -1 : 1;
            residual >>= 1;

            #ifdef MEASURE_FLAC_PERFORMANCE_LPC
                end = std::chrono::steady_clock::now();

                elapsed = end - start;
                division_length += elapsed.count();
            #endif

            if(values != nullptr)
            {
                #ifdef MEASURE_FLAC_PERFORMANCE_LPC
                    start = std::chrono::steady_clock::now();
                #endif
                int decoded_pcm = 0;
                for(int j = 0; j < order; j++)
                {
                    decoded_pcm += coefficients[j] * values[index - j - 1];
                }
                decoded_pcm >>= shift;
                decoded_pcm += residual;
                values[index] = decoded_pcm;
                #ifdef MEASURE_FLAC_PERFORMANCE_LPC
                    end = std::chrono::steady_clock::now();

                    elapsed = end - start;
                    value_length += elapsed.count();
                #endif
            }

            index++;
        }
    }

    #ifdef MEASURE_FLAC_PERFORMANCE_LPC
        std::cout << "Setup: " << setup_length << ", Division: " << division_length << ", Values: " << value_length << ", Total: " <<
                     (setup_length + division_length + value_length) << std::endl;
    #endif

    return true;
}

bool read_subframe(bit_reader* reader, FLAC_frame* frame, int subframe_number, int** values)
{
    if(PRINT_FLAC_DEBUG) std::cout << "  Current bit index: " << std::hex << reader->get_bit_index() << std::dec << " (" << reader->get_bit_index() << ")" << std::endl;
    bool pad = reader->read_as_int(1, false);

    int warmup_sample_bit_depth = frame->sample_size;
    if(frame->channel_assignment_hint == 8 || frame->channel_assignment_hint == 10)
    {
        if(subframe_number == 1) warmup_sample_bit_depth++;
    }
    else if(frame->channel_assignment_hint == 9)
    {
        if(subframe_number == 0) warmup_sample_bit_depth++;
    }

    if(pad)
    {
        std::cerr << "0-bit padding at the start of subframe not present." << std::endl;
        return false;
    }

    int subframe_code = reader->read_as_int(6, false);

    int wasted_bits = reader->read_as_int(1, false);
    if(wasted_bits)
    {
        std::cerr << "Wasted bits not supported yet." << std::endl;
        return false;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "    Subframe code: " << subframe_code << std::endl;

    if(subframe_code == 0)
    {
        //Constant.
        read_residual_constant(reader, frame, values == nullptr ? nullptr : values[subframe_number]);
    }
    else if(subframe_code == 1)
    {
        //Verbatim.
        read_residual_verbatim(reader, frame, values == nullptr ? nullptr : values[subframe_number]);
    }
    else if(subframe_code >= 8 && subframe_code <= 15)
    {
        //Fixed coefficients.
        int order = subframe_code & 7;
        if(order > 4)
        {
            std::cerr << "Invalid fixed predictor order." << std::endl;
            return false;
        }

        int* warmup_samples = nullptr;
        int* coefficients = nullptr;

        if(order > 0)
        {
            warmup_samples = new int[order];
            if(PRINT_FLAC_DEBUG) std::cout << "    Warm-up samples: ";
            for(int i = 0; i < order; i++)
            {
                warmup_samples[i] = reader->read_as_int(warmup_sample_bit_depth, true);
                if(PRINT_FLAC_DEBUG) std::cout << warmup_samples[i] << " ";
            }
            if(PRINT_FLAC_DEBUG) std::cout << std::endl;

            coefficients = new int[order];
            switch(order)
            {
                case 1:
                    coefficients[0] = 1;
                    break;
                case 2:
                    coefficients[0] = 2;
                    coefficients[1] = -1;
                    break;
                case 3:
                    coefficients[0] = 3;
                    coefficients[1] = -3;
                    coefficients[2] = 1;
                    break;
                case 4:
                    coefficients[0] = 4;
                    coefficients[1] = -6;
                    coefficients[2] = 4;
                    coefficients[3] = -1;
                    break;
            }
        }

        read_residual_lpc(reader, frame, order, 0, warmup_samples, coefficients, values == nullptr ? nullptr : values[subframe_number]);

        if(warmup_samples != nullptr) delete[] warmup_samples;
        if(coefficients != nullptr) delete[] coefficients;
    }
    else if(subframe_code >= 32)
    {
        //LPC.
        int order = (subframe_code & 31) + 1;

        int* warmup_samples = new int[order];
        if(PRINT_FLAC_DEBUG) std::cout << "    Warm-up samples: ";
        for(int i = 0; i < order; i++)
        {
            warmup_samples[i] = reader->read_as_int(warmup_sample_bit_depth, true);
            if(PRINT_FLAC_DEBUG) std::cout << warmup_samples[i] << " ";
        }
        if(PRINT_FLAC_DEBUG) std::cout << std::endl;

        int lpc_precision = reader->read_as_int(4, false) + 1;
        if(lpc_precision == 16)
        {
            std::cerr << "Invalid LPC precision. (16)" << std::endl;
            return false;
        }

        int lpc_shift = reader->read_as_int(5, true);

        if(PRINT_FLAC_DEBUG) std::cout << "    LPC precision: " << lpc_precision << std::endl;
        if(PRINT_FLAC_DEBUG) std::cout << "    LPC shift: " << lpc_shift << std::endl;

        int* coefficients = new int[order];
        if(PRINT_FLAC_DEBUG) std::cout << "    Coefficients: ";
        for(int i = 0; i < order; i++)
        {
            coefficients[i] = reader->read_as_int(lpc_precision, true);
            if(PRINT_FLAC_DEBUG) std::cout << coefficients[i] << " ";
        }
        if(PRINT_FLAC_DEBUG) std::cout << std::endl;

        read_residual_lpc(reader, frame, order, lpc_shift, warmup_samples, coefficients, values == nullptr ? nullptr : values[subframe_number]);

        if(warmup_samples != nullptr) delete[] warmup_samples;
        if(coefficients != nullptr) delete[] coefficients;
    }
    else
    {
        std::cerr << "Invalid subframe code: " << subframe_code << std::endl;
        return false;
    }

    return true;
}

bool scan_frame(bit_reader* reader, crest::audio_format* format, FLAC_frame* frame, UINT64* frame_offset)
{
    if(PRINT_FLAC_DEBUG) std::cout << "Reading frame..." << std::dec << std::endl;
    if(PRINT_FLAC_DEBUG) std::cout << "Current byte index: " << std::hex << reader->get_bit_index() / 8 << std::dec << " (" << reader->get_bit_index() / 8 << ")" << std::endl; 

    UINT16 sync = reader->read_as_int(14, false);
    if(sync != 16382)
    {
        std::cerr << "Sync code not present at the start of frame." << std::endl;
        return false;
    }

    if(reader->read_bit())
    {
        std::cerr << "Zero-bit padding not present at the start of frame." << std::endl;
        return false;
    }

    reader->read_bit(); //Blocking strategy - determines the meaning of the number of the frame. We don't care about it.
    int blocksize_code = reader->read_as_int(4, false);

    switch(blocksize_code)
    {
        case 0:
            return false;
        case 1:
            frame->blocksize = 192;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            frame->blocksize = 576 * (1 << (blocksize_code - 2));
            break;
        case 6:
        case 7:
            break;
        default:
            frame->blocksize = 256 * (1 << (blocksize_code - 8));
            break;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "  Blocksize: " << frame->blocksize << " (code: " << blocksize_code << ")" << std::endl;

    int samplerate_code = reader->read_as_int(4, false);

    switch(samplerate_code)
    {
        case 0:
            frame->sample_rate = format->sample_rate;
            break;
        case 1:
            frame->sample_rate = 88200;
            break;
        case 2:
            frame->sample_rate = 176400;
            break;
        case 3:
            frame->sample_rate = 192000;
            break;
        case 4:
            frame->sample_rate = 8000;
            break;
        case 5:
            frame->sample_rate = 16000;
            break;
        case 6:
            frame->sample_rate = 22050;
            break;
        case 7:
            frame->sample_rate = 24000;
            break;
        case 8:
            frame->sample_rate = 32000;
            break;
        case 9:
            frame->sample_rate = 44100;
            break;
        case 10:
            frame->sample_rate = 48000;
            break;
        case 11:
            frame->sample_rate = 96000;
            break;
        default:
            break;
        case 15:
            std::cerr << "Sample rate uses resvered value. Frame is invalid." << std::endl;
            return false;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "  Sample rate: " << frame->sample_rate << " (code: " << samplerate_code << ")" << std::endl;

    frame->channel_assignment_hint = reader->read_as_int(4, false);

    int channels;

    switch(frame->channel_assignment_hint)
    {
        default:
            channels = frame->channel_assignment_hint + 1;
            break;
        case 8:
        case 9:
        case 10:
            channels = 2;
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            std::cerr << "Channel assignment hint uses reserved value." << std::endl;
            return false;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "  Channels: " << channels << " (code: " << (int) frame->channel_assignment_hint << ")" << std::endl;

    int samplesize_code = reader->read_as_int(3, false);

    switch(samplesize_code)
    {
        case 0:
            frame->sample_size = format->bit_depth;
            break;
        case 1:
            frame->sample_size = 8;
            break;
        case 2:
            frame->sample_size = 12;
            break;
        case 3:
            return false;
        case 4:
            frame->sample_size = 16;
            break;
        case 5:
            frame->sample_size = 20;
            break;
        case 6:
            frame->sample_size = 24;
            break;
        case 7:
            frame->sample_size = 32;
            break;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "  Sample size: " << (int) frame->sample_size << " (code: " << samplesize_code << ")" << std::endl;

    if(reader->read_as_int(1, false)) return false;

    frame->number = reader->read_utf8();

    if(blocksize_code == 6)
    {
        frame->blocksize = reader->read_as_int(8, false) + 1;
    }
    else if(blocksize_code == 7)
    {
        frame->blocksize = reader->read_as_int(16, false) + 1;
    }

    if(PRINT_FLAC_DEBUG) std::cout << "  Frame number: " << frame->number << std::endl;

    if(samplerate_code == 12)
    {
        frame->sample_rate = reader->read_as_int(8, false);
    }
    else if(samplerate_code == 13)
    {
        frame->sample_rate = reader->read_as_int(16, false);
    }
    else if(samplerate_code == 14)
    {
        frame->sample_rate = reader->read_as_int(16, false) * 10;
    }

    int crc = reader->read_as_int(8, false);
    if(PRINT_FLAC_DEBUG) std::cout << "  CRC: " << crc << std::endl;

    frame->subframe_file_offset = reader->get_bit_index() / 8;
    frame->frame_offset = *frame_offset;

    *frame_offset += frame->blocksize;

    for(int i = 0; i < channels; i++)
    {
        if(PRINT_FLAC_DEBUG) std::cout << "  Reading subframe " << i << "..." << std::endl;
        if(!read_subframe(reader, frame, i, nullptr)) return false;
    }

    reader->jump_to_next_byte();
    reader->increment_bit_index(16);

    return true;
}

//-------------------------------------------------------------------------------
//flac_stream
//-------------------------------------------------------------------------------

crest::flac_stream::flac_stream(std::string filepath) : stream(), filepath(filepath)
{
    reader = new bit_reader(filepath);
    can_read_file = initialize();
    usable = can_read_file;

    read_frame_index = 0;
    read_sample_index = 0;
}

crest::flac_stream::~flac_stream()
{
    delete reader;
}

crest::stream* crest::flac_stream::copy()
{
    stream* s = new flac_stream(filepath);

    for(int i = 0; i < 1; i++)
    {
        s->set_flag(i, flags[i]);
    }

    for(UINT16 i = 0; i < transforms.size(); i++)
    {
        s->add_transform(transforms[i]);
    }

    return s;
}

bool crest::flac_stream::initialize()
{
    if(!reader->readable())
    {
        return false;
    }

    if(!read_flac_tag(reader))
    {
        std::cerr << "[ERR] fLaC tag not present at the start of the file. Are you sure this is a valid FLAC file?" << std::endl;
        std::cerr << " --> " << reader->get_filepath() << std::endl;
        return false;
    }

    if(!read_metadata(reader, &format))
    {
        std::cerr << "[ERR] Failed to read metadata." << std::endl;
        return false;
    }

    UINT64 frame_offset = 0;
    UINT64 file_bit_size = reader->get_file_size() << 3;
    while(reader->get_bit_index() < file_bit_size)
    {
        FLAC_frame frame;
        if(!scan_frame(reader, &format, &frame, &frame_offset))
        {
            std::cerr << "[ERR] Failed to scan frame." << std::endl;
            return false;
        }
        frames.push_back(frame);
    }
    
    return true;
}

float* crest::flac_stream::pull(UINT32 request, UINT32* length, bool* eof)
{
    if(!can_read_file) return nullptr;

    UINT32 samples = request * format.channels;
    FLOAT* data = new FLOAT[samples];

    *length = samples;

    *eof = false;

    for(UINT32 i = 0; i < samples; i++)
        data[i] = 0;

    int* decoded_pcm = nullptr;
    int decoded_pcm_length;

    int div = (1 << (format.bit_depth - 1));

    for(UINT32 i = 0; i < samples; i += format.channels)
    {
        if(read_sample_index >= frames[read_frame_index].blocksize)
        {
            read_frame_index++;
            delete[] decoded_pcm;
            decoded_pcm = nullptr;
            read_sample_index = 0;

            if(read_frame_index == frames.size())
            {
                *eof = true;
                *length = i - format.channels;
                break;
            }
        }

        if(decoded_pcm == nullptr)
        {
            if(!read_frame(read_frame_index, &decoded_pcm, &decoded_pcm_length))
            {
                std::cerr << "[ERR] Failed to read .flac frame during stream: " << filepath << std::endl;
                return nullptr;
            }
        }

        for(int j = 0; j < format.channels; j++)
        {
            int pcm = decoded_pcm[read_sample_index * format.channels + j];

            float pcm_f = (float) pcm / div;
            data[i + j] = pcm_f;
        }

        read_sample_index++;
    }

    delete[] decoded_pcm;
    read_sample_index -= 1;
    
    return data;
}

bool crest::flac_stream::read_frame(unsigned int frame_index, int** decoded_pcm, int* decoded_pcm_length)
{
    FLAC_frame frame = frames[frame_index];
    int channels = format.channels;

    reader->set_bit_index(frame.subframe_file_offset * 8);

    int** values = new int*[channels];
    for(int i = 0; i < channels; i++)
        values[i] = new int[frame.blocksize];

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < channels; i++)
    {
        if(PRINT_FLAC_DEBUG) std::cout << "  Reading subframe " << i << "..." << std::endl;
        if(!read_subframe(reader, &frame, i, values)) return false;
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::micro> elapsed = end - start;

    *decoded_pcm_length = frame.blocksize * channels;
    *decoded_pcm = new int[frame.blocksize * channels];

    int pcm_index = 0;
    for(int i = 0; i < frame.blocksize; i++)
    {
        for(int j = 0; j < channels; j++)
        {
            (*decoded_pcm)[pcm_index + j] = values[j][i];
        }

        if(frame.channel_assignment_hint == 8)
        {
            (*decoded_pcm)[pcm_index + 1] = (*decoded_pcm)[pcm_index] - (*decoded_pcm)[pcm_index + 1];
        }
        else if(frame.channel_assignment_hint == 9)
        {
            (*decoded_pcm)[pcm_index] = (*decoded_pcm)[pcm_index] + (*decoded_pcm)[pcm_index + 1];
        }
        else if(frame.channel_assignment_hint == 10)
        {
            int mid = (*decoded_pcm)[pcm_index] << 1;
            int side = (*decoded_pcm)[pcm_index + 1];

            if(side % 2 == 1) mid++;

            (*decoded_pcm)[pcm_index] = (mid + side) >> 1;
            (*decoded_pcm)[pcm_index + 1] = (mid - side) >> 1;
        }

        pcm_index += channels;
    }

    reader->jump_to_next_byte();
    reader->increment_bit_index(16);

    return true;
}

void crest::flac_stream::reset()
{
    read_frame_index = 0;
    read_sample_index = 0;
}