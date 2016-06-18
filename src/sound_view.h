#pragma once


#include <stdint.h>


struct BitmapRGBA;
class Sound;


class SoundView
{
private:
    void UpdateDisplaySize(int pixelWidth);

public:
    Sound *m_sound;

    int64_t m_h_offset;

    int m_display_width;
    int16_t *m_display_mins;    // An array of length m_display_width.
    int16_t *m_display_maxes;   // An array of length m_display_width.

    double m_h_zoom_ratio;

    SoundView(Sound *sound);

    void Advance();
    void Render(BitmapRGBA *bmp);
};
