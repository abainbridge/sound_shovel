// Own header
#include "container_vert.h"

// Project headers
#include "andy_string.h"
#include "gui/cursor_manager.h"
#include "gui/gui_manager_base.h"
#include "gui/menu.h"
#include "gui/widget_history.h"

// Contrib headers
#include "df_input.h"
#include "df_window_manager.h"

// Standard headers
#include <stdlib.h>


ContainerVert::ContainerVert(char const *name, Widget *parent, int x, int y)
:   Container(name, parent, x, y),
    m_resizingWidget(NULL)
{
}


void ContainerVert::AddWidget(Widget *w)
{
    m_widgets.PutData(w);

    String key = (String)w->m_name + "Height";
    int defaultHeight = w->m_height;
    w->m_height = g_widgetHistory->GetInt(key.c_str(), defaultHeight);
}


void ContainerVert::RemoveWidget(char const *name)
{
    for (int i = 0; i < m_widgets.Size(); ++i)
    {
        if (stricmp(name, m_widgets[i]->m_name) == 0)
        {
            m_widgets.RemoveData(i);
            return;
        }
    }
}


// This function should be called every frame. It notices if the mouse is over
// any of the resize handles and updates the mouse cursor accordingly.
void ContainerVert::AdvanceResizing()
{
    bool setRectNeeded = false;
    int y = m_top;

    for (int i = 0; i < m_widgets.Size() - 1; ++i)
    {
        Widget *w = m_widgets[i];
        Widget *nextW = m_widgets[i + 1];
        y += m_widgets[i]->m_height;
	    
	    if (!m_resizingWidget &&
            (g_inputManager.mouseY >= (y - 0)) && 
			(g_inputManager.mouseY <= (y + WIDGET_SPACER + 1)) &&
			w->m_growable && nextW->m_growable && nextW->m_hideState == HideStateShown)
	    {
		    g_guiManager->m_cursorManager.RequestCursorType(CursorManager::CursorVertDrag);
            if (g_inputManager.lmbClicked)
		    {
                m_resizingWidget = w;
		    }
        }
         
        if (g_inputManager.lmb && m_resizingWidget == w && g_inputManager.mouseVelY != 0)
		{
			Widget *w1 = m_widgets[i];
			Widget *w2 = m_widgets[i + 1];

            int newH1 = g_inputManager.mouseY - w1->m_top;
			int newH2 = (w2->m_top + w2->m_height) - g_inputManager.mouseY;

			if (newH1 < 50)
			{
                int delta = 50 - newH1;
				newH1 = 50;
                newH2 -= delta;
			}
			else if (newH2 < 50)
			{
                int delta = 50 - newH2;
				newH2 = 50;
                newH1 -= delta;
			}

            w1->m_height = newH1;
			w2->m_height = newH2;
			setRectNeeded = true;
			String key1 = (String)w1->m_name + "Height";
			g_widgetHistory->SetInt(key1.c_str(), w1->m_height);
			String key2 = (String)w2->m_name + "Height";
			g_widgetHistory->SetInt(key2.c_str(), w2->m_height);
		}

        if (g_inputManager.lmbUnClicked)
	    {
			m_resizingWidget = NULL;
	    }

        y += WIDGET_SPACER;
    }

    if (m_resizingWidget)
        g_guiManager->m_cursorManager.RequestCursorType(CursorManager::CursorVertDrag);

    if (setRectNeeded)
    {
        SetRect(m_left, m_top, m_width, m_height);
    }
}


void ContainerVert::Advance()
{
    if (IsPointInBounds(g_inputManager.mouseX, g_inputManager.mouseY))
    {
        AdvanceResizing();
    }

	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		Widget *w = m_widgets[i];
        if (w->m_hideState != Widget::HideStateHidden)
        {
            w->Advance();
        }
	}
}


void ContainerVert::Render()
{
    // Render everything other than the menu bar
	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		Widget *w = m_widgets[i];
		if (w->m_hideState == HideStateHidden) continue;
		if (stricmp(w->m_name, MENU_BAR_NAME) != 0)
		{
			g_guiManager->DrawFrame(w->m_left, w->m_top, w->m_width, w->m_height);
//			ClipRect cr(w->m_left, w->m_top, w->m_width, w->m_height); // TODO implement cliprect
			w->Render();
		}
	}
}


void ContainerVert::SetRect(int x, int y, int w, int h)
{
	if (x != -1)
	{
		Widget::SetRect(x, y, w, h);
	}

	// Calculate the total height requested by growable widgets and also
	// the total height reserved for non-growables and widget spacers.
	int heightRemaining = m_height + WIDGET_SPACER;
	int totalHeightRequested = 0;

	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		if (m_widgets[i]->m_hideState == HideStateHidden) continue;

		heightRemaining -= WIDGET_SPACER;
		if (m_widgets[i]->m_growable && m_widgets[i]->m_hideState != HideStateNewlyShown)
		{
			totalHeightRequested += m_widgets[i]->m_height;
		}
		else
		{
			heightRemaining -= m_widgets[i]->m_height;
		}
	}

	// Now assign all the widgets their new size and pos.
	int y1 = m_top;
	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		Widget *widget = m_widgets[i];
		if (widget->m_hideState != HideStateHidden)
		{
			if (widget->m_growable && widget->m_hideState != HideStateNewlyShown)
			{
				float fractionOfHeightRemaining = (float)widget->m_height / (float)totalHeightRequested;
				int actualHeight = (float)heightRemaining * fractionOfHeightRemaining + 0.5f;
				widget->SetRect(m_left, y1, m_width, actualHeight);
				y1 += actualHeight + WIDGET_SPACER;
			}
			else
			{
				widget->SetRect(m_left, y1, m_width, widget->m_height);
				y1 += widget->m_height + WIDGET_SPACER;
			}
			widget->m_hideState = HideStateShown;
		}
    }
}
