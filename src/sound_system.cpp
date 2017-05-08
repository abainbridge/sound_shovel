#include "sound_system.h"

// Project includes
#include "sound.h"
#include "sample_block.h"
#include "sound_channel.h"
#include "gui/sound_widget.h"
#include "sound/sound_device.h"

// Contrib includes
#include "df_time.h"

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
    m_soundWidget = NULL;

	g_soundDevice = new SoundDevice;
	g_soundDevice->SetCallback(SoundCallback);
}


void SoundSystem::Advance()
{
	g_soundDevice->TopupBuffer();
}


void SoundSystem::DeviceCallback(StereoSample *buf, unsigned int numSamples)
{
    if (!m_soundWidget || !m_soundWidget->m_sound || !m_soundWidget->m_isPlaying)
    {
        memset(buf, 0, numSamples * sizeof(StereoSample));
        return;
    }

    Sound *sound = m_soundWidget->m_sound;
    ReleaseAssert(sound->m_numChannels == 2, "Write more code");

    SoundChannel::SoundPos poses[2];
    poses[0] = sound->m_channels[0]->GetSoundPosFromSampleIdx(m_soundWidget->m_playbackIdx);
    poses[1] = sound->m_channels[1]->GetSoundPosFromSampleIdx(m_soundWidget->m_playbackIdx);

    int64_t idx = poses[0].m_sampleIdx;

    for (int i = 0; i < numSamples; i++)
    {
        SampleBlock *blocks[2];
        blocks[0] = sound->m_channels[0]->m_blocks[poses[0].m_blockIdx];
        blocks[1] = sound->m_channels[1]->m_blocks[poses[1].m_blockIdx];

        buf[i].m_left = blocks[0]->m_samples[idx];
        buf[i].m_right = blocks[1]->m_samples[idx];

        idx++;
        if (idx >= blocks[0]->m_len)
        {
            idx = 0;
            poses[0].m_blockIdx++;
            poses[1].m_blockIdx++;

            if (poses[0].m_blockIdx >= sound->m_channels[0]->m_blocks.Size())
            {
                memset(buf + i + 1, 0, (numSamples - (i + 1)) * sizeof(StereoSample));
                m_soundWidget->m_playbackIdx = -1;
                m_soundWidget->Pause();
                return;
            }

            blocks[0] = sound->m_channels[0]->m_blocks[poses[0].m_blockIdx];
            blocks[1] = sound->m_channels[0]->m_blocks[poses[1].m_blockIdx];
        }
    }

    m_soundWidget->m_playbackIdx += numSamples;
}


void SoundSystem::PlaySound(SoundWidget *soundWidget)
{
    m_soundWidget = soundWidget;
}
