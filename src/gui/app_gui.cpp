// Own header
#include "app_gui.h"

// Project headers
#include "main.h"
#include "sound.h"
#include "sound_widget.h"

// Contrib headers
#include "gui/container_vert.h"
#include "gui/menu.h"
#include "gui/status_bar.h"
#include "df_window.h"


AppGui::AppGui()
    : GuiBase()
{
    m_aboutString =
        "               " APPLICATION_NAME "\n\n"
        "              (Beta Version)\n"
        "               " __DATE__ "\n\n"
        " Created by Deadfrog Software  \n";
}


void AppGui::Initialise()
{
    //     DArray<String> files = FileDialogOpen();
    //     if (files.Size() == 0)
    //         return -1;

    // Create main container
    m_mainContainer = new ContainerVert("MainContainer", this);
    int SCREEN_X = 0;
    int SCREEN_Y = 0;
    int SCREEN_W = g_window->bmp->width;
    int SCREEN_H = g_window->bmp->height;
    SetRect(SCREEN_X, SCREEN_W, SCREEN_W, SCREEN_H);

    // Create MenuBar
    MenuBar *menuBar = new MenuBar(m_mainContainer);
    m_mainContainer->AddWidget(menuBar);
    menuBar->Initialise();

    SoundWidget *soundView = new SoundWidget(m_mainContainer);
    m_mainContainer->AddWidget(soundView);

    // Create StatusBar
    StatusBar *statusBar = new StatusBar(m_mainContainer);
    m_mainContainer->AddWidget(statusBar);

    SetRect(SCREEN_X, SCREEN_Y, SCREEN_W, SCREEN_H);

    SetFocussedWidget(soundView);
}


void AppGui::Advance()
{
    SoundWidget *sv = (SoundWidget*)GetWidgetByName(SOUND_VIEW_NAME);
    if (sv)
    {
        int64_t startIdx, endIdx;
        sv->GetSelectionBlock(&startIdx, &endIdx);
        g_statusBar->SetLeftString("Pos: %.0f   Selection Size: %.0f", 
            (double)startIdx, (double)endIdx - startIdx + 1);
        g_statusBar->SetRightString("Zoom: %.0f", sv->m_hZoomRatio);
    }

    GuiBase::Advance();
}