#include "df_bitmap.h"
#include "df_hi_res_time.h"
#include "df_input.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))


struct SoundBlock
{
    enum { MAX_SAMPLES = 131072 };
    enum { SAMPLES_PER_LUT_ITEM = 1024 };
    enum { LUT_SIZE = MAX_SAMPLES / SAMPLES_PER_LUT_ITEM };

    SoundBlock *m_next;
    int16_t     m_samples[MAX_SAMPLES];
    unsigned    m_len;   // Number of valid items in m_samples
    int16_t     m_max_lut[LUT_SIZE];
    int16_t     m_min_lut[LUT_SIZE];

    SoundBlock()
    {
        m_next = NULL;
        m_len = MAX_SAMPLES;

        for (unsigned i = 0; i < m_len; i++)
            m_samples[i] = i - 32768;

        RecalcLuts();
    }

    void RecalcLuts()
    {
        int16_t *current_sample = m_samples;
        for (unsigned i = 0; i < LUT_SIZE; i++)
        {
            int16_t _min = INT16_MAX;
            int16_t _max = INT16_MIN;
            for (unsigned j = 0; j < SAMPLES_PER_LUT_ITEM; j++)
            {
                _min = MIN(*current_sample, _min);
                _max = MAX(*current_sample, _max);
                current_sample++;
            }

            m_max_lut[i] = _max;
            m_min_lut[i] = _min;
        }
    }
};


struct SoundPos
{
    SoundBlock *m_block;
    unsigned m_current_idx;

    SoundPos(SoundBlock *block, unsigned current_idx)
    {
        m_block = block;
        m_current_idx = current_idx;
    }
};


void CalcMinMaxForRange(SoundPos *pos, unsigned num_samples, int16_t *result_min, int16_t *result_max)
{
    SoundBlock *block = pos->m_block;
    unsigned idx = pos->m_current_idx;

    int16_t _min = INT16_MAX;
    int16_t _max = INT16_MIN;

    const unsigned lut_item_boundary_mask = SoundBlock::SAMPLES_PER_LUT_ITEM - 1;

    // For each block we need to consider...
    while (block && num_samples)
    {
        // Do the slow bit at the start of the range.
        {
            unsigned num_samples_to_next_lut_item_boundary = (SoundBlock::SAMPLES_PER_LUT_ITEM - idx) & lut_item_boundary_mask;
            unsigned num_slow_samples = num_samples_to_next_lut_item_boundary;
            if (num_slow_samples > num_samples)
                num_slow_samples = num_samples;

            unsigned end_idx = idx + num_slow_samples;
            while (idx < end_idx)
            {
                _min = MIN(block->m_samples[idx], _min);
                _max = MAX(block->m_samples[idx], _max);
                idx++;
            }

            num_samples -= num_slow_samples;
        }

        if (num_samples == 0)
            break;

        if (idx == block->m_len)
        {
            block = block->m_next;
            idx = 0;
            continue;
        }

        // Do the fast bit
        {
            // Cases here:
            // 1. There is one or more LUT items in this block that we can use.
            // 2. There is one or more LUT items left, but the pixel doesn't span entirely over it/them.

            unsigned num_lut_items_we_need = num_samples / SoundBlock::SAMPLES_PER_LUT_ITEM;
            unsigned current_lut_item_idx = idx / SoundBlock::SAMPLES_PER_LUT_ITEM;
            unsigned num_lut_items_in_this_block = ((block->m_len - 1) / SoundBlock::SAMPLES_PER_LUT_ITEM) + 1;
            unsigned num_lut_items_left_in_this_block = num_lut_items_in_this_block - current_lut_item_idx;

            unsigned num_lut_items_to_use = num_lut_items_we_need;
            if (num_lut_items_left_in_this_block < num_lut_items_to_use)
                num_lut_items_to_use = num_lut_items_left_in_this_block;

            unsigned end_lut_item_idx = current_lut_item_idx + num_lut_items_to_use;
            while (current_lut_item_idx < end_lut_item_idx)
            {
                _min = MIN(block->m_min_lut[current_lut_item_idx], _min);
                _max = MAX(block->m_max_lut[current_lut_item_idx], _max);
                current_lut_item_idx++;
            }

            unsigned num_samples_left_in_this_block_before_we_did_the_fast_bit = block->m_len - idx;
            if (num_samples > num_samples_left_in_this_block_before_we_did_the_fast_bit)
                num_samples -= num_samples_left_in_this_block_before_we_did_the_fast_bit;
            else
                num_samples -= num_lut_items_to_use * SoundBlock::SAMPLES_PER_LUT_ITEM;

            idx += num_lut_items_to_use * SoundBlock::SAMPLES_PER_LUT_ITEM;
        }

        if (idx >= block->m_len)
        {
            block = block->m_next;
            continue;
        }

        // Do the slow bit at the end of the range.
        {
            // Cases here:
            // 1. Not at end of block and num_samples less than next LUT item boundary.

            unsigned end_idx = idx + num_samples;
            while (idx < end_idx)
            {
                _min = MIN(block->m_samples[idx], _min);
                _max = MAX(block->m_samples[idx], _max);
                idx++;
            }

            break;
        }
    }

    pos->m_block = block;
    pos->m_current_idx = idx;

    *result_min = _min;
    *result_max = _max;
}


void CalcDisplayData(int16_t *mins, int16_t *maxes, unsigned width_in_pixels, SoundBlock *first_block, unsigned samples_per_pixel)
{
    double start_time = GetHighResTime();
    SoundPos pos(first_block, 0);

    for (unsigned x = 0; x < width_in_pixels; x++)
        CalcMinMaxForRange(&pos, samples_per_pixel, mins + x, maxes + x);

    double end_time = GetHighResTime();
    DebugOut("Time taken: %.3f\n", end_time - start_time);
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(800, 600, WT_WINDOWED, "Sound Shovel");
    TextRenderer *font = CreateTextRenderer("Lucida Console", 10, 4);

    SoundBlock *first = new SoundBlock;
    SoundBlock *current = first;
    unsigned num_blocks = 80;
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
    int y_mid = win_height / 2;

    int16_t *display_mins = new int16_t[win_width];
    int16_t *display_maxes = new int16_t[win_width];

    CalcDisplayData(display_mins, display_maxes, win_width, first, h_zoom_ratio);

    RGBAColour sound_colour = Colour(40, 40, 255);

    while (!g_window->windowClosed && !g_inputManager.keyDowns[KEY_ESC])
    {
        ClearBitmap(g_window->bmp, g_colourBlack);
        InputManagerAdvance();

//         SoundIterator it(first, 0);
//         for (unsigned x = 0; x < win_width; x++)
//         {
//             unsigned start_idx = x * h_zoom_ratio;
//             unsigned end_idx = (x + 1) * h_zoom_ratio - 1;
//             int num_samples_this_pixel = end_idx - start_idx + 1;
//             for (int i = 0; i < num_samples_this_pixel; i++)
//             {
//                 int16_t sample = it.GetNextSample();
//                 PutPix(g_window->bmp, x, y_mid + sample * v_zoom_ratio, sound_colour);
//             }
//         }
        for (unsigned x = 0; x < win_width; x++)
        {
            int vline_len = (display_maxes[x] - display_mins[x]) * v_zoom_ratio;
            VLine(g_window->bmp, x, y_mid - display_maxes[x] * v_zoom_ratio, vline_len, sound_colour);
        }

        HLine(g_window->bmp, 0, win_height / 2, win_width, Colour(255, 255, 255, 60));

        // Draw frames per second counter
        DrawTextRight(font, g_colourWhite, g_window->bmp, g_window->bmp->width - 5, 0, "FPS:%i", g_window->fps);

        UpdateWin();
        SleepMillisec(10);
    }

    return 0;
}
