#include "sound.h"


// Own header
#include "sound.h"

// Project headers
#include "sound_channel.h"
#include "df_lib_plus_plus/binary_file_reader.h"

// Contrib headers
#include "df_time.h"

// Standard headers
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


Sound::Sound()
{
    m_channels = NULL;
    m_numChannels = 0;
    m_cachedLength = -1;
    m_playbackIdx = -1;
}


bool Sound::LoadWav(char const *filename)
{
    BinaryFileReader f(filename);
    if (!f.IsOpen())
        return false;

    // 
    // Read header

    unsigned char buf1[4];
    if (f.ReadBytes(4, buf1) != 4 || memcmp(buf1, "RIFF", 4) != 0)
        return false;

    unsigned chunkSize = f.ReadS32();

    if (f.ReadBytes(4, buf1) != 4 || memcmp(buf1, "WAVE", 4) != 0)
        return false;

    //
    // Read fmt chunk

    if (f.ReadBytes(4, buf1) != 4 || memcmp(buf1, "fmt ", 4) != 0)
        return false;
    unsigned fmtChunkSize = f.ReadU32();
    unsigned audioFormat = f.ReadU16();
    m_numChannels = f.ReadU16();
    unsigned sampleRate = f.ReadU32();
    unsigned byteRate = f.ReadU32();
    unsigned bytesPerGroup = f.ReadU16();
    unsigned bitsPerSample = f.ReadU16();

    ReleaseAssert(audioFormat == 1, "File '%s' unsupported format", filename);
    ReleaseAssert(m_numChannels == 2, "File '%s' is not stereo", filename);
    ReleaseAssert(bytesPerGroup == 4, "File '%s' unsupported block alignment", filename);
    ReleaseAssert(bitsPerSample == 16, "File '%s' is not 16 bits sample depth", filename);


    //
    // Read data chunk

    if (f.ReadBytes(4, buf1) != 4 || memcmp(buf1, "data", 4) != 0)
        return false;
    unsigned dataChunkSize = f.ReadU32();
    ReleaseAssert(dataChunkSize % bytesPerGroup == 0, "File '%s' ends with half a sample", filename);
    unsigned numGroups = dataChunkSize / bytesPerGroup;
    unsigned numBlocks = numGroups / SampleBlock::MAX_SAMPLES;
    if (numGroups % SampleBlock::MAX_SAMPLES != 0)
        numBlocks++;

    m_channels = new SoundChannel* [m_numChannels];
    for (int i = 0; i < m_numChannels; i++)
        m_channels[i] = new SoundChannel;

    int16_t *buf = new int16_t [SampleBlock::MAX_SAMPLES * m_numChannels];

    for (int blockCount = 0; blockCount < numBlocks; blockCount++)
    {
        unsigned bytesToRead = bytesPerGroup * SampleBlock::MAX_SAMPLES;
        if (blockCount + 1 == numBlocks)
            bytesToRead = (numGroups % SampleBlock::MAX_SAMPLES) * bytesPerGroup;
        size_t bytesRead = f.ReadBytes(bytesToRead, (unsigned char *)buf);
        size_t groupsRead = bytesRead / bytesPerGroup;

        for (int chan_idx = 0; chan_idx < m_numChannels; chan_idx++)
        {
            SoundChannel *chan = m_channels[chan_idx];
            SampleBlock *block = new SampleBlock;

            for (size_t i = 0; i < groupsRead; i++)
                block->m_samples[i] = buf[i * m_numChannels + chan_idx];

            block->m_len = groupsRead;
            block->RecalcLuts();

            DebugAssert(block->m_len > 0);
            chan->m_blocks.Push(block);

            if (block->m_len != SampleBlock::MAX_SAMPLES)
                DebugAssert(blockCount + 1 == numBlocks);
        }
    }

    delete[] buf;

    return true;
}


int64_t Sound::GetLength()
{
    if (m_numChannels == 0)
        return 0;

    if (m_cachedLength < 0)
        m_cachedLength = m_channels[0]->GetLength();

    return m_cachedLength;
}