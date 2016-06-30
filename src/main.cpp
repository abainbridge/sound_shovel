// Own header
#include "main.h"

// Project headers
#include "sound.h"
#include "sound_view.h"

//
#include "gui/file_dialog.h"
#include "sound/sound_system.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_hi_res_time.h"
#include "df_input.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

// Platform headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// Standard headers
#include <algorithm>
#include <stdint.h>


bool g_can_sleep = true;


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(1000, 600, WT_WINDOWED, "Sound Shovel");
    g_defaultTextRenderer = CreateTextRenderer("Lucida Console", 10, 4);

    g_soundSystem = new SoundSystem;

//     DArray<String> files = FileDialogOpen();
//     if (files.Size() == 0)
//         return -1;

    Sound sound;
//    sound.LoadWav(files[0].c_str());
    sound.LoadWav("C:/users/andy/desktop/andante.wav");
    g_soundSystem->PlaySound(&sound, 0);

    SoundView sound_view(&sound);

    while (!g_window->windowClosed && !g_inputManager.keyDowns[KEY_ESC])
    {
        ClearBitmap(g_window->bmp, Colour(50, 50, 50));
        InputManagerAdvance();

        sound_view.Advance();
        sound_view.Render(g_window->bmp);
        
        DrawTextRight(g_defaultTextRenderer, g_colourWhite, g_window->bmp, 
            g_window->bmp->width - 5, 2, "FPS: %d", g_window->fps);

        g_soundSystem->Advance();
        UpdateWin();
        
        if (g_can_sleep)
            SleepMillisec(50);

        g_can_sleep = true;
    }

    return 0;
}
