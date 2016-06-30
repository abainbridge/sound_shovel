#include "sound_system.h"

// Project includes
#include "sound.h"
#include "sample_block.h"
#include "sound_channel.h"
#include "sound_device.h"

// Contrib includes
#include "df_hi_res_time.h"

// Standard headers
#include <memory.h>


SoundSystem *g_soundSystem = NULL;



// ***************************************************************************
// SoundSystem
// ***************************************************************************

static void SoundCallback(StereoSample *buf, unsigned int num_samples)
{
	g_soundSystem->DeviceCallback(buf, num_samples);
}


SoundSystem::SoundSystem()
{
    m_sound = NULL;

	g_soundDevice = new SoundDevice;
	g_soundDevice->SetCallback(SoundCallback);
}


void SoundSystem::Advance()
{
	g_soundDevice->TopupBuffer();
}


#include <math.h>
void SoundSystem::DeviceCallback(StereoSample *buf, unsigned int num_samples)
{
//     static double phase = 0.0;
//     double const phase_inc = (2.0 * 3.14159265 * 1000.0) / 44100.0;
//     for (int i = 0; i < num_samples; i++)
//     {
//         buf[i].m_left = buf[i].m_right = sin(phase) * 10000.0;
//         phase += phase_inc;
//     }
//     return;

    if (!m_sound || !m_current_blocks[0] || !m_current_blocks[1])
    {
        memset(buf, 0, num_samples * sizeof(StereoSample));
        return;
    }

    ReleaseAssert(m_sound->m_num_channels == 2, "Write more code");

    for (int i = 0; i < num_samples; i++)
    {
        buf[i].m_left = m_current_blocks[0]->m_data->m_samples[m_sample_idx];
        buf[i].m_right = m_current_blocks[1]->m_data->m_samples[m_sample_idx];

        m_sample_idx++;
        if (m_sample_idx > m_current_blocks[0]->m_data->m_len)
        {
            m_sample_idx = 0;
            m_current_blocks[0] = m_current_blocks[0]->m_next;
            m_current_blocks[1] = m_current_blocks[1]->m_next;

            if (!m_current_blocks[0] || !m_current_blocks[1])
            {
                memset(buf + i + 1, 0, (num_samples - (i + 1)) * sizeof(StereoSample));
                return;
            }
        }
    }
}


void SoundSystem::PlaySound(Sound *sound, int64_t start_sample_idx)
{
    m_sound = sound;
    m_sample_idx = start_sample_idx;

    m_current_blocks[0] = m_sound->m_channels[0]->m_blocks.GetItem(0);
    m_current_blocks[1] = m_sound->m_channels[1]->m_blocks.GetItem(0);
}
