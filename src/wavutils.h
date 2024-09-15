#ifndef _CREST_WAVUTILS_H_
#define _CREST_WAVUTILS_H_

#include "crest.h"

#include <string>

namespace crest
{
    class CREST_LIB wav_stream : public stream
    {
        public:
            wav_stream(std::string filepath);

            stream* copy();
            float* pull(UINT32 request, UINT32* length, bool* eof);
            void reset();

            void dft();
        private:
            std::string filepath;
            unsigned int read_offset, data_size, data_offset;
            
            bool can_read_file;

            bool initialize();
    };
};

#endif