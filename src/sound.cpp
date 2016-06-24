#include "sound.h"


// Own header
#include "sound.h"

// Project headers
#include "sound_channel.h"

// Contrib headers
#include "df_hi_res_time.h"

// Standard headers
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


Sound::Sound()
{
    m_num_channels = 0;
}


bool Sound::LoadWav(char const *filename)
{
    double start_time = GetHighResTime();

    FILE *fin = fopen(filename, "rb");
    if (!fin)
        return false;

    m_num_channels = 2;
    m_channels = new SoundChannel* [m_num_channels];
    for (int i = 0; i < m_num_channels; i++)
        m_channels[i] = new SoundChannel;

    int16_t *buf = new int16_t [SampleBlock::MAX_SAMPLES * m_num_channels];
    size_t bytes_per_group = sizeof(int16_t) * m_num_channels;

    bool finished = false;
    while (!finished)
    {
        size_t groups_read = fread(buf, bytes_per_group, SampleBlock::MAX_SAMPLES, fin);

        for (int chan_idx = 0; chan_idx < m_num_channels; chan_idx++)
        {
            SoundChannel *chan = m_channels[chan_idx];
            SampleBlock *block = new SampleBlock;

            for (size_t i = 0; i < groups_read; i++)
                block->m_samples[i] = buf[i * m_num_channels + chan_idx];

            block->m_len = groups_read;
            block->RecalcLuts();

            if (block->m_len > 0)
                chan->m_blocks.PutDataAtEnd(block);
            else
                delete block;

            if (block->m_len != SampleBlock::MAX_SAMPLES)
                finished = true;
        }
    }

    fclose(fin);

    double end_time = GetHighResTime();
    DebugOut("%6.3f\n", end_time - start_time);

    return true;
}


int64_t Sound::GetLength()
{
    if (m_num_channels == 0)
        return 0;

    return m_channels[0]->GetLength();
}