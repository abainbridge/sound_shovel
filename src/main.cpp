// Own header
#include "main.h"

// Project headers
#include "sound.h"
#include "sound_system.h"
#include "sound_view.h"

// Contrib headers
#include "gui/file_dialog.h"
#include "gui/gui_manager.h"
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


bool g_canSleep = true;


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(1000, 600, WT_WINDOWED, "Sound Shovel");
    g_defaultTextRenderer = CreateTextRenderer("Lucida Console", 10, 4);

    g_guiManager = new GuiManager;

    g_soundSystem = new SoundSystem;

//     DArray<String> files = FileDialogOpen();
//     if (files.Size() == 0)
//         return -1;

    Sound sound;
//    sound.LoadWav(files[0].c_str());
    sound.LoadWav("C:/users/andy/desktop/andante.wav");
    g_soundSystem->PlaySound(&sound, 0);

    SoundView soundView(&sound);

    while (!g_guiManager->m_exitAtEndOfFrame)
    {
        ClearBitmap(g_window->bmp, Colour(44, 51, 59));
        InputManagerAdvance();

        g_guiManager->Advance();
        g_guiManager->Render();

//         soundView.Advance();
//         soundView.Render(g_window->bmp);

        g_soundSystem->Advance();
        UpdateWin();

//         if (g_canSleep)
//             SleepMillisec(50);

        g_canSleep = true;
    }

    return 0;
}
