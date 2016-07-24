#pragma once


// Contrib headers
#include "df_colour.h"
#include "gui/widget.h"

// Standard headers
#include <stdint.h>


#define SOUND_VIEW_NAME "SoundWidget"


struct DfBitmap;
class Sound;


class SoundWidget: public Widget
{
private:
    double m_targetHZoomRatio;
    double m_targetHOffset;
    double m_playbackPos;       // This value smoothly tracks m_playbackIdx. The point is to hide the fact that m_playbackIdx is advanced in audio-buffer sized steps, which looks jerky when zoomed in.

    int64_t m_selectionStart;
    int64_t m_selectionEnd;        // Set to -1 if no selection.

    void AdvanceSelection();
    void AdvancePlaybackPos();

    void RenderMarker(DfBitmap *bmp, int64_t sample_idx, DfColour col);

    void RenderWaveform(DfBitmap *bmp, double v_zoom_ratio);
    void RenderSelection(DfBitmap *bmp);

public:
    Sound *m_sound;

    int64_t m_playbackIdx;
    bool m_isPlaying;

    double m_hOffset;
    double m_hZoomRatio;

    int16_t *m_displayMins;    // An array of length m_width.
    int16_t *m_displayMaxes;   // An array of length m_width.

    SoundWidget(Widget *parent);

    bool Open(char const *filename);
    bool OpenDialog();
    void Close();

    void GetSelectionBlock(int64_t *startIdx, int64_t *endIdx);

    void TogglePlayback();
    void Play();
    void Pause();

    void FadeIn();
    void FadeOut();
    void Normalize();

    // Overridden Widget functions
    void SetRect(int x = -1, int y = -1, int w = -1, int h = -1);
    void Advance();
    void Render();
    char *ExecuteCommand(char const *object, char const *command, char const *arguments);

    int64_t GetSampleIndexFromScreenPos(int screenX);
    double GetScreenPosFromSampleIndex(int64_t sampleIdx);
};
