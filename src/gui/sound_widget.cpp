// Own header
#include "sound_widget.h"

// Project headers
#include "app_gui_manager.h"
#include "main.h"
#include "sound.h"
#include "sound_channel.h"
#include "sound_system.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_common.h"
#include "df_input.h"
#include "df_time.h"
#include "df_font.h"
#include "df_window.h"

// Standard headers
#include <math.h>
#include <stdlib.h>


// ***************************************************************************
// Private Methods
// ***************************************************************************

void SoundWidget::AdvanceSelection()
{
    if (g_guiManager->m_focussedWidget == this)
    {
        if (g_input.lmbClicked)
        {
            m_selectionStart = GetSampleIndexFromScreenPos(g_input.mouseX);
            m_selectionEnd = -1;
        }
        else if (g_input.lmb)
        {
            m_selectionEnd = GetSampleIndexFromScreenPos(g_input.mouseX);
        }
    }
}


void SoundWidget::AdvancePlaybackPos()
{
    double targetIdx = m_playbackIdx;
    if (targetIdx < 0)
        return;

    if (fabs(targetIdx - m_playbackPos) > 10000.0)
    {
        m_playbackPos = targetIdx;
    }
    else
    {
        m_playbackPos = targetIdx * 0.1 + m_playbackPos * 0.9;
    }
}


void SoundWidget::RenderMarker(DfBitmap *bmp, int64_t sampleIdx, DfColour col)
{
    double x = GetScreenPosFromSampleIndex(sampleIdx);
    double fractionalX = x - (int64_t)x;
    float const gammaCorrectionFactor = 0.75;
    DfColour c = col;
    c.a = col.a * powf(1.0 - fractionalX, gammaCorrectionFactor);
    VLine(bmp, floor(x), m_top, m_height, c);
    c.a = col.a * powf(fractionalX, gammaCorrectionFactor);
    VLine(bmp, ceil(x), m_top, m_height, c);
}


void SoundWidget::RenderWaveform(DfBitmap *bmp, double vZoomRatio)
{
    DfColour soundColour = Colour(52, 152, 219);
    int channelHeight = m_height / m_sound->m_numChannels;

    for (int chanIdx = 0; chanIdx < m_sound->m_numChannels; chanIdx++)
    {
        SoundChannel *chan = m_sound->m_channels[chanIdx];

        chan->CalcDisplayData(m_hOffset, m_displayMins, m_displayMaxes, m_width, m_hZoomRatio);

        int yMid = m_top + channelHeight * chanIdx + channelHeight / 2;

        for (unsigned x = 0; x < m_width; x++)
        {
            int vline_len = RoundToInt(m_displayMaxes[x] - m_displayMins[x]) * vZoomRatio;
            int y = ceil(yMid - m_displayMaxes[x] * vZoomRatio);
            if (vline_len == 0)
                PutPix(bmp, m_left + x, y, soundColour);
            else
                VLine(bmp, m_left + x, y, vline_len, soundColour);
        }

        HLine(bmp, m_left, yMid, m_width, Colour(255, 255, 255, 60));
        HLine(bmp, m_left, yMid + 32767 * vZoomRatio, m_width, Colour(255, 255, 255, 60));
    }
}


void SoundWidget::RenderSelection(DfBitmap *bmp)
{
    DfColour col = Colour(255, 40, 59, 63);

    if (m_selectionEnd >= 0)
    {
        int64_t selectionStart = m_selectionStart;
        int64_t selectionEnd = m_selectionEnd;
        if (selectionEnd < selectionStart)
        {
            selectionStart = m_selectionEnd;
            selectionEnd = m_selectionStart;
        }

        double x1 = GetScreenPosFromSampleIndex(selectionStart);
        double x2 = GetScreenPosFromSampleIndex(selectionEnd);
        RectFill(bmp, x1, m_top, x2 - x1 + 1, m_height, col);
    }
    else
    {
        RenderMarker(bmp, m_selectionStart, col);
    }
}


// ***************************************************************************
// Public Methods
// ***************************************************************************

SoundWidget::SoundWidget(Widget *parent)
    : Widget(SOUND_VIEW_NAME, parent)
{
    m_hOffset = 0.0;
    m_targetHOffset = 0.0;
    m_hZoomRatio = m_targetHZoomRatio = -1.0;

    m_playbackPos = -1.0;

    m_displayMins = NULL;
    m_displayMaxes = NULL;

    m_selectionStart = -1.0;
    m_selectionEnd = -1.0;

    m_sound = new Sound;
    m_sound->LoadWav("C:/users/andy/desktop/andante.wav");
    m_playbackIdx = 0;
    m_isPlaying = true;
    g_soundSystem->PlaySound(this);
}


void SoundWidget::GetSelectionBlock(int64_t *startIdx, int64_t *endIdx)
{
    if (m_selectionStart < m_selectionEnd)
    {
        *startIdx = m_selectionStart;
        *endIdx = m_selectionEnd;
    }
    else
    {
        *startIdx = m_selectionEnd;
        *endIdx = m_selectionStart;
    }
}


void SoundWidget::TogglePlayback()
{
    m_isPlaying = !m_isPlaying;
}


void SoundWidget::Play()
{
    m_isPlaying = true;
}


void SoundWidget::Pause()
{
    m_isPlaying = false;
}


void SoundWidget::FadeIn()
{
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->FadeIn(startIdx, endIdx);
}


void SoundWidget::FadeOut()
{
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->FadeOut(startIdx, endIdx);
}


void SoundWidget::Normalize()
{
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->Normalize(startIdx, endIdx);
}


static bool NearlyEqual(double a, double b)
{
    double diff = fabs(a - b);
    return diff < 1e-5;
}


void SoundWidget::SetRect(int x /* = -1 */, int y /* = -1 */, int w /* = -1 */, int h /* = -1 */)
{  
    if (w != m_width)
    {
        delete[] m_displayMins;
        delete[] m_displayMaxes;

        m_displayMins = new int16_t[w];
        m_displayMaxes = new int16_t[w];
    }

    Widget::SetRect(x, y, w, h);
}


void SoundWidget::Advance()
{
    if (m_hZoomRatio < 0.0)
        return;

    AdvanceSelection();

    double hZoomRatioBefore = m_hZoomRatio;
    double maxHOffset = m_sound->GetLength() - m_width * m_hZoomRatio;
    maxHOffset = IntMax(0.0, maxHOffset);

    double advanceTime = g_window->advanceTime;


    //
    // Take mouse input

    double const ZOOM_INCREMENT = 1.2;
    if (g_input.mouseVelZ < 0)
        m_targetHZoomRatio *= ZOOM_INCREMENT;
    else if (g_input.mouseVelZ > 0)
        m_targetHZoomRatio /= ZOOM_INCREMENT;

    if (g_input.mmb)
    {
        m_hOffset -= g_input.mouseVelX * m_hZoomRatio;
        m_targetHOffset = m_hOffset - g_input.mouseVelX * m_hZoomRatio;
    }
    else if (g_input.mmbUnClicked)
    {
        m_targetHOffset -= g_input.mouseVelX * m_hZoomRatio * 50.0;
    }

    if (g_input.lmbClicked && IsMouseInBounds() && m_isPlaying)
    { 
        m_playbackIdx = GetSampleIndexFromScreenPos(g_input.mouseX);
    }


    //
    // Take keyboard input

    const double KEY_ZOOM_INCREMENT = 1.0 + 1.3 * advanceTime;
    if (g_input.keys[KEY_UP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT;
    if (g_input.keys[KEY_DOWN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT;
    if (g_input.keyDowns[KEY_PGUP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT * 8.0;
    if (g_input.keyDowns[KEY_PGDN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT * 8.0;

    double const KEY_H_SCROLL_IMPLUSE = 1000.0 * m_hZoomRatio * advanceTime;
    if (g_input.keys[KEY_LEFT])
        m_targetHOffset -= KEY_H_SCROLL_IMPLUSE;
    if (g_input.keys[KEY_RIGHT])
        m_targetHOffset += KEY_H_SCROLL_IMPLUSE;

    if (g_input.keyDowns[KEY_HOME])
    {
        m_targetHOffset = 0;
    }
    else if (g_input.keyDowns[KEY_END])
    {
        double samples_on_screen = m_width * m_hZoomRatio;
        m_targetHOffset = m_sound->GetLength() - samples_on_screen;
    }

    if (g_input.keyDowns[KEY_SPACE])
        m_isPlaying = !m_isPlaying;


    //
    // Enforce constraints

    double maxHZoomRatio = m_sound->GetLength() / (double)m_width;
    if (m_targetHZoomRatio > maxHZoomRatio)
        m_targetHZoomRatio = maxHZoomRatio;

    double minHZoomRatio = 1.0;
    if (m_targetHZoomRatio < minHZoomRatio)
        m_targetHZoomRatio = 1.0;


    //
    // Do physics

    // Zoom
    double zoomBlendFactor = advanceTime * 8.0;
    m_hZoomRatio = (1.0 - zoomBlendFactor) * m_hZoomRatio + zoomBlendFactor * m_targetHZoomRatio;

    // H offset
    {
        double factor1 = 3.0 * advanceTime;
        double factor2 = 1.0 - factor1;
        m_hOffset = factor2 * m_hOffset + factor1 * m_targetHOffset;
    }


    //
    // Correct h_offset to compensate for zoom change this frame.

    {
        // The aim is to keep some part of the visible waveform in the same
        // place on the display. If there is a selection start, we use that,
        // otherwise we just use the sample that is currently in the middle
        // of the display.
        double numSamplesVisible = m_width * hZoomRatioBefore;
        double keepThing = (m_selectionStart - m_hOffset) / numSamplesVisible;
        if (keepThing < 0.0 || keepThing > 1.0)
            keepThing = 0.5;

        double deltaHZoom = hZoomRatioBefore / m_hZoomRatio;

        if (deltaHZoom < 1.0)
        {
            double tmp = (deltaHZoom - 1.0) * numSamplesVisible;
            tmp *= keepThing / deltaHZoom;
            m_targetHOffset += tmp;
            m_hOffset += tmp;
        }
        else
        {
            double tmp = -((1.0 - deltaHZoom) * numSamplesVisible);
            tmp *= keepThing / deltaHZoom;
            m_targetHOffset += tmp;
            m_hOffset += tmp;
        }
    }


    //
    // Enforce constraints

    m_hOffset = ClampDouble(m_hOffset, 0.0, maxHOffset);
    m_targetHOffset = ClampDouble(m_targetHOffset, 0.0, maxHOffset);
    if (m_hZoomRatio < 1.0)
        m_hZoomRatio = 1.0;

    if (!NearlyEqual(m_hZoomRatio, m_targetHZoomRatio) ||
        !NearlyEqual(m_hOffset, m_targetHOffset))
    {
        g_canSleep = false;
    }

    AdvancePlaybackPos();
}


void SoundWidget::Render()
{
    if (m_hZoomRatio < 0.0)
    {
        m_hZoomRatio = (double)m_sound->GetLength() / (double)m_width;
        m_targetHZoomRatio = m_hZoomRatio;
    }

    double vZoomRatio = (double)m_height / (65536 * m_sound->m_numChannels);

    RenderWaveform(g_window->bmp, vZoomRatio);
    RenderSelection(g_window->bmp);

    if (m_playbackIdx)
        RenderMarker(g_window->bmp, m_playbackPos, Colour(255, 255, 255, 90));
}


char *SoundWidget::ExecuteCommand(char const *object, char const *command, char const *arguments)
{
    if (COMMAND_IS("FadeIn"))           FadeIn();
    else if (COMMAND_IS("FadeOut"))     FadeOut();
    else if (COMMAND_IS("Normalize"))   Normalize();
    else if (COMMAND_IS("Play"))        Play();
    else if (COMMAND_IS("Pause"))       Pause();
    else if (COMMAND_IS("Save"))        m_sound->SaveWav();
    else if (COMMAND_IS("TogglePlay"))  TogglePlayback();

    return NULL;
}


int64_t SoundWidget::GetSampleIndexFromScreenPos(int screenX)
{
    int64_t rv = m_hOffset + (double)screenX * m_hZoomRatio + 0.5;
    if (rv < 0 || rv > m_sound->GetLength()) 
        rv = -1;
    return rv;
}


double SoundWidget::GetScreenPosFromSampleIndex(int64_t sampleIdx)
{
    return m_left + ((double)sampleIdx - m_hOffset) / m_hZoomRatio;
}
