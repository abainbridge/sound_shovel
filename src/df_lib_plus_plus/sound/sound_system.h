#pragma once


#include "llist.h"
#include <stdint.h>


class Sound;
struct SampleBlock;
class StereoSample;


class SoundSystem
{
public:
    Sound *m_sound;
    int64_t m_sample_idx;
    LListItem<SampleBlock *> *m_current_blocks[2];

	SoundSystem();

	void Advance();
	void DeviceCallback(StereoSample *buf, unsigned int num_samples);

    void PlaySound(Sound *sound, int64_t start_sample_idx);
};


extern SoundSystem *g_soundSystem;
