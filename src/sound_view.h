#pragma once


#include <stdint.h>


struct BitmapRGBA;
class Sound;


class SoundView
{
private:
    void UpdateDisplaySize(int pixelWidth);

    double m_target_h_zoom_ratio;
    double m_h_offset_velocity;

public:
    Sound *m_sound;

    double m_h_offset;
    double m_h_zoom_ratio;

    int m_display_width;
    int16_t *m_display_mins;    // An array of length m_display_width.
    int16_t *m_display_maxes;   // An array of length m_display_width.

    SoundView(Sound *sound);

    void Advance();
    void Render(BitmapRGBA *bmp);
};
