#pragma once

// Project headers
#include "sample_block.h"

// Contrib headers
#include "df_lib_plus_plus/llist.h"

// Standard headers
#include <stdint.h>


class Sound
{
private:
    struct SoundPos
    {
        unsigned m_block_idx;
        unsigned m_sample_idx;

        SoundPos(unsigned block_idx, unsigned sample_idx)
        {
            m_block_idx = block_idx;
            m_sample_idx = sample_idx;
        }
    };

    void CalcMinMaxForRange(SoundPos *pos, unsigned num_samples, int16_t *result_min, int16_t *result_max);

public:
    LList <SampleBlock *> m_blocks;

    bool LoadWav(char const *filename);

    unsigned GetLength();

    void CalcDisplayData(unsigned start_sample_idx, int16_t *mins, int16_t *maxes, unsigned width_in_pixels, unsigned samples_per_pixel);
};
