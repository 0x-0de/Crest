#include "effects.h"

#include <iostream>

void process_value_change(float& value, float& value_effect, float step)
{
    if(value_effect < value)
    {
        value_effect += step;
        if(value_effect > value) value_effect = value;
    }
    else if(value_effect > value)
    {
        value_effect -= step;
        if(value_effect < value) value_effect = value;
    }
}

crest::effect_volume::effect_volume(float volume) : volume(volume)
{
    volume_effect = volume;
    change_buffer = 0.25f;
}

crest::effect_volume::~effect_volume() {}

void crest::effect_volume::set_volume(float value)
{
    volume = value;
}

void crest::effect_volume::transform(float* data, UINT32 length, audio_format format)
{
    float step = ((1.f / format.sample_rate) / format.channels) / change_buffer;

    for(UINT32 i = 0; i < length; i++)
    {
        process_value_change(volume, volume_effect, step);

        data[i] *= volume_effect;
    }
}

crest::effect_balance::effect_balance(float volume_left, float volume_right) : volume_left(volume_left), volume_right(volume_right)
{
    volume_left_effect = volume_left;
    volume_right_effect = volume_right;

    change_buffer = 0.25f;
}

crest::effect_balance::~effect_balance() {}

void crest::effect_balance::set_volume_left(float value)
{
    volume_left = value;
}

void crest::effect_balance::set_volume_right(float value)
{
    volume_right = value;
}

void crest::effect_balance::transform(float* data, UINT32 length, audio_format format)
{
    float step = ((1.f / format.sample_rate) / format.channels) / change_buffer;

    for(UINT32 i = 0; i < length; i += 2)
    {
        process_value_change(volume_left, volume_left_effect, step);
        process_value_change(volume_right, volume_right_effect, step);
        
        data[i    ] *= volume_left_effect;
        data[i + 1] *= volume_right_effect;
    }
}