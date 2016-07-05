#pragma once


#include <stdint.h>


class SoundChannel;


class Sound
{
private:
    int64_t m_cachedLength;

public:
    SoundChannel **m_channels;
    int64_t m_playbackIdx;
    int m_numChannels;

    Sound();

    bool LoadWav(char const *filename);

    int64_t GetLength();
};
