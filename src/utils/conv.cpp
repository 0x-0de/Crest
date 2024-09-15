#include "conv.h"

int conv_bytes_to_int(char* bytes, unsigned int length, bool is_signed)
{
    int result = 0;
    for(unsigned int i = 0; i < length; i++)
    {
        int r = bytes[i];
        if(r < 0) r += 256;
        result += r << (8 * i);
    }

    if(is_signed && length < 4)
    {
        int correction = 1 << (length * 8);
        int signed_cap = 1 << (length * 8 - 1);

        if(result >= signed_cap)
        {
            result -= correction;
        }
    }

    return result;
}

float conv_bytes_to_float(char* bytes, unsigned int length)
{
    int raw = conv_bytes_to_int(bytes, length, true);
    int signed_cap = 1 << (length * 8 - 1);

    return (float) raw / signed_cap;
}