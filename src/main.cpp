// Project headers
#include "sound.h"
#include "sound_view.h"

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
    g_defaultTextRenderer = CreateTextRenderer("Lucida Console", 10, 4);

    Sound sound;
//    sound.LoadWav("c:/Users/Andy/Desktop/2016-03-18-raspbian-jessie-lite.img");
    sound.LoadWav("c:/Users/Andy/Desktop/EightyOne V1.2/EightyOne.exe");
    //    SoundBlock *first = LoadWav("h:/video/later_apr23.mpg");

    SoundView sound_view(&sound);

    while (!g_window->windowClosed && !g_inputManager.keyDowns[KEY_ESC])
    {
        ClearBitmap(g_window->bmp, g_colourBlack);
        InputManagerAdvance();

        sound_view.Advance();
        sound_view.Render(g_window->bmp);
        
        UpdateWin();
        SleepMillisec(10);
    }

    return 0;
}
