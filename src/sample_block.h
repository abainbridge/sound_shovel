#pragma once


#include <stdint.h>


#define SAMPLE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define SAMPLE_MAX(a,b) ((a) > (b) ? (a) : (b))


struct SampleBlock
{
    enum { MAX_SAMPLES = 131072 };
    enum { SAMPLES_PER_LUT_ITEM = 1024 };
    enum { LUT_SIZE = MAX_SAMPLES / SAMPLES_PER_LUT_ITEM };

    int16_t     m_samples[MAX_SAMPLES];
    unsigned    m_len;   // Number of valid items in m_samples
    int16_t     m_max_lut[LUT_SIZE];
    int16_t     m_min_lut[LUT_SIZE];

    SampleBlock();

    void RecalcLuts();
};
