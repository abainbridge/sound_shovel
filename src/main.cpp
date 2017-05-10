// Own header
#include "main.h"

// Project headers
#include "gui/app_gui.h"
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


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	CreateWin(1000, 600, WT_WINDOWED, APPLICATION_NAME);
    g_defaultFont = DfCreateFont("Lucida Console", 10, 4);

    g_soundSystem = new SoundSystem;

    g_widgetHistory = new WidgetHistory("widget_history.txt");  // TODO - re-introduce the system_info module and make this filename be in the user's home folder.
    g_gui = new AppGui;
    g_gui->Initialise();

    while (1)
    {
        g_gui->m_canSleep = true;
        for (int i = 0; i < 500 && g_gui->m_canSleep; i++)
        {
            DfSleepMillisec(1);

            InputManagerAdvance();
            g_gui->Advance();

            if (g_gui->m_exitAtEndOfFrame)
                return 0;

            g_soundSystem->Advance();
        }

        ClearBitmap(g_window->bmp, Colour(44, 51, 59));
        g_gui->Render();

        UpdateWin();
    }

    return 0;
}
