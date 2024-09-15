#ifndef _CREST_CONV_H_
#define _CREST_CONV_H_

#include "../crest.h"

CREST_LIB int conv_bytes_to_int(char* bytes, unsigned int length, bool is_signed);
CREST_LIB float conv_bytes_to_float(char* bytes, unsigned int length);

#endif