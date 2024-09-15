#ifndef _CREST_VORBISUTILS_H_
#define _CREST_VORBISUTILS_H_

#include "../crest.h"

#include <string>

namespace crest
{
    class CREST_LIB vorbis_stream : public stream
    {
        public:
            vorbis_stream(std::string filepath);
            ~vorbis_stream();

            stream* copy();
            float* pull(UINT32 request, UINT32* length, bool* eof);
            void reset();
        private:
            std::string filepath;
            void* v; //stb_vorbis pointer.

            UINT32 buffer_offset, buffer_size;
            float** buffer;
            
            bool can_read_file, buffer_is_initialized, end_of_file;

            bool initialize();
            bool decode_next_frame();
    };
};

#endif