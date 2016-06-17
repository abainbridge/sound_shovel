// Own header
#include "sound.h"

// Contrib headers
#include "df_hi_res_time.h"

// Standard headers
#include <stdio.h>


bool Sound::LoadWav(char const *filename)
{
    double start_time = GetHighResTime();

    FILE *fin = fopen(filename, "rb");
    if (!fin)
        return false;

    while (1)
    {
        SampleBlock *block = new SampleBlock;

        block->m_len = fread(block->m_samples, sizeof(int16_t), SampleBlock::MAX_SAMPLES, fin);

        block->RecalcLuts();

        if (block->m_len > 0)
        {
            m_blocks.PutDataAtEnd(block);
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


unsigned Sound::GetLength()
{
    unsigned len = 0;
    for (int i = 0; i < m_blocks.Size(); i++)
        len += m_blocks[i]->m_len;
    return len;
}


Sound::SoundPos Sound::GetSoundPosFromSampleIdx(int sample_idx)
{
    int block_idx = 0;
    
    while (sample_idx > m_blocks[block_idx]->m_len)
    {
        sample_idx -= m_blocks[block_idx]->m_len;
        block_idx++;
        if (block_idx > m_blocks.Size())
            return SoundPos(-1, -1);
    }

    return SoundPos(block_idx, sample_idx);
}


void Sound::CalcMinMaxForRange(SoundPos *pos, unsigned num_samples, int16_t *result_min, int16_t *result_max)
{
    SampleBlock *block = m_blocks[pos->m_block_idx];
    unsigned idx = pos->m_sample_idx;

    int16_t _min = INT16_MAX;
    int16_t _max = INT16_MIN;

    const unsigned lut_item_boundary_mask = SampleBlock::SAMPLES_PER_LUT_ITEM - 1;

    // Do the slow bit at the start of the range.
    {
        unsigned num_samples_to_next_lut_item_boundary = (SampleBlock::SAMPLES_PER_LUT_ITEM - idx) & lut_item_boundary_mask;
        unsigned num_slow_samples = num_samples_to_next_lut_item_boundary;
        if (num_slow_samples > num_samples)
            num_slow_samples = num_samples;

        unsigned end_idx = idx + num_slow_samples;
        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }

        num_samples -= num_slow_samples;
    }

    // Do the fast bit
    while (pos->m_block_idx < m_blocks.Size() && num_samples > SampleBlock::SAMPLES_PER_LUT_ITEM)
    {
        if (idx == block->m_len)
        {
            pos->m_block_idx++;
            if (pos->m_block_idx >= m_blocks.Size())
                break;

            block = m_blocks[pos->m_block_idx];
            idx = 0;
        }

        // Cases here:
        // 1. There is one or more LUT items in this block that we can use.
        // 2. There is one or more LUT items left, but the pixel doesn't span entirely over it/them.

        unsigned num_lut_items_we_need = num_samples / SampleBlock::SAMPLES_PER_LUT_ITEM;
        unsigned current_lut_item_idx = idx / SampleBlock::SAMPLES_PER_LUT_ITEM;
        unsigned num_lut_items_in_this_block = ((block->m_len - 1) / SampleBlock::SAMPLES_PER_LUT_ITEM) + 1;
        unsigned num_lut_items_left_in_this_block = num_lut_items_in_this_block - current_lut_item_idx;

        unsigned num_lut_items_to_use = num_lut_items_we_need;
        if (num_lut_items_left_in_this_block < num_lut_items_to_use)
            num_lut_items_to_use = num_lut_items_left_in_this_block;

        unsigned end_lut_item_idx = current_lut_item_idx + num_lut_items_to_use;
        while (current_lut_item_idx < end_lut_item_idx)
        {
            _min = SAMPLE_MIN(block->m_min_lut[current_lut_item_idx], _min);
            _max = SAMPLE_MAX(block->m_max_lut[current_lut_item_idx], _max);
            current_lut_item_idx++;
        }

        // Calculate how many samples we processed. It's a little complex because the
        // LUT items we used might not have been "full", if the block has been part of
        // and insert or delete operation previously.
        unsigned num_samples_left_in_this_block_before_we_did_the_fast_bit = block->m_len - idx;
        if (num_samples > num_samples_left_in_this_block_before_we_did_the_fast_bit)
            num_samples -= num_samples_left_in_this_block_before_we_did_the_fast_bit;
        else
            num_samples -= num_lut_items_to_use * SampleBlock::SAMPLES_PER_LUT_ITEM;

        idx += num_lut_items_to_use * SampleBlock::SAMPLES_PER_LUT_ITEM;
    }

    if (idx == block->m_len)
    {
        pos->m_block_idx++;
        if (pos->m_block_idx < m_blocks.Size())
            block = m_blocks[pos->m_block_idx];
        else
            block = NULL;
        idx = 0;
    }

    // Do the slow bit at the end of the range.
    if (block)
    {
        // Cases here:
        // 1. Not at end of block and num_samples less than next LUT item boundary.

        unsigned end_idx = idx + num_samples;
        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }
    }

    pos->m_sample_idx = idx;

    *result_min = _min;
    *result_max = _max;
}


void Sound::CalcDisplayData(int start_sample_idx, int16_t *mins, int16_t *maxes, unsigned width_in_pixels, unsigned samples_per_pixel)
{
    SoundPos pos = GetSoundPosFromSampleIdx(start_sample_idx);

    for (unsigned x = 0; x < width_in_pixels; x++)
    {
        CalcMinMaxForRange(&pos, samples_per_pixel, mins + x, maxes + x);
        if (x > 0)
        {
            if (mins[x] > maxes[x - 1])
                mins[x] = maxes[x - 1] + 1;
            if (maxes[x] < mins[x - 1])
                maxes[x] = mins[x - 1] - 1;
        }
    }
}
