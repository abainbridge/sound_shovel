// Own header
#include "sound_view.h"

// Project headers
#include "main.h"
#include "sound.h"
#include "sound_channel.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_common.h"
#include "df_input.h"
#include "df_hi_res_time.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

// Standard headers
#include <math.h>
#include <stdlib.h>


// ***************************************************************************
// Private Methods
// ***************************************************************************

void SoundView::UpdateDisplaySize(int pixelWidth)
{
    if (m_displayWidth == pixelWidth)
        return;

    delete[] m_displayMins;
    delete[] m_displayMaxes;

    m_displayWidth = pixelWidth;
    m_displayMins = new int16_t[pixelWidth];
    m_displayMaxes = new int16_t[pixelWidth];
}


void SoundView::AdvanceSelection()
{
    if (g_inputManager.lmbClicked)
    {
        m_selectionStart = GetSampleIndexFromScreenPos(g_inputManager.mouseX);
    }
}


void SoundView::AdvancePlaybackPos()
{
    double targetIdx = m_sound->m_playbackIdx;
    if (targetIdx < 0)
        return;

    if (targetIdx > m_playbackPos + 10000.0)
    {
        m_playbackPos = targetIdx;
    }
    else
    {
        m_playbackPos = targetIdx * 0.1 + m_playbackPos * 0.9;
    }
}


void SoundView::RenderMarker(BitmapRGBA *bmp, int64_t sampleIdx, RGBAColour col)
{
    double x = GetScreenPosFromSampleIndex(sampleIdx);
    double fractionalX = x - (int64_t)x;
    float const gammaCorrectionFactor = 0.75;
    RGBAColour c = col;
    c.a = col.a * powf(1.0 - fractionalX, gammaCorrectionFactor);
    VLine(bmp, floor(x), 0, bmp->height, c);
    c.a = col.a * powf(fractionalX, gammaCorrectionFactor);
    VLine(bmp, ceil(x), 0, bmp->height, c);
}


void SoundView::RenderWaveform(BitmapRGBA *bmp, double vZoomRatio)
{
    RGBAColour soundColour = Colour(52, 152, 219);
    int channelHeight = bmp->height / m_sound->m_numChannels;

    for (int chanIdx = 0; chanIdx < m_sound->m_numChannels; chanIdx++)
    {
        SoundChannel *chan = m_sound->m_channels[chanIdx];

        chan->CalcDisplayData(m_hOffset, m_displayMins, m_displayMaxes, m_displayWidth, m_hZoomRatio);

        int yMid = channelHeight * chanIdx + channelHeight / 2;

        for (unsigned x = 0; x < m_displayWidth; x++)
        {
            int vline_len = RoundToInt(m_displayMaxes[x] - m_displayMins[x]) * vZoomRatio;
            int y = ceil(yMid - m_displayMaxes[x] * vZoomRatio);
            if (vline_len == 0)
                PutPix(bmp, x, y, soundColour);
            else
                VLine(bmp, x, y, vline_len, soundColour);
        }

        HLine(bmp, 0, yMid - 32767 * vZoomRatio, m_displayWidth, Colour(255, 255, 255, 60));
        HLine(bmp, 0, yMid, m_displayWidth, Colour(255, 255, 255, 60));
        HLine(bmp, 0, yMid + 32767 * vZoomRatio, m_displayWidth, Colour(255, 255, 255, 60));
    }
}


void SoundView::RenderSelection(BitmapRGBA *bmp)
{
    RenderMarker(bmp, m_selectionStart, Colour(255, 255, 50, 90));
}


// ***************************************************************************
// Public Methods
// ***************************************************************************

SoundView::SoundView(Sound *sound)
{
    m_sound = sound;

    m_hOffset = 0.0;
    m_targetHOffset = 0.0;
    m_hZoomRatio = m_targetHZoomRatio = -1.0;

    m_playbackPos = -1.0;

    m_displayWidth = -1;
    m_displayMins = NULL;
    m_displayMaxes = NULL;

    m_selectionStart = -1.0;
    m_selectionEnd = -1.0;
}


static bool NearlyEqual(double a, double b)
{
    double diff = fabs(a - b);
    return diff < 1e-5;
}


void SoundView::Advance()
{
    if (m_hZoomRatio < 0.0)
        return;

    AdvanceSelection();

    double hZoomRatioBefore = m_hZoomRatio;
    double maxHOffset = m_sound->GetLength() - m_displayWidth * m_hZoomRatio;
    maxHOffset = IntMax(0.0, maxHOffset);

    double advanceTime = g_window->advanceTime;


    //
    // Take mouse input

    double const ZOOM_INCREMENT = 1.2;
    if (g_inputManager.mouseVelZ < 0)
        m_targetHZoomRatio *= ZOOM_INCREMENT;
    else if (g_inputManager.mouseVelZ > 0)
        m_targetHZoomRatio /= ZOOM_INCREMENT;

    if (g_inputManager.mmb)
    {
        m_hOffset -= g_inputManager.mouseVelX * m_hZoomRatio;
        m_targetHOffset = m_hOffset - g_inputManager.mouseVelX * m_hZoomRatio;
    }
    else if (g_inputManager.mmbUnClicked)
    {
        m_targetHOffset -= g_inputManager.mouseVelX * m_hZoomRatio * 50.0;
    }


    //
    // Take keyboard input

    const double KEY_ZOOM_INCREMENT = 1.0 + 1.3 * advanceTime;
    if (g_inputManager.keys[KEY_UP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT;
    if (g_inputManager.keys[KEY_DOWN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT;
    if (g_inputManager.keyDowns[KEY_PGUP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT * 8.0;
    if (g_inputManager.keyDowns[KEY_PGDN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT * 8.0;

    double const KEY_H_SCROLL_IMPLUSE = 1000.0 * m_hZoomRatio * advanceTime;
    if (g_inputManager.keys[KEY_LEFT])
        m_targetHOffset -= KEY_H_SCROLL_IMPLUSE;
    if (g_inputManager.keys[KEY_RIGHT])
        m_targetHOffset += KEY_H_SCROLL_IMPLUSE;

    if (g_inputManager.keyDowns[KEY_HOME])
    {
        m_targetHOffset = 0;
    }
    else if (g_inputManager.keyDowns[KEY_END])
    {
        double samples_on_screen = m_displayWidth * m_hZoomRatio;
        m_targetHOffset = m_sound->GetLength() - samples_on_screen;
    }


    //
    // Enforce constraints

    double maxHZoomRatio = m_sound->GetLength() / (double)m_displayWidth;
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
        double numSamplesVisible = m_displayWidth * hZoomRatioBefore;
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


void SoundView::Render(BitmapRGBA *bmp)
{
    if (m_hZoomRatio < 0.0)
    {
        m_hZoomRatio = (double)m_sound->GetLength() / (double)bmp->width;
        m_targetHZoomRatio = m_hZoomRatio;
    }

    UpdateDisplaySize(bmp->width);

    double vZoomRatio = (double)bmp->height / (65536 * m_sound->m_numChannels);

    RenderWaveform(bmp, vZoomRatio);
    RenderSelection(bmp);

    if (m_sound->m_playbackIdx)
        RenderMarker(bmp, m_playbackPos, Colour(255, 255, 255, 90));

    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, bmp->height - 20, "Zoom: %.1f", m_hZoomRatio);
}


int64_t SoundView::GetSampleIndexFromScreenPos(int screenX)
{
    return m_hOffset + (double)screenX * m_hZoomRatio + 0.5;
}


double SoundView::GetScreenPosFromSampleIndex(int64_t sampleIdx)
{
    return ((double)sampleIdx - m_hOffset) / m_hZoomRatio;
}
