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
	void DeviceCallback(StereoSample *buf, unsigned int numSamples);

    void PlaySound(Sound *sound, int64_t startSampleIdx);
};


extern SoundSystem *g_soundSystem;
