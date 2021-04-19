// Own header
#include "container_hori.h"

// Project headers
#include "andy_string.h"
#include "mouse_cursor.h"
#include "gui_base.h"
#include "widget_history.h"

// Contrib headers
#include "df_window.h"


ContainerHori::ContainerHori(char const *name, Widget *parent)
:   Container(name, parent),
    m_resizingWidget(NULL)
{
}


void ContainerHori::AddWidget(Widget *w)
{
    m_widgets.PutData(w);
    String key = (String)w->m_name + "Width";
//    int defaultWidth = 500 / (m_widgets.Size() + 1);
//    w->m_width = g_widgetHistory->GetInt(key.c_str(), defaultWidth);
}


void ContainerHori::AdvanceResizing()
{
    bool setRectNeeded = false;
    int x = m_left;

    for (int i = 0; i < m_widgets.Size() - 1; ++i)
    {
        Widget *w = m_widgets[i];
        Widget *nextW = m_widgets[i + 1];
        x += m_widgets[i]->m_width;
	    
	    if (!m_resizingWidget &&
            (g_input.mouseX >= (x - 0)) && 
			(g_input.mouseX <= (x + WIDGET_SPACER + 1)) &&
			w->m_growable && nextW->m_growable && nextW->m_hideState == HideStateShown)
	    {
		    g_gui->m_mouseCursor.RequestCursorType(MouseCursor::CursorDragHori);
            if (g_input.lmbClicked)
		    {
                m_resizingWidget = w;
		    }
        }
         
        if (g_input.lmb && m_resizingWidget == w && g_input.mouseVelX != 0)
		{
			Widget *w1 = m_widgets[i];
			Widget *w2 = m_widgets[i + 1];

            int newW1 = g_input.mouseX - w1->m_left;
			int newW2 = (w2->m_left + w2->m_width) - g_input.mouseX;

			if (newW1 < 30)
			{
                int delta = 30 - newW1;
				newW1 = 30;
                newW2 -= delta;
			}
			else if (newW2 < 30)
			{
                int delta = 30 - newW2;
				newW2 = 30;
                newW1 -= delta;
			}

            w1->m_width = newW1;
			w2->m_width = newW2;
			setRectNeeded = true;
			String key1 = (String)w1->m_name + "Width";
			g_widgetHistory->SetInt(key1.c_str(), w1->m_width);
			String key2 = (String)w2->m_name + "Width";
			g_widgetHistory->SetInt(key2.c_str(), w2->m_width);
		}

        if (g_input.lmbUnClicked)
	    {
			m_resizingWidget = NULL;
	    }

        x += WIDGET_SPACER;
    }

    if (m_resizingWidget)
        g_gui->m_mouseCursor.RequestCursorType(MouseCursor::CursorDragHori);

    if (setRectNeeded)
    {
        SetRect(m_left, m_top, m_width, m_height);
    }
}


void ContainerHori::Advance()
{
    if (IsMouseInBounds())
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


void ContainerHori::Render()
{
	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		Widget *w = m_widgets[i];		
		g_gui->DrawFrame(w->m_left, w->m_top, w->m_width, w->m_height);
//		ClipRect cr(w->m_left, w->m_top, w->m_width, w->m_height); // TODO implement cliprect
		m_widgets[i]->Render();
	}
}


void ContainerHori::SetRect(int x, int y, int w, int h)
{
	if (x != -1)
	{
		Widget::SetRect(x, y, w, h);
	}

	// Calculate the total width requested by growable widgets and also
	// the total width reserved for non-growables and widget spacers.
	int widthRemaining = m_width + WIDGET_SPACER;
	int totalwidthRequested = 0;

	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		if (m_widgets[i]->m_hideState == HideStateHidden) continue;

		widthRemaining -= WIDGET_SPACER;
		if (m_widgets[i]->m_growable && m_widgets[i]->m_hideState != HideStateNewlyShown)
		{
			totalwidthRequested += m_widgets[i]->m_width;
		}
		else
		{
			widthRemaining -= m_widgets[i]->m_width;
		}
	}


    while (widthRemaining < 0)
    {
        for (int i = 0; i < m_widgets.Size(); ++i)
        {
            Widget *widget = m_widgets[i];
            if (widget->m_hideState != HideStateHidden && widget->m_growable)
            {
                widget->m_width--;
                widthRemaining++;
            }
        }
    }

	// Now assign all the widgets their new size and pos.
	int x1 = m_left;
	for (int i = 0; i < m_widgets.Size(); ++i)
	{
		Widget *widget = m_widgets[i];
		if (widget->m_hideState != HideStateHidden)
		{
			if (widget->m_growable && widget->m_hideState != HideStateNewlyShown)
			{
				float fractionOfwidthRemaining = (float)widget->m_width / (float)totalwidthRequested;
				int actualwidth = (float)widthRemaining * fractionOfwidthRemaining + 0.5f;
				widget->SetRect(x1, m_top, actualwidth, m_height);
				x1 += actualwidth + WIDGET_SPACER;
			}
			else
			{
				widget->SetRect(x1, m_top, widget->m_width, m_height);
				x1 += widget->m_width + WIDGET_SPACER;
			}

			widget->m_hideState = HideStateShown;
		}
    }
}
