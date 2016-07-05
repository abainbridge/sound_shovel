// Own header
#include "sample_block.h"


SampleBlock::SampleBlock()
{
    m_len = 0;
}


void SampleBlock::RecalcLuts()
{
    int16_t *currentSample = m_samples;
    for (unsigned i = 0; i < LUT_SIZE; i++)
    {
        int16_t _min = INT16_MAX;
        int16_t _max = INT16_MIN;
        for (unsigned j = 0; j < SAMPLES_PER_LUT_ITEM; j++)
        {
            //                _min = std::min(*currentSample, _min);
            //                _max = std::max(*currentSample, _max);
            _min = SAMPLE_MIN(*currentSample, _min);
            _max = SAMPLE_MAX(*currentSample, _max);
            currentSample++;
        }

        m_maxLut[i] = _max;
        m_minLut[i] = _min;
    }
}
