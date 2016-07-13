#pragma once


// Contrib headers
#include "df_rgba_colour.h"
#include "gui/widget.h"

// Standard headers
#include <stdint.h>


#define SOUND_VIEW_NAME "SoundView"


struct BitmapRGBA;
class Sound;


class SoundView: public Widget
{
private:
    double m_targetHZoomRatio;
    double m_targetHOffset;
    double m_playbackPos;

    void AdvanceSelection();
    void AdvancePlaybackPos();

    void RenderMarker(BitmapRGBA *bmp, int64_t sample_idx, RGBAColour col);

    void RenderWaveform(BitmapRGBA *bmp, double v_zoom_ratio);
    void RenderSelection(BitmapRGBA *bmp);

public:
    Sound *m_sound;

    double m_hOffset;
    double m_hZoomRatio;

    int16_t *m_displayMins;    // An array of length m_width.
    int16_t *m_displayMaxes;   // An array of length m_width.

    int64_t m_selectionStart;
    int64_t m_selectionEnd;        // Set to -1 if no selection.

    SoundView(Sound *sound, Widget *parent);

    // Overridden Widget functions
    void SetRect(int x = -1, int y = -1, int w = -1, int h = -1);
    void Advance();
    void Render();
    char *ExecuteCommand(char const *object, char const *command, char const *arguments) { return NULL; }

    int64_t GetSampleIndexFromScreenPos(int screenX);
    double GetScreenPosFromSampleIndex(int64_t sampleIdx);
};
