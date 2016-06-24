// Own header
#include "sound_view.h"

// Project headers
#include "sound.h"
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


SoundView::SoundView(Sound *sound)
{
    m_sound = sound;

    m_h_offset = m_target_h_offset = 0;
    m_h_zoom_ratio = m_target_h_zoom_ratio = -1.0;

    m_display_width = -1;
    m_display_mins = NULL;
    m_display_maxes = NULL;
}


void SoundView::Advance()
{
    if (m_h_zoom_ratio < 0.0)
        return;

    double h_zoom_ratio_before = m_h_zoom_ratio;


    //
    // Take mouse input

    double const ZOOM_INCREMENT = 1.2;
    if (g_inputManager.mouseVelZ < 0)
        m_target_h_zoom_ratio *= ZOOM_INCREMENT;
    else if (g_inputManager.mouseVelZ > 0)
        m_target_h_zoom_ratio /= ZOOM_INCREMENT;

    if (g_inputManager.mmb && g_inputManager.mouseVelX)
    {
        m_h_offset -= g_inputManager.mouseVelX * m_h_zoom_ratio;
    }


    //
    // Take keyboard input

    static double last_time = GetHighResTime();
    double now = GetHighResTime();
    double advance_time = now - last_time;

    const double KEY_ZOOM_INCREMENT = 1.03;
    if (g_inputManager.keys[KEY_UP])
        m_target_h_zoom_ratio /= KEY_ZOOM_INCREMENT;
    if (g_inputManager.keys[KEY_DOWN])
        m_target_h_zoom_ratio *= KEY_ZOOM_INCREMENT;

    double const KEY_H_SCROLL_INCREMENT = 400.0 * m_h_zoom_ratio * advance_time;
    if (g_inputManager.keys[KEY_LEFT])
        m_h_offset -= KEY_H_SCROLL_INCREMENT;
    if (g_inputManager.keys[KEY_RIGHT])
        m_h_offset += KEY_H_SCROLL_INCREMENT;


    //
    // Enforce constraints

    double max_h_zoom_ratio = m_sound->GetLength() / (double)m_display_width;
    if (m_target_h_zoom_ratio > max_h_zoom_ratio)
        m_target_h_zoom_ratio = max_h_zoom_ratio;

    double min_h_zoom_ratio = 1.0;
    if (m_target_h_zoom_ratio < min_h_zoom_ratio)
        m_target_h_zoom_ratio = 1.0;


    //
    // Do physics

    double force = advance_time * 8.0;
    m_h_zoom_ratio = (1.0 - force) * m_h_zoom_ratio + force * m_target_h_zoom_ratio;
    last_time = now;

    double delta_h_zoom = h_zoom_ratio_before / m_h_zoom_ratio;
    if (delta_h_zoom < 1.0)
        m_h_offset += (delta_h_zoom - 1.0) / 2.0 * m_display_width * m_h_zoom_ratio;
    else
        m_h_offset -= (1.0 - delta_h_zoom) / 2.0 * m_display_width * m_h_zoom_ratio;

    // Enforce constraints

    if (m_h_offset < 0)
        m_h_offset = 0;
}


void SoundView::Render(BitmapRGBA *bmp)
{
    if (m_h_zoom_ratio < 0.0)
    {
        m_h_zoom_ratio = (double)m_sound->GetLength() / (double)bmp->width;
        m_target_h_zoom_ratio = m_h_zoom_ratio;
    }

    UpdateDisplaySize(bmp->width);

    double v_zoom_ratio = (double)bmp->height / 1e5;

    RGBAColour sound_colour = Colour(80, 80, 240);
    double start_time = GetHighResTime();
    double end_time = start_time;

    int channel_height = bmp->height / m_sound->m_num_channels;

    for (int chan_idx = 0; chan_idx < m_sound->m_num_channels; chan_idx++)
    {
        SoundChannel *chan = m_sound->m_channels[chan_idx];

        chan->CalcDisplayData(m_h_offset, m_display_mins, m_display_maxes, m_display_width, m_h_zoom_ratio);
        end_time = GetHighResTime();

        int y_mid = channel_height * chan_idx + channel_height / 2;

        int prev_y = y_mid;
        for (unsigned x = 0; x < m_display_width; x++)
        {
            int vline_len = (m_display_maxes[x] - m_display_mins[x]) * v_zoom_ratio;
            VLine(bmp, x, y_mid - m_display_maxes[x] * v_zoom_ratio, vline_len, sound_colour);
        }

        HLine(bmp, 0, y_mid, m_display_width, Colour(255, 255, 255, 60));
    }

    // Display time taken to calc display buffer
    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, 0, "Calc time (ms):%3.1f", (end_time - start_time) * 1000.0);

    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, bmp->height - 20, "Zoom: %.1f", m_h_zoom_ratio);
}
