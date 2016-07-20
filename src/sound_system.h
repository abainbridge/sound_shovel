#pragma once


#include <stdint.h>


class SoundWidget;
class StereoSample;


class SoundSystem
{
public:
    SoundWidget *m_soundWidget;

	SoundSystem();

	void Advance();
	void DeviceCallback(StereoSample *buf, unsigned int numSamples);

    void PlaySound(SoundWidget *SoundWidget);
};


extern SoundSystem *g_soundSystem;
