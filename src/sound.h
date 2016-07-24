#pragma once


#include <stdint.h>


class SoundChannel;


class Sound
{
private:
    int64_t m_cachedLength;
    void SetVolumeHelper(int64_t startIdx, int64_t endIdx, double startVol, double endVol);

public:
    SoundChannel **m_channels;
    int m_numChannels;
    char *m_filename;

    Sound();

    void FadeIn(int64_t startIdx, int64_t endIdx);
    void FadeOut(int64_t startIdx, int64_t endIdx);
    void Normalize(int64_t startIdx, int64_t endIdx);

    bool LoadWav(char const *filename);
    bool SaveWav();

    int64_t GetLength();
};
