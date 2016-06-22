// Own header
#include "sound_view.h"

// Project headers
#include "sound_channel.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_input.h"
#include "df_hi_res_time.h"
#include "df_text_renderer.h"

// Standard headers
#include <stdlib.h>


void SoundView::UpdateDisplaySize(int pixelWidth)
{
    if (m_display_width == pixelWidth)
        return;

    delete[] m_display_mins;
    delete[] m_display_maxes;

    m_display_width = pixelWidth;
    m_display_mins = new int16_t[pixelWidth];
    m_display_maxes = new int16_t[pixelWidth];
}


SoundView::SoundView(SoundChannel *sound)
{
    m_sound = sound;

    m_h_offset = 0;

    m_display_width = -1;
    m_display_mins = NULL;
    m_display_maxes = NULL;

    m_h_zoom_ratio = -1.0;
}


void SoundView::Advance()
{
    if (g_inputManager.mouseVelZ != 0)
    {
        double const ZOOM_INCREMENT = 1.2;
        if (g_inputManager.mouseVelZ < 0)
        {
            if (m_h_zoom_ratio * m_display_width < m_sound->GetLength())
            {
                m_h_offset -= (ZOOM_INCREMENT - 1.0) / 2.0 * m_display_width * m_h_zoom_ratio;
                m_h_zoom_ratio *= ZOOM_INCREMENT;
            }
        }
        else
        {
            if (m_h_zoom_ratio > 1.0)
            {
                m_h_offset += (1.0 - 1.0 / ZOOM_INCREMENT) / 2.0 * m_display_width * m_h_zoom_ratio;
                m_h_zoom_ratio /= ZOOM_INCREMENT;
            }
        }
    }

    if (g_inputManager.mmb && g_inputManager.mouseVelX)
    {
        m_h_offset -= g_inputManager.mouseVelX * m_h_zoom_ratio;
    }

    if (m_h_offset < 0)
        m_h_offset = 0;
}


void SoundView::Render(BitmapRGBA *bmp)
{
    if (m_h_zoom_ratio < 0.0)
        m_h_zoom_ratio = (double)m_sound->GetLength() / (double)bmp->width;

    UpdateDisplaySize(bmp->width);

    double v_zoom_ratio = (double)bmp->height / 1e5;
    int y_mid = bmp->height / 2;

    RGBAColour sound_colour = Colour(80, 80, 240);
    double start_time = GetHighResTime();
    m_sound->CalcDisplayData(m_h_offset, m_display_mins, m_display_maxes, m_display_width, m_h_zoom_ratio);
    double end_time = GetHighResTime();

    int prev_y = y_mid;
    for (unsigned x = 0; x < m_display_width; x++)
    {
        int vline_len = (m_display_maxes[x] - m_display_mins[x]) * v_zoom_ratio;
        VLine(bmp, x, y_mid - m_display_maxes[x] * v_zoom_ratio, vline_len, sound_colour);
    }

    HLine(bmp, 0, bmp->height / 2, m_display_width, Colour(255, 255, 255, 60));

    // Display time taken to calc display buffer
    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, 0, "Calc time (ms):%3.1f", (end_time - start_time) * 1000.0);

    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, bmp->height - 20, "Zoom: %.1f", m_h_zoom_ratio);
}
