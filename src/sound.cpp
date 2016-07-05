#include "sound.h"


// Own header
#include "sound.h"

// Project headers
#include "sound_channel.h"
#include "df_lib_plus_plus/binary_file_reader.h"

// Contrib headers
#include "df_hi_res_time.h"

// Standard headers
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


Sound::Sound()
{
    m_channels = NULL;
    m_num_channels = 0;
    m_cached_length = -1;
    m_playback_idx = -1;
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
    m_num_channels = f.ReadU16();
    unsigned sampleRate = f.ReadU32();
    unsigned byteRate = f.ReadU32();
    unsigned bytes_per_group = f.ReadU16();
    unsigned bitsPerSample = f.ReadU16();

    ReleaseAssert(audioFormat == 1, "File '%s' unsupported format", filename);
    ReleaseAssert(m_num_channels == 2, "File '%s' is not stereo", filename);
    ReleaseAssert(bytes_per_group == 4, "File '%s' unsupported block alignment", filename);
    ReleaseAssert(bitsPerSample == 16, "File '%s' is not 16 bits sample depth", filename);


    //
    // Read data chunk

    if (f.ReadBytes(4, buf1) != 4 || memcmp(buf1, "data", 4) != 0)
        return false;
    unsigned dataChunkSize = f.ReadU32();
    ReleaseAssert(dataChunkSize % bytes_per_group == 0, "File '%s' ends with half a sample", filename);
    unsigned num_groups = dataChunkSize / bytes_per_group;
    unsigned num_blocks = num_groups / SampleBlock::MAX_SAMPLES;
    if (num_groups % SampleBlock::MAX_SAMPLES != 0)
        num_blocks++;

    m_channels = new SoundChannel* [m_num_channels];
    for (int i = 0; i < m_num_channels; i++)
        m_channels[i] = new SoundChannel;

    int16_t *buf = new int16_t [SampleBlock::MAX_SAMPLES * m_num_channels];

    for (int block_count = 0; block_count < num_blocks; block_count++)
    {
        unsigned bytes_to_read = bytes_per_group * SampleBlock::MAX_SAMPLES;
        if (block_count + 1 == num_blocks)
            bytes_to_read = (num_groups % SampleBlock::MAX_SAMPLES) * bytes_per_group;
        size_t bytes_read = f.ReadBytes(bytes_to_read, (unsigned char *)buf);
        size_t groups_read = bytes_read / bytes_per_group;

        for (int chan_idx = 0; chan_idx < m_num_channels; chan_idx++)
        {
            SoundChannel *chan = m_channels[chan_idx];
            SampleBlock *block = new SampleBlock;

            for (size_t i = 0; i < groups_read; i++)
                block->m_samples[i] = buf[i * m_num_channels + chan_idx];

            block->m_len = groups_read;
            block->RecalcLuts();

            DebugAssert(block->m_len > 0);
            chan->m_blocks.Push(block);

            if (block->m_len != SampleBlock::MAX_SAMPLES)
                DebugAssert(block_count + 1 == num_blocks);
        }
    }

    delete[] buf;

    return true;
}


int64_t Sound::GetLength()
{
    if (m_num_channels == 0)
        return 0;

    if (m_cached_length < 0)
        m_cached_length = m_channels[0]->GetLength();

    return m_cached_length;
}