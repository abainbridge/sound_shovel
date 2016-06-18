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


// Move SoundPos along by the specified num_samples, moving over (potentially part
// full) blocks as needed.
//
// Returns a pointer to the block we end on, or NULL if we go off the end of the
// Sound.
SampleBlock *Sound::IncrementSoundPos(SoundPos *pos, int64_t num_samples)
{
    if (pos->m_block_idx >= m_blocks.Size())
        return NULL;

    SampleBlock *block = m_blocks[pos->m_block_idx];
    ReleaseAssert(pos->m_sample_idx < block->m_len, "Invalid SoundPos - m_sample_idx beyond end of block");

    // Iterate across blocks until we've crossed enough samples.
    while (num_samples)
    {
        // Has this block got at least 'num_samples' left?
        if (pos->m_sample_idx + num_samples < block->m_len)
        {
            // Yes
            pos->m_sample_idx += num_samples;
            num_samples = 0;

            ReleaseAssert(pos->m_sample_idx < block->m_len, "kjhkjh");
        }
        else
        {
            // No
            num_samples -= block->m_len - pos->m_sample_idx;
            pos->m_block_idx++;
            pos->m_sample_idx = 0;
            if (pos->m_block_idx >= m_blocks.Size())
                return NULL;
        }
    }

    return block;
}


void Sound::CalcMinMaxForRange(SoundPos *pos, unsigned num_samples, int16_t *result_min, int16_t *result_max)
{
    // If pos is entirely outside the bounds of the sample, then we set result_min and result_max to zero.
    if (pos->m_block_idx >= m_blocks.Size())
    {
        *result_max = 0;
        *result_min = 0;
        return;
    }

    SampleBlock *block = m_blocks[pos->m_block_idx];

    int16_t _min = INT16_MAX;
    int16_t _max = INT16_MIN;

    // Do the slow bit at the start of the range.
    {
        const unsigned lut_item_boundary_mask = SampleBlock::SAMPLES_PER_LUT_ITEM - 1;
        unsigned num_samples_to_next_lut_item_boundary = (SampleBlock::SAMPLES_PER_LUT_ITEM - pos->m_sample_idx) & lut_item_boundary_mask;
        unsigned num_slow_samples = num_samples_to_next_lut_item_boundary;
        if (num_slow_samples > num_samples)
            num_slow_samples = num_samples;

        unsigned idx = pos->m_sample_idx;
        unsigned end_idx = idx + num_slow_samples;
        if (end_idx > block->m_len)
            end_idx = block->m_len;
        
        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }

        num_samples -= num_slow_samples;
        block = IncrementSoundPos(pos, num_slow_samples);
    }

    // Do the fast bit - one block per iteration.
    while (block && num_samples > SampleBlock::SAMPLES_PER_LUT_ITEM)
    {
        // Cases here:
        // 1. There is one or more LUT items in this block that we can use.
        // 2. There is one or more LUT items left, but the pixel doesn't span entirely over it/them.

        unsigned num_lut_items_we_need = num_samples / SampleBlock::SAMPLES_PER_LUT_ITEM;
        unsigned current_lut_item_idx = pos->m_sample_idx / SampleBlock::SAMPLES_PER_LUT_ITEM;
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
        unsigned num_samples_left_in_this_block_before_we_did_the_fast_bit = block->m_len - pos->m_sample_idx;
        unsigned num_samples_processed_this_iteration = num_lut_items_to_use * SampleBlock::SAMPLES_PER_LUT_ITEM;
        if (num_samples > num_samples_left_in_this_block_before_we_did_the_fast_bit)
            num_samples_processed_this_iteration = num_samples_left_in_this_block_before_we_did_the_fast_bit;

        num_samples -= num_samples_processed_this_iteration;
        block = IncrementSoundPos(pos, num_samples_processed_this_iteration);
    }

    // Do the slow bit at the end of the range.
    if (block)
    {
        // We are not at end of block and num_samples is less than next LUT item boundary.
        // Cases:
        // 1 - There are enough samples in the current block to cover the current pixel.
        // 2 - There aren't.

        unsigned idx = pos->m_sample_idx;
        unsigned end_idx = pos->m_sample_idx + num_samples;
        if (end_idx > block->m_len)
            end_idx = block->m_len;

        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }

        block = IncrementSoundPos(pos, num_samples);
    }

    *result_min = _min;
    *result_max = _max;
}


void Sound::CalcDisplayData(int start_sample_idx, int16_t *mins, int16_t *maxes, unsigned width_in_pixels, unsigned samples_per_pixel)
{
    SoundPos pos = GetSoundPosFromSampleIdx(start_sample_idx);

    for (unsigned x = 0; x < width_in_pixels; x++)
    {
        if (pos.m_block_idx < m_blocks.Size())
        {
            CalcMinMaxForRange(&pos, samples_per_pixel, mins + x, maxes + x);

            // On all but the first iteration of the loop, make vline join onto
            // the previous, so no gaps are visible.
            if (x > 0)
            {
                if (mins[x] > maxes[x - 1])
                    mins[x] = maxes[x - 1] + 1;
                if (maxes[x] < mins[x - 1])
                    maxes[x] = mins[x - 1] - 1;
            }
        }
        else
        {
            mins[x] = 0;
            maxes[x] = 0;
        }
    }
}
