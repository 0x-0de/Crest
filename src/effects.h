#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "crest.h"

namespace crest
{
    class CREST_LIB effect_volume : public stream_transform
    {
        public:
            effect_volume(float volume);
            ~effect_volume();

            void set_volume(float value);

            void transform(float* data, UINT32 length, audio_format format);
        private:
            float volume, volume_effect;
    };

    class CREST_LIB effect_balance : public stream_transform
    {
        public:
            effect_balance(float volume_left, float volume_right);
            ~effect_balance();

            void set_volume_left(float value);
            void set_volume_right(float value);

            void transform(float* data, UINT32 length, audio_format format);
        private:
            float volume_left, volume_right, volume_left_effect, volume_right_effect;
    };
}

#endif