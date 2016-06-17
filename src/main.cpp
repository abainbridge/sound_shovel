// Project headers
#include "sound.h"

#include "df_bitmap.h"
#include "df_hi_res_time.h"
#include "df_input.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <stdint.h>


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(1000, 600, WT_WINDOWED, "Sound Shovel");
    TextRenderer *font = CreateTextRenderer("Lucida Console", 10, 4);

//     SoundBlock *first = new SoundBlock;
//     SoundBlock *current = first;
//     unsigned num_blocks = 2637; // 2 hours @ 48 kHz.
//     for (int i = 0; i < num_blocks; i++)
//     {
//         current->m_next = new SoundBlock;
//         current = current->m_next;
//     }

    Sound sound;
    sound.LoadWav("c:/Users/Andy/Desktop/2016-03-18-raspbian-jessie-lite.img");
//    SoundBlock *first = LoadWav("h:/video/later_apr23.mpg");

    unsigned win_width = g_window->bmp->width;
    unsigned win_height = g_window->bmp->height;
    unsigned total_len = sound.GetLength();
    double h_zoom_ratio = (double)total_len / (double)win_width;
    int  h_offset = 0;
    double v_zoom_ratio = (double)win_height / 1e5;
    int y_mid = win_height / 2;

    int16_t *display_mins = new int16_t[win_width];
    int16_t *display_maxes = new int16_t[win_width];

    RGBAColour sound_colour = Colour(40, 40, 255);

    while (!g_window->windowClosed && !g_inputManager.keyDowns[KEY_ESC])
    {
        ClearBitmap(g_window->bmp, g_colourBlack);
        InputManagerAdvance();

        if (g_inputManager.mouseVelZ != 0)
        {
            double const ZOOM_INCREMENT = 1.2;
            if (g_inputManager.mouseVelZ < 0)
            {
                if (h_zoom_ratio * win_width < total_len)
                {
                    h_offset -= (ZOOM_INCREMENT - 1.0) / 2.0 * win_width * h_zoom_ratio;
                    h_zoom_ratio *= ZOOM_INCREMENT;
                }
            }
            else
            {
                if (int(h_zoom_ratio) > 1)
                {
                    h_offset += (1.0 - 1.0/ZOOM_INCREMENT) / 2.0 * win_width * h_zoom_ratio;
                    h_zoom_ratio /= ZOOM_INCREMENT;
                }
            }
        }

        if (g_inputManager.mmb && g_inputManager.mouseVelX)
        {
            h_offset -= g_inputManager.mouseVelX * h_zoom_ratio;
        }

        if (h_offset < 0)
            h_offset = 0;

        double start_time = GetHighResTime();
        sound.CalcDisplayData(h_offset, display_mins, display_maxes, win_width, h_zoom_ratio);
        double end_time = GetHighResTime();
        DebugOut("Time taken: %.4f\n", end_time - start_time);

        int prev_y = y_mid;
        for (unsigned x = 0; x < win_width; x++)
        {
           int vline_len = (display_maxes[x] - display_mins[x]) * v_zoom_ratio;
           VLine(g_window->bmp, x, y_mid - display_maxes[x] * v_zoom_ratio, vline_len, sound_colour);
        }

        HLine(g_window->bmp, 0, win_height / 2, win_width, Colour(255, 255, 255, 60));

        // Display time taken to calc display buffer
        DrawTextRight(font, g_colourWhite, g_window->bmp, g_window->bmp->width - 5, 0, "Calc time (ms):%3.1f", (end_time - start_time) * 1000.0);

        DrawTextRight(font, g_colourWhite, g_window->bmp, g_window->bmp->width - 5, win_height - 20, "Zoom: %.1f", h_zoom_ratio);

        UpdateWin();
        SleepMillisec(10);
    }

    return 0;
}
