#pragma once


#include <stdint.h>


class BinaryStreamReader;
class BinaryStreamWriter;
class SoundChannel;


class Sound
{
private:
    int64_t m_cachedLength;
    void SetVolumeHelper(int64_t startIdx, int64_t endIdx, double startVol, double endVol);

public:
    enum
    {
        ERROR_NO_ERROR,
        ERROR_WRONG_NUMBER_OF_CHANNELS
    };

    SoundChannel **m_channels;  // All the channels contain the same number of samples.
    int m_numChannels;
    char *m_filename;

    Sound();
    ~Sound();

    void Delete(int64_t startIdx, int64_t endIdx);
    int Insert(int64_t startIdx, Sound *sound);

    void FadeIn(int64_t startIdx, int64_t endIdx);
    void FadeOut(int64_t startIdx, int64_t endIdx);
    void Normalize(int64_t startIdx, int64_t endIdx);

    bool LoadWav(BinaryStreamReader *stream);
    bool SaveWav(); // Wrapper of BinaryStreamWriter overload. Saves to file called m_filename.
    bool SaveWav(BinaryStreamWriter *stream, int64_t startIdx, int64_t endIdx);

    int64_t GetLength();
};
