#pragma once


//*****************************************************************************
// Class StereoSample
//*****************************************************************************

class StereoSample
{
public:
	short m_left;
	short m_right;

	StereoSample(): m_left(0), m_right(0) {}
};


class StereoSampleBuf;


//*****************************************************************************
// Class SoundDevice
//*****************************************************************************

class SoundDevice
{
protected:
	StereoSampleBuf	*m_buffers;
	unsigned int	m_num_buffers;
	unsigned int	m_next_buffer;		// Index of next buffer to send to sound card

public:
	unsigned int	m_fills_requested;	// Number of outstanding requests for more sound data that Windows has issued
	unsigned int	m_freq;
	unsigned int	m_samplesPerBuffer;
    void			(*m_callback) (StereoSample *buf, unsigned int numSamples);

public:
	SoundDevice();

	int				GetSamplesPerChunk() { return m_samplesPerBuffer; }	// Max num samples that the callback will ask for in a single call
    void			SetCallback(void (*_callback) (StereoSample *, unsigned int));
	void			TopupBuffer();
};


extern SoundDevice *g_soundDevice;
