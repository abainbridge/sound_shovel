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

// Standard headers
#include <math.h>
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

    m_h_offset = 0.0;
    m_h_offset_velocity = 0.0;
    m_h_zoom_ratio = m_target_h_zoom_ratio = -1.0;

    m_display_width = -1;
    m_display_mins = NULL;
    m_display_maxes = NULL;
}


static bool nearly_equal(double a, double b)
{
    double diff = fabs(a - b);
    return diff < 1e-5;
}


void SoundView::Advance()
{
    if (m_h_zoom_ratio < 0.0)
        return;

    double h_zoom_ratio_before = m_h_zoom_ratio;

    static double last_time = GetHighResTime();
    double now = GetHighResTime();
    double advance_time = now - last_time;
    last_time = now;


    //
    // Take mouse input

    double const ZOOM_INCREMENT = 1.2;
    if (g_inputManager.mouseVelZ < 0)
        m_target_h_zoom_ratio *= ZOOM_INCREMENT;
    else if (g_inputManager.mouseVelZ > 0)
        m_target_h_zoom_ratio /= ZOOM_INCREMENT;

    if (g_inputManager.mmb && g_inputManager.mouseVelX)
        m_h_offset_velocity = -g_inputManager.mouseVelX * m_h_zoom_ratio / (advance_time * 2.0);


    //
    // Take keyboard input

    const double KEY_ZOOM_INCREMENT = 1.0 + 1.3 * advance_time;
    if (g_inputManager.keys[KEY_UP])
        m_target_h_zoom_ratio /= KEY_ZOOM_INCREMENT;
    if (g_inputManager.keys[KEY_DOWN])
        m_target_h_zoom_ratio *= KEY_ZOOM_INCREMENT;

    double const KEY_H_SCROLL_IMPLUSE = 1600.0 * m_h_zoom_ratio * advance_time;
    if (g_inputManager.keys[KEY_LEFT])
        m_h_offset_velocity -= KEY_H_SCROLL_IMPLUSE;
    if (g_inputManager.keys[KEY_RIGHT])
        m_h_offset_velocity += KEY_H_SCROLL_IMPLUSE;


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

    // Zoom
    double zoom_blend_factor = advance_time * 8.0;
    m_h_zoom_ratio = (1.0 - zoom_blend_factor) * m_h_zoom_ratio + zoom_blend_factor * m_target_h_zoom_ratio;

    // H offset
    m_h_offset += m_h_offset_velocity * advance_time;
    m_h_offset_velocity -= m_h_offset_velocity * 2.7 * advance_time; // Friction


    //
    // Correct h_offset to compensate for zoom change this frame

    double delta_h_zoom = h_zoom_ratio_before / m_h_zoom_ratio;
    if (delta_h_zoom < 1.0)
        m_h_offset += (delta_h_zoom - 1.0) / 2.0 * m_display_width * m_h_zoom_ratio;
    else
        m_h_offset -= (1.0 - delta_h_zoom) / 2.0 * m_display_width * m_h_zoom_ratio;


    //
    // Enforce constraints

    if (m_h_offset < 0)
    {
        m_h_offset = 0.0;
        m_h_offset_velocity = 0.0;
    }
    else 
    {
        double max_h_offset = m_sound->GetLength() - m_display_width * m_h_zoom_ratio;
        if (max_h_offset < 0.0)
            max_h_offset = 0.0;

        if (m_h_offset > max_h_offset)
        {
            m_h_offset = max_h_offset;
            m_h_offset_velocity = 0.0;
        }
    }

    if (!nearly_equal(m_h_zoom_ratio, h_zoom_ratio_before) ||
        fabs(m_h_offset_velocity) > 1e-3)
    {
        g_can_sleep = false;
    }
}


void SoundView::Render(BitmapRGBA *bmp)
{
    if (m_h_zoom_ratio < 0.0)
    {
        m_h_zoom_ratio = (double)m_sound->GetLength() / (double)bmp->width;
        m_target_h_zoom_ratio = m_h_zoom_ratio;
    }

    UpdateDisplaySize(bmp->width);

    double v_zoom_ratio = (double)bmp->height / (65536 * m_sound->m_num_channels);

    RGBAColour sound_colour = Colour(60, 120, 220);
    int channel_height = bmp->height / m_sound->m_num_channels;

    for (int chan_idx = 0; chan_idx < m_sound->m_num_channels; chan_idx++)
    {
        SoundChannel *chan = m_sound->m_channels[chan_idx];

        chan->CalcDisplayData(m_h_offset, m_display_mins, m_display_maxes, m_display_width, m_h_zoom_ratio);

        int y_mid = channel_height * chan_idx + channel_height / 2;

        int prev_y = y_mid;
        for (unsigned x = 0; x < m_display_width; x++)
        {
            int vline_len = RoundToInt(m_display_maxes[x] - m_display_mins[x]) * v_zoom_ratio;
//            vline_len = 200 * v_zoom_ratio;
            VLine(bmp, x, ceil(y_mid - m_display_maxes[x] * v_zoom_ratio), vline_len, sound_colour);
//            VLine(bmp, x, y_mid - 100 * v_zoom_ratio, vline_len, sound_colour);
        }

        HLine(bmp, 0, y_mid - 32767 * v_zoom_ratio, m_display_width, Colour(255, 255, 255, 60));
        HLine(bmp, 0, y_mid, m_display_width, Colour(255, 255, 255, 60));
        HLine(bmp, 0, y_mid + 32767 * v_zoom_ratio, m_display_width, Colour(255, 255, 255, 60));
    }

    DrawTextRight(g_defaultTextRenderer, g_colourWhite, bmp, bmp->width - 5, bmp->height - 20, "Zoom: %.1f", m_h_zoom_ratio);
}
