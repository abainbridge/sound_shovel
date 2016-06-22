#pragma once


#include <stdint.h>


struct BitmapRGBA;
class SoundChannel;


class SoundView
{
private:
    void UpdateDisplaySize(int pixelWidth);

public:
    SoundChannel *m_sound;

    int64_t m_h_offset;

    int m_display_width;
    int16_t *m_display_mins;    // An array of length m_display_width.
    int16_t *m_display_maxes;   // An array of length m_display_width.

    double m_h_zoom_ratio;

    SoundView(SoundChannel *sound);

    void Advance();
    void Render(BitmapRGBA *bmp);
};
