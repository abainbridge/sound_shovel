#pragma once


#include <stdint.h>


class Sound;
class StereoSample;


class SoundSystem
{
public:
    Sound *m_sound;

	SoundSystem();

	void Advance();
	void DeviceCallback(StereoSample *buf, unsigned int num_samples);

    void PlaySound(Sound *sound, int64_t start_sample_idx);
};


extern SoundSystem *g_soundSystem;
