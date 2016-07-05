#include "sound_system.h"

// Project includes
#include "sound.h"
#include "sample_block.h"
#include "sound_channel.h"
#include "sound/sound_device.h"

// Contrib includes
#include "df_hi_res_time.h"

// Standard headers
#include <memory.h>


SoundSystem *g_soundSystem = NULL;



// ***************************************************************************
// SoundSystem
// ***************************************************************************

static void SoundCallback(StereoSample *buf, unsigned int numSamples)
{
	g_soundSystem->DeviceCallback(buf, numSamples);
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


void SoundSystem::DeviceCallback(StereoSample *buf, unsigned int numSamples)
{
    if (!m_sound)
    {
        memset(buf, 0, numSamples * sizeof(StereoSample));
        return;
    }

    ReleaseAssert(m_sound->m_numChannels == 2, "Write more code");

    SoundChannel::SoundPos poses[2];
    poses[0] = m_sound->m_channels[0]->GetSoundPosFromSampleIdx(m_sound->m_playbackIdx);
    poses[1] = m_sound->m_channels[1]->GetSoundPosFromSampleIdx(m_sound->m_playbackIdx);

    int64_t idx = poses[0].m_sampleIdx;

    for (int i = 0; i < numSamples; i++)
    {
        SampleBlock *blocks[2];
        blocks[0] = m_sound->m_channels[0]->m_blocks[poses[0].m_blockIdx];
        blocks[1] = m_sound->m_channels[0]->m_blocks[poses[1].m_blockIdx];

        buf[i].m_left = blocks[0]->m_samples[idx];
        buf[i].m_right = blocks[1]->m_samples[idx];

        idx++;
        if (idx > blocks[0]->m_len)
        {
            idx = 0;
            poses[0].m_blockIdx++;
            poses[1].m_blockIdx++;

            if (poses[0].m_blockIdx >= m_sound->m_channels[0]->m_blocks.Size())
            {
                memset(buf + i + 1, 0, (numSamples - (i + 1)) * sizeof(StereoSample));
                m_sound->m_playbackIdx = -1;
                return;
            }

            blocks[0] = m_sound->m_channels[0]->m_blocks[poses[0].m_blockIdx];
            blocks[1] = m_sound->m_channels[0]->m_blocks[poses[1].m_blockIdx];
        }
    }

    m_sound->m_playbackIdx += numSamples;
}


void SoundSystem::PlaySound(Sound *sound, int64_t startSampleIdx)
{
    m_sound = sound;
    m_sound->m_playbackIdx = startSampleIdx;
}
