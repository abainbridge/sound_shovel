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

    m_num_channels = 1;
    m_channels = new SoundChannel* [m_num_channels];
    SoundChannel *chan = new SoundChannel;
    m_channels[0] = chan;

    while (1)
    {
        SampleBlock *block = new SampleBlock;

        block->m_len = fread(block->m_samples, sizeof(int16_t), SampleBlock::MAX_SAMPLES, fin);

        block->RecalcLuts();

        if (block->m_len > 0)
        {
            chan->m_blocks.PutDataAtEnd(block);
        }
        else
        {
            delete block;
            break;
        }

        if (block->m_len != SampleBlock::MAX_SAMPLES)
            break;
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