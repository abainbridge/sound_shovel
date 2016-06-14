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

    SampleBlock()
    {
        m_len = 0; // MAX_SAMPLES;

                   //         for (unsigned i = 0; i < m_len; i++)
                   //             m_samples[i] = i - 32768;
                   // 
                   //         RecalcLuts();
    }

    void RecalcLuts()
    {
        int16_t *current_sample = m_samples;
        for (unsigned i = 0; i < LUT_SIZE; i++)
        {
            int16_t _min = INT16_MAX;
            int16_t _max = INT16_MIN;
            for (unsigned j = 0; j < SAMPLES_PER_LUT_ITEM; j++)
            {
                //                _min = std::min(*current_sample, _min);
                //                _max = std::max(*current_sample, _max);
                _min = SAMPLE_MIN(*current_sample, _min);
                _max = SAMPLE_MAX(*current_sample, _max);
                current_sample++;
            }

            m_max_lut[i] = _max;
            m_min_lut[i] = _min;
        }
    }
};
