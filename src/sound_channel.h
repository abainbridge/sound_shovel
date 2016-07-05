#pragma once

// Project headers
#include "sample_block.h"

// Contrib headers
#include "df_lib_plus_plus/darray.h"

// Standard headers
#include <stdint.h>


class SoundChannel
{
public:
    struct SoundPos
    {
        int m_blockIdx;
        int m_sampleIdx;

        SoundPos()
        {
	        m_blockIdx = 0;
    	    m_sampleIdx = 0;
        }

        SoundPos(int blockIdx, int sampleIdx)
        {
            m_blockIdx = blockIdx;
            m_sampleIdx = sampleIdx;
        }
    };

private:
    void CalcMinMaxForRange(SoundPos *pos, unsigned numSamples, int16_t *resultMin, int16_t *resultMax);

public:
    SoundPos GetSoundPosFromSampleIdx(int64_t sampleIdx);
    SampleBlock *IncrementSoundPos(SoundPos *pos, int64_t numSamples);

    DArray <SampleBlock *> m_blocks;

    unsigned GetLength();

    void CalcDisplayData(int startSampleIdx, int16_t *mins, int16_t *maxes, unsigned widthInPixels, double samplesPerPixel);
};
