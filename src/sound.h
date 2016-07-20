#pragma once


#include <stdint.h>


class SoundChannel;


class Sound
{
private:
    int64_t m_cachedLength;

public:
    SoundChannel **m_channels;
    int m_numChannels;
    char *m_filename;

    Sound();

    bool LoadWav(char const *filename);
    bool SaveWav();

    int64_t GetLength();
};
