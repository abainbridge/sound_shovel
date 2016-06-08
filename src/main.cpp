#include "df_hi_res_time.h"
#include "df_input.h"
#include "df_bitmap.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>



struct SoundBlock
{
    enum { MAX_SAMPLES = 131072 };

    SoundBlock *m_next;
    int16_t m_samples[MAX_SAMPLES];
    unsigned m_len;   // Number of valid items in m_samples

    SoundBlock()
    {
        m_next = NULL;
        m_len = MAX_SAMPLES;

        for (unsigned i = 0; i < m_len; i++)
            m_samples[i] = i - 32768;
    }
};


struct SoundIterator
{
    SoundBlock *m_block;
    unsigned m_current_idx;

    SoundIterator(SoundBlock *block, unsigned current_idx)
    {
        m_block = block;
        m_current_idx = current_idx;
    }

    int16_t GetNextSample()
    {
        if (!m_block)
            return 0;

        if (m_current_idx >= m_block->m_len)
        {
            m_block = m_block->m_next;
            m_current_idx = 0;
            if (!m_block)
                return 0;
        }

        int16_t rv = m_block->m_samples[m_current_idx];
        m_current_idx++;
        return rv;
    }
};


// int16_t buf[1500];
// SoundIterator it = { first_block, 0 };
// double zoom_ratio = 10 ^ 9 / 1500.0;
// for (int x = 0; x < 1500;) {
//     unsigned start_idx = x * zoom_ratio;
//     unsigned end_idx = (x + 1) * zoom_ratio - 1;
// 
//     buf[x] = get_max(&it, start_idx, end_idx);
// }
// 
// 
// int16_t get_max(SoundIterator *it, unsigned start_idx, unsigned end_idx)
// {
//     int16_t max = MIN_INT16;
//     if (start_idx & 4095 == 0) // We're on a 4096 boundary
// }
// 
// 
// int16_t get_max_upto_4096_boundary(SoundIterator *it, unsigned *start_idx)
// {
//     int16_t m = MIN_INT16;
//     unsigned end_idx = (start_idx | 4095) + 1;
//     for (; *start_idx < end_idx; *start_idx++)
//         m = max(m, it->samples[sample_number]
// }


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(800, 600, WT_WINDOWED, "Sound Shovel");
    TextRenderer *font = CreateTextRenderer("Lucida Console", 10, 4);

    SoundBlock *first = new SoundBlock;
    SoundBlock *current = first;
    unsigned num_blocks = 10;
    for (int i = 0; i < num_blocks; i++)
    {
        current->m_next = new SoundBlock;      
        current = current->m_next;
    }
    current->m_len = SoundBlock::MAX_SAMPLES;

    unsigned win_width = g_window->bmp->width;
    unsigned win_height = g_window->bmp->height;
    unsigned total_len = num_blocks * SoundBlock::MAX_SAMPLES;
    double h_zoom_ratio = (double)total_len / (double)win_width;
    double v_zoom_ratio = (double)win_height / 1e5;

    while (!g_window->windowClosed && !g_inputManager.keyDowns[KEY_ESC])
    {
        ClearBitmap(g_window->bmp, g_colourBlack);
        InputManagerAdvance();

        SoundIterator it(first, 0);
        int y_mid = win_height / 2;
        for (int x = 0; x < win_width; x++)
        {
            unsigned start_idx = x * h_zoom_ratio;
            unsigned end_idx = (x + 1) * h_zoom_ratio - 1;
            int num_samples_this_pixel = end_idx - start_idx + 1;
            for (int i = 0; i < num_samples_this_pixel; i++)
            {
                int16_t sample = it.GetNextSample();
                PutPix(g_window->bmp, x, y_mid + sample * v_zoom_ratio, Colour(40, 40, 255));
            }
        }

        HLine(g_window->bmp, 0, win_height / 2, win_width, Colour(255, 255, 255, 60));

        // Draw frames per second counter
        DrawTextRight(font, g_colourWhite, g_window->bmp, g_window->bmp->width - 5, 0, "FPS:%i", g_window->fps);

        UpdateWin();
        SleepMillisec(10);
    }

    return 0;
}
