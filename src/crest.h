#ifndef _CREST_H_
#define _CREST_H_

#include "windows.h"

#include <vector>

#ifdef CREST_EXPORT
    #define CREST_LIB __declspec(dllexport)
#else
    #define CREST_LIB
#endif

#define CREST_STREAMFLAG_LOOP 0

namespace crest
{
    struct CREST_LIB audio_format
    {
        UINT32 sample_rate;
        UINT8 bit_depth, channels;

        UINT64 num_samples;
    };

    class CREST_LIB stream_transform
    {
        public:
            stream_transform();
            virtual ~stream_transform();

            float get_change_buffer_length() const { return change_buffer; }
            void set_change_buffer_length(float value);

            virtual void transform(float* data, UINT32 length, audio_format format) = 0;
        protected:
            float change_buffer;
    };

    class CREST_LIB stream
    {
        public:
            stream();
            virtual ~stream();

            void add_transform(stream_transform* transform);

            bool get_flag(UINT16 flag) const;
            audio_format get_format() const;

            virtual float* pull(UINT32 request_frames, UINT32* length, bool* terminate) = 0;
            float* pull_data(UINT32 request_frames, UINT32* length, bool* terminate);

            virtual stream* copy() = 0;
            virtual void reset() = 0;

            void set_flag(UINT16 flag, bool val);
        protected:
            audio_format format;
            std::vector<stream_transform*> transforms;

            bool flags[1];
    };

    class CREST_LIB audio_source
    {
        public:
            audio_source();
            ~audio_source();

            //Adds an audio stream to the audio source.
            void add_stream(stream* stream);

            //Checks if a stream is currently contained by the audio source.
            bool contains_stream(stream* stream);

            //Obtains data from an audio source, puts in location specified by data, and also writes the amount read to read_length.
            void read(FLOAT** data, UINT32 requested_read_samples);

            //Removes an audio stream from the source.
            void remove_stream(stream* stream);

            UINT32 get_number_of_streams() const;
        private:
            std::vector<stream*> streams;
    };

    void CREST_LIB init(UINT32 buffer_ms);
    void CREST_LIB terminate();

    void CREST_LIB register_source(audio_source* source);
    void CREST_LIB remove_source(audio_source* source);
}

#endif