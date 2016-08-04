// Own header
#include "sound_channel.h"

// Contrib headers
#include "df_common.h"

// Standard headers
#include <math.h>
#include <memory.h>
#include <stdlib.h>


unsigned SoundChannel::GetLength()
{
    unsigned len = 0;
    for (int i = 0; i < m_blocks.Size(); i++)
        len += m_blocks[i]->m_len;
    return len;
}


void SoundChannel::Delete(int64_t startIdx, int64_t endIdx)
{
    int64_t numSamplesToDelete = endIdx - startIdx + 1;
    SoundPos pos = GetSoundPosFromSampleIdx(startIdx);
    while (pos.m_blockIdx < m_blocks.Size())
    {
        SampleBlock *block = m_blocks[pos.m_blockIdx];
        int64_t numSamplesToDeleteFromThisBlock = numSamplesToDelete;
        int numSamplesLeftInThisBlock = block->m_len - pos.m_sampleIdx;
        if (numSamplesToDeleteFromThisBlock > numSamplesLeftInThisBlock)
        {
            // Delete up to the end of the block
            block->m_len = pos.m_sampleIdx;
            pos.m_sampleIdx = 0;

            numSamplesToDelete -= numSamplesLeftInThisBlock;

            block->RecalcLuts();
        }
        else
        {
            // Delete a bit from the middle (or maybe the start) of the block
            int numSamplesToCopy = block->m_len - (pos.m_sampleIdx + numSamplesToDeleteFromThisBlock);
            int16_t *whereToCopyFrom = block->m_samples + pos.m_sampleIdx + numSamplesToDeleteFromThisBlock;
            memcpy(block->m_samples + pos.m_sampleIdx,
                whereToCopyFrom,
                numSamplesToCopy);
            block->m_len -= numSamplesToDeleteFromThisBlock;

            block->RecalcLuts();

            break;
        }

        pos.m_blockIdx++;
    }

    // Now remove any empty blocks
    {
        int i = 0;
        int j = 0;

        // i keeps track of the position in the input array (m_blocks)
        // j keeps track of the position in the output array (also m_blocks)
        // Each time when find a block to be deleted, i increments, while j stays the same.

        for (i = 0; i < m_blocks.Size(); i++)
        {
            if (m_blocks[i]->m_len > 0)
            {
                m_blocks[j] = m_blocks[i];
                j++;
            }
        }

        while (i > j)
        {
            m_blocks.Pop();
            j++;
        }
    }
}


SoundChannel::SoundPos SoundChannel::GetSoundPosFromSampleIdx(int64_t sampleIdx)
{
    int blockIdx = 0;

    while (sampleIdx >= m_blocks[blockIdx]->m_len)
    {
        sampleIdx -= m_blocks[blockIdx]->m_len;
        blockIdx++;
        if (blockIdx > m_blocks.Size())
            return SoundPos(-1, -1);
    }

    return SoundPos(blockIdx, sampleIdx);
}


// Move SoundPos along by the specified numSamples, moving over (potentially part
// full) blocks as needed.
//
// Returns a pointer to the block we end on, or NULL if we go off the end of the
// Sound.
SampleBlock *SoundChannel::IncrementSoundPos(SoundPos *pos, int64_t numSamples)
{
    if (pos->m_blockIdx >= m_blocks.Size())
        return NULL;

    SampleBlock *block = m_blocks[pos->m_blockIdx];
    ReleaseAssert(pos->m_sampleIdx < block->m_len, "Invalid SoundPos - m_sampleIdx beyond end of block");

    // Iterate across blocks until we've crossed enough samples.
    while (numSamples)
    {
        // Has this block got at least 'numSamples' left?
        if (pos->m_sampleIdx + numSamples < block->m_len)
        {
            // Yes
            pos->m_sampleIdx += numSamples;
            numSamples = 0;
        }
        else
        {
            // No
            numSamples -= block->m_len - pos->m_sampleIdx;
            pos->m_blockIdx++;
            if (pos->m_blockIdx >= m_blocks.Size())
                return NULL;

            block = m_blocks[pos->m_blockIdx];
            pos->m_sampleIdx = 0;
        }
    }

    return block;
}


void SoundChannel::CalcMinMaxForRange(SoundPos *pos, unsigned numSamples, int16_t *resultMin, int16_t *resultMax)
{
    // If pos is entirely outside the bounds of the sample, then we set resultMin and resultMax to zero.
    if (pos->m_blockIdx >= m_blocks.Size())
    {
        *resultMax = 0;
        *resultMin = 0;
        return;
    }

    SampleBlock *block = m_blocks[pos->m_blockIdx];

    int16_t _min = INT16_MAX;
    int16_t _max = INT16_MIN;

    // Do the slow bit at the start of the range.
    {
        const unsigned lutItemBoundaryMask = SampleBlock::SAMPLES_PER_LUT_ITEM - 1;
        unsigned numSamplesToNextLutItemBoundary = (SampleBlock::SAMPLES_PER_LUT_ITEM - pos->m_sampleIdx) & lutItemBoundaryMask;
        unsigned numSlowSamples = numSamplesToNextLutItemBoundary;
        if (numSlowSamples > numSamples)
            numSlowSamples = numSamples;
        if (numSlowSamples > block->m_len)
            numSlowSamples = block->m_len;

        unsigned idx = pos->m_sampleIdx;
        unsigned end_idx = idx + numSlowSamples;
        if (end_idx > block->m_len)
            end_idx = block->m_len;

        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }

        numSamples -= numSlowSamples;
        block = IncrementSoundPos(pos, numSlowSamples);
    }

    // Do the fast bit - one block per iteration.
    while (block && numSamples > SampleBlock::SAMPLES_PER_LUT_ITEM)
    {
        // Cases here:
        // 1. There is one or more LUT items in this block that we can use.
        // 2. There is one or more LUT items left, but the pixel doesn't span entirely over it/them.

        unsigned numLutItemsWeNeed = numSamples / SampleBlock::SAMPLES_PER_LUT_ITEM;
        unsigned currentLutItemIdx = pos->m_sampleIdx / SampleBlock::SAMPLES_PER_LUT_ITEM;
        unsigned numLutItemsInThisBlock = ((block->m_len - 1) / SampleBlock::SAMPLES_PER_LUT_ITEM) + 1;
        unsigned numLutItemsLeftInThisBlock = numLutItemsInThisBlock - currentLutItemIdx;

        unsigned numLutItemsToUse = numLutItemsWeNeed;
        if (numLutItemsLeftInThisBlock < numLutItemsToUse)
            numLutItemsToUse = numLutItemsLeftInThisBlock;

        unsigned endLutItemIdx = currentLutItemIdx + numLutItemsToUse;
        while (currentLutItemIdx < endLutItemIdx)
        {
            _min = SAMPLE_MIN(block->m_minLut[currentLutItemIdx], _min);
            _max = SAMPLE_MAX(block->m_maxLut[currentLutItemIdx], _max);
            currentLutItemIdx++;
        }

        // Calculate how many samples we processed. It's a little complex because the
        // LUT items we used might not have been "full", if the block has been part of
        // and insert or delete operation previously.
        unsigned numSamplesLeftInThisBlockBeforeWeDidTheFastBit = block->m_len - pos->m_sampleIdx;
        unsigned numSamplesProcessedThisIteration = numLutItemsToUse * SampleBlock::SAMPLES_PER_LUT_ITEM;
        if (numSamples > numSamplesLeftInThisBlockBeforeWeDidTheFastBit)
            numSamplesProcessedThisIteration = numSamplesLeftInThisBlockBeforeWeDidTheFastBit;

        numSamples -= numSamplesProcessedThisIteration;
        block = IncrementSoundPos(pos, numSamplesProcessedThisIteration);
    }

    // Do the slow bit at the end of the range.
    while (block && numSamples)
    {
        // We are not at end of block and numSamples is less than next LUT item boundary.
        // Cases:
        // 1 - There are enough samples in the current block to cover the current pixel.
        // 2 - There aren't.

        unsigned idx = pos->m_sampleIdx;
        unsigned end_idx = pos->m_sampleIdx + numSamples;
        if (end_idx > block->m_len)
            end_idx = block->m_len;

        unsigned numSamplesThisIteration = end_idx - idx;
        
        while (idx < end_idx)
        {
            _min = SAMPLE_MIN(block->m_samples[idx], _min);
            _max = SAMPLE_MAX(block->m_samples[idx], _max);
            idx++;
        }

        block = IncrementSoundPos(pos, numSamplesThisIteration);
        numSamples -= numSamplesThisIteration;
    }

    *resultMin = _min;
    *resultMax = _max;
}


void SoundChannel::CalcDisplayData(int start_sample_idx, int16_t *mins, int16_t *maxes, unsigned widthInPixels, double samplesPerPixel)
{
    SoundPos pos = GetSoundPosFromSampleIdx(start_sample_idx);

    double widthErrorPerPixel = samplesPerPixel - floorf(samplesPerPixel);
    double error = 0;
    for (unsigned x = 0; x < widthInPixels; x++)
    {
        if (pos.m_blockIdx < m_blocks.Size())
        {
            int samplesThisPixel = samplesPerPixel;
            if (error > 1.0)
            {
                samplesThisPixel++;
                error -= 1.0;
            }
            error += widthErrorPerPixel;

            CalcMinMaxForRange(&pos, samplesThisPixel, mins + x, maxes + x);

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
