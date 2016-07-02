#pragma once


#include <stdint.h>


struct BitmapRGBA;
class Sound;


class SoundView
{
private:
    double m_target_h_zoom_ratio;
    double m_h_offset_velocity;
    double m_playback_pos;

    void UpdateDisplaySize(int pixel_width);
    void AdvanceSelection();
    void AdvancePlaybackPos();

    void RenderWaveform(BitmapRGBA *bmp, double v_zoom_ratio);
    void RenderSelection(BitmapRGBA *bmp);

public:
    Sound *m_sound;

    double m_h_offset;
    double m_h_zoom_ratio;

    int m_display_width;
    int16_t *m_display_mins;    // An array of length m_display_width.
    int16_t *m_display_maxes;   // An array of length m_display_width.

    int64_t m_selection_start;
    int64_t m_selection_end;        // Set to -1 if no selection.

    SoundView(Sound *sound);

    void Advance();
    void Render(BitmapRGBA *bmp);

    int64_t GetSampleIndexFromScreenPos(int screen_x);
    double GetScreenPosFromSampleIndex(int64_t sample_idx);
};
