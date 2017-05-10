// Own header
#include "gui_base.h"

// Project headers
#include "preferences.h"
#include "threading.h"
#include "gui/container_hori.h"
#include "gui/container_vert.h"
#include "gui/drawing_primitives.h"
#include "gui/keyboard_shortcuts.h"
#include "gui/menu.h"
#include "gui/status_bar.h"
#include "gui/tooltip_manager.h"
#include "gui/widget_history.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_colour.h"
#include "df_input.h"
#include "df_message_dialog.h"
#include "df_font.h"
#include "df_window.h"


GuiBase *g_gui = NULL;


// ****************************************************************************
//  Class GuiBase
// ****************************************************************************

void MouseUpdateHandler(int x, int y)
{
    if (g_gui)
    {
        g_gui->m_mouseCursor.Render(x, y);
    }
}


GuiBase::GuiBase()
:   m_modalWidget(NULL),
    m_previouslyFocussedWidget(NULL),
    m_focussedWidget(NULL),
    m_mainContainer(NULL),
    m_exitRequested(false),
    m_exitAtEndOfFrame(false),
    m_canSleep(true),
    Widget(GUI_MANAGER_NAME, NULL)
{
    g_keyboardShortcutManager = new KeyboardShortcutManager("data/config_keys.txt");

    SetColours();
    m_propFont = DfCreateFont("Tahoma", 8, 5);
    ReleaseAssert((int)m_propFont, "Couldn't load font 'Tahoma'");

    m_aboutString = "";
    m_highlightFocussedWidget = false;

    g_commandSender.RegisterReceiver(this);

    // Register the mouse update handler function with the input manager
    //    g_input.RegisterMouseUpdateCallback(&MouseUpdateHandler);  // TODO - Implement RegisterMouseUpdateCallback
}


DfColour GuiBase::GetColour(char const *name, DfColour const &defaultC)
{
    DfColour rv = defaultC;

    char const *str = NULL;// g_prefs->GetString(name); // TODO - re-enable the prefs system

    if (str)
        StringToColour(str, &rv);

    return rv;
}


void GuiBase::SetColours()
{
    m_backgroundColour = GetColour("GuiWindowBackgroundColour", Colour(50,50,50));

    m_frameColour2 = GetColour("GuiFrameColour", Colour(83, 83, 83));
    m_textColourNormal = GetColour("GuiTextColourNormal", Colour(200,200,200));
    m_selectionBlockColour = GetColour("GuiSelectionBlockColour", Colour(60, 60, 155, 64));
    m_highlightColour = GetColour("GuiHighlightColour", m_backgroundColour + Colour(15,15,15));
    m_strongHighlightColour = GetColour("GuiStrongHighlightColour", m_backgroundColour + Colour(35,35,35));
    m_focusBoxColour = GetColour("GuiFocusBoxColour", Colour(255,120,120));

    m_selectionBlockUnfocussedColour = m_selectionBlockColour;
    m_selectionBlockUnfocussedColour = m_selectionBlockUnfocussedColour * 0.5f + m_backgroundColour * 0.5f;

    m_frameColour1 = m_frameColour2 * 0.4f;
    m_frameColour3 = m_frameColour2 * 1.2f;
    m_frameColour4 = m_frameColour2 * 1.6f;
    m_frameColour5 = m_frameColour2 * 2.0f;

    m_textColourFrame = GetColour("GuiTextColourFrame", Colour(240,240,240));
}


unsigned long __stdcall AboutProc(void *data)
{
    MessageDialog("About", g_gui->m_aboutString, MsgDlgTypeOk);

    return 0;
}


void GuiBase::About()
{
    StartThread(AboutProc, NULL);
}


bool GuiBase::StringToColour(char const *str, DfColour *col)
{
    col->r = atoi(str);
    str = strchr(str, ',');
    if (!str) return false;
    str++;
    col->g = atoi(str);
    str = strchr(str, ',');
    if (!str) return false;
    str++;
    col->b = atoi(str);

    return true;
}


void GuiBase::FillBackground(int x, int y, int w, int h, bool highlighted) const
{
    DrawOutlineBox(x, y, w, h, m_frameColour5);
    x++; y++;
    w -= 2; h -= 2;
//    RectFill(g_window->bmp, x, y, w, h, m_backgroundColour);
}


void GuiBase::DrawFrame(int x, int y, int w, int h) const
{
    RectFill(g_window->bmp, x-3, y-3, w+8, 3, m_frameColour2);
    RectFill(g_window->bmp, x-3, y+h, w+8, 4, m_frameColour2);
    RectFill(g_window->bmp, x-3, y, 4, h, m_frameColour2);
    RectFill(g_window->bmp, x+w, y, 4, h, m_frameColour2);
}


char *GuiBase::ExecuteCommand(char const *object, char const *command, char const *arguments)
{
    if (COMMAND_IS("About"))               About();
    else if (COMMAND_IS("Exit"))           RequestExit();

    return NULL;
}


void GuiBase::SetRect(int x, int y, int w, int h)
{
    if (x != m_left || y != m_top || m_width != w || m_height != h)
    {
        g_widgetHistory->SetInt("MainWindowWidth", w);
        g_widgetHistory->SetInt("MainWindowHeight", h);
        g_widgetHistory->SetInt("MainWindowLeft", x);
        g_widgetHistory->SetInt("MainWindowTop", y);
        m_width = w;
        m_height = h;
        m_left = x;
        m_top = y;
    }

    m_mainContainer->SetRect(0 + WIDGET_SPACER, 0 + WIDGET_SPACER,
                             w - (WIDGET_SPACER*2), h - (WIDGET_SPACER*2));
}


Widget *GuiBase::GetWidgetAtPos(int x, int y)
{
    return m_mainContainer->GetWidgetAtPos(x, y);
}


Widget *GuiBase::GetWidgetByName(char const *name)
{
    // Special case when looking for "CurrentDoc"
    if (m_focussedWidget)
    {
        Widget *rv = m_focussedWidget->GetWidgetByName(name);
        if (rv)
            return rv;
    }

    return m_mainContainer->GetWidgetByName(name);
}


void GuiBase::Show(char const *widgetToShow)
{
    m_mainContainer->Show(widgetToShow);
}


void GuiBase::RequestExit()
{
    m_exitRequested = true;
}


void GuiBase::SetFocussedWidget(Widget *w)
{
    m_focussedWidget = w;
    while (w != this)
    {
        w->m_hideState = HideStateShown;
        w = w->m_parent;
    }
}


void GuiBase::Advance()
{
    if (g_window->windowClosed)
    {
        g_window->windowClosed = false;
        g_gui->RequestExit();
    }

    // g_commandSender.ProcessDeferredCommands();

    // Update the size of all the widgets, unless we are minimized.
    if (g_window->bmp->width > 100)
        SetRect(0, 0, g_window->bmp->width, g_window->bmp->height);

    if (!m_modalWidget)
        g_keyboardShortcutManager->Advance();

    // Update exit request status
    m_exitAtEndOfFrame = m_exitRequested;
    m_exitRequested = false;

    // Update cursor bitmap
    m_mouseCursor.Advance();

    if (m_modalWidget)
    {
        m_modalWidget->Advance();
    }
    else
    {
        // Update which widget is highlighted
        MenuBar *menu = (MenuBar*)GetWidgetByName(MENU_BAR_NAME);
        DebugAssert(menu);
        if ((g_input.lmbClicked || g_input.rmbClicked)
            && !menu->DoesClickHitMenu())
        {
            Widget *hit = GetWidgetAtPos(g_input.mouseX, g_input.mouseY);
            if (hit && hit->m_highlightable)
            {
                m_focussedWidget = hit;
            }
        }

        // Advance our sub-widgets. Only advance the menu if it
        // is currently highlighted
        if (m_focussedWidget == menu)
        {
            menu->Advance();
        }
        else
        {
            m_mainContainer->Advance();
        }
    }

    g_tooltipManager.Advance();
}


void GuiBase::Render()
{
    const int winW = g_window->bmp->width;
    const int winH = g_window->bmp->height;
//  const int docViewManagerWidth = winW / 5;

    HLine(g_window->bmp, 0, 0, winW, m_frameColour3);
    VLine(g_window->bmp, 0, 0, winH, m_frameColour3);

    m_mainContainer->Render();

#if PROFILER_ENABLED
    m_profileWindow->Render();
#endif

    if (m_highlightFocussedWidget)
    {
        // Draw a box around the highlighted widget, unless it is the menu bar

        Widget const *hw = m_focussedWidget;

        if (g_input.windowHasFocus && hw &&
            stricmp(MENU_BAR_NAME, hw->m_name) != 0)
        {
            DrawOutlineBox(hw->m_left - 2, hw->m_top - 2,
                hw->m_width + 4, hw->m_height + 4,
                m_frameColour5);
            DrawOutlineBox(hw->m_left - 1, hw->m_top - 1,
                hw->m_width + 2, hw->m_height + 2,
                m_focusBoxColour);
        }
    }


    // Render the menu bar and any context menu
    Widget *menubar = GetWidgetByName(MENU_BAR_NAME);
    g_gui->DrawFrame(menubar->m_left, menubar->m_top, menubar->m_width, menubar->m_height-2);
    menubar->Render();

    if (m_modalWidget)
        m_modalWidget->Render();

    g_tooltipManager.Render();

    // Cursor
    g_gui->m_mouseCursor.Render(g_input.mouseX, g_input.mouseY);
}


void GuiBase::SetModalWidget(Widget *w)
{
    if (w)
    {
        m_previouslyFocussedWidget = m_focussedWidget;
        m_focussedWidget = w;
    }
    else
    {
        m_focussedWidget = m_previouslyFocussedWidget;
        m_previouslyFocussedWidget = NULL;
    }

    m_modalWidget = w;
}
