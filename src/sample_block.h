#pragma once


#include <stdint.h>


#define SAMPLE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define SAMPLE_MAX(a,b) ((a) > (b) ? (a) : (b))


struct SampleBlock
{
    enum { MAX_SAMPLES = 131072 };
    enum { SAMPLES_PER_LUT_ITEM = 256 };	// For optimal results, set this to sqrt(sample_len / screen_width_in_pixels), rounded to the nearest power of 2.
    enum { LUT_SIZE = MAX_SAMPLES / SAMPLES_PER_LUT_ITEM };

    int16_t     m_samples[MAX_SAMPLES];
    unsigned    m_len;   // Number of valid items in m_samples
    int16_t     m_maxLut[LUT_SIZE];
    int16_t     m_minLut[LUT_SIZE];

    SampleBlock();

    void RecalcLuts();
};
