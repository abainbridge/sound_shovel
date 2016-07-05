#pragma once


// Contrib headers
#include "df_rgba_colour.h"

// Standard headers
#include <stdint.h>


struct BitmapRGBA;
class Sound;


class SoundView
{
private:
    double m_targetHZoomRatio;
    double m_targetHOffset;
    double m_playbackPos;

    void UpdateDisplaySize(int pixel_width);
    void AdvanceSelection();
    void AdvancePlaybackPos();

    void RenderMarker(BitmapRGBA *bmp, int64_t sample_idx, RGBAColour col);

    void RenderWaveform(BitmapRGBA *bmp, double v_zoom_ratio);
    void RenderSelection(BitmapRGBA *bmp);

public:
    Sound *m_sound;

    double m_hOffset;
    double m_hZoomRatio;

    int m_displayWidth;
    int16_t *m_displayMins;    // An array of length m_displayWidth.
    int16_t *m_displayMaxes;   // An array of length m_displayWidth.

    int64_t m_selectionStart;
    int64_t m_selectionEnd;        // Set to -1 if no selection.

    SoundView(Sound *sound);

    void Advance();
    void Render(BitmapRGBA *bmp);

    int64_t GetSampleIndexFromScreenPos(int screenX);
    double GetScreenPosFromSampleIndex(int64_t sampleIdx);
};
