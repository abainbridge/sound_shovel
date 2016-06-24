#pragma once


#include <stdint.h>


class SoundChannel;


class Sound
{
private:
    int64_t m_cached_length;

public:
    SoundChannel **m_channels;
    int m_num_channels;

    Sound();

    bool LoadWav(char const *filename);

    int64_t GetLength();
};
