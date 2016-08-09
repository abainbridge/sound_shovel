// Own header
#include "main.h"

// Project headers
#include "gui/app_gui_manager.h"
#include "sound_system.h"

// Contrib headers
#include "gui/file_dialog.h"
#include "gui/widget_history.h"
#include "df_bitmap.h"
#include "df_time.h"
#include "df_input.h"
#include "df_font.h"
#include "df_window.h"

// Platform headers
#define NOMINMAX
#include <windows.h>

// Standard headers
#include <algorithm>
#include <stdint.h>


bool g_canSleep = true;


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(1000, 600, WT_WINDOWED, APPLICATION_NAME);
    g_defaultFont = DfCreateFont("Lucida Console", 10, 4);

    g_soundSystem = new SoundSystem;

    g_widgetHistory = new WidgetHistory("widget_history.txt");  // TODO - re-introduce the system_info module and make this filename be in the user's home folder.
    g_guiManager = new AppGuiManager;
    g_guiManager->Initialise();

    while (!g_guiManager->m_exitAtEndOfFrame)
    {
        ClearBitmap(g_window->bmp, Colour(44, 51, 59));
        InputManagerAdvance();

        g_guiManager->Advance();
        g_guiManager->Render();

        g_soundSystem->Advance();
        UpdateWin();

//         if (g_canSleep)
//             SleepMillisec(50);

        g_canSleep = true;
    }

    return 0;
}
