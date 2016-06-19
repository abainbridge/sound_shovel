// Own header
#include "sample_block.h"


SampleBlock::SampleBlock()
{
    m_len = 0;
}


void SampleBlock::RecalcLuts()
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
