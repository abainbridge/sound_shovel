// Own header
#include "widget.h"

// Project headers
#include "drawing_primitives.h"
#include "gui_base.h"
#include "widget_history.h"
#include "andy_string.h"
#include "string_utils.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_window.h"


Widget::Widget(char const *name, Widget *parent, int w, int h)
:	CommandReceiver(name),
	m_left(0),
	m_top(0),
    m_width(w),
    m_height(h),
	m_growable(true),
	m_highlightable(true),
    m_ghosted(false),
    m_parent(parent)
{
	m_name = StringDuplicate(name);

    String key = (String)m_name + "Hidden";
    bool hide = g_widgetHistory->GetInt(key.c_str(), false);
    if (hide)
	{
		m_hideState = HideStateHidden;
	}
	else
	{
		m_hideState = HideStateShown;
	}
}


Widget::~Widget()
{
    delete [] m_name;
}


void Widget::SetRect(int x, int y, int w, int h)
{
	m_left = x;
	m_top = y;
	m_width = w;
	m_height = h;
}


bool Widget::IsMouseInBounds()
{
    if (m_ghosted || m_hideState == HideStateHidden)
        return false;

    int x = g_input.mouseX;
    int y = g_input.mouseY;
	if (x >= m_left && x < m_left + m_width &&
		y >= m_top && y < m_top + m_height)
	{
		return true;
	}

	return false;
}


Widget *Widget::GetWidgetAtPos(int x, int y)
{
	if (m_hideState != HideStateHidden)
	{
        if (x >= m_left && x < m_left + m_width &&
            y >= m_top && y < m_top + m_height)
        {
            return this;
        }
	}

	return NULL;
}


Widget *Widget::GetWidgetByName(char const *name)
{
    if (stricmp(name, m_name) == 0) return this;
    return NULL;
}


Widget *Widget::GetLeftWidget(Widget *caller) 
{
    return m_parent->GetLeftWidget(this); 
}


Widget *Widget::GetRightWidget(Widget *caller) 
{ 
    return m_parent->GetRightWidget(this); 
}


void Widget::Advance()
{
}


void Widget::ToggleHide()
{
	switch (m_hideState)
	{
	case HideStateHidden:
        g_gui->Show(m_name);
		break;
	case HideStateShown:
		Hide(); // TODO: make this call g_guiManager->Hide, like Show does
		break;
	case HideStateNewlyShown:
		break;
	}
}


void Widget::Hide()
{
    String key = (String)m_name + "Hidden";
    g_widgetHistory->SetInt(key.c_str(), true);
    m_hideState = HideStateHidden;
}


void Widget::Show(char const *name)
{
	DebugAssert(stricmp(name, m_name) == 0);

    if (m_hideState == HideStateHidden)
    {
        String key = (String)m_name + "Hidden";
        g_widgetHistory->SetInt(key.c_str(), false);
        m_hideState = HideStateNewlyShown;
    }
}


char *Widget::ExecuteCommand(char const *object, char const *command, char const *arguments)
{ 
	if (COMMAND_IS("toggleHide"))
	{
		ToggleHide();
	}
	else
	{
		return NULL;
	}
	
	return COMMAND_RETURN_NOTHING;
}


void Widget::DrawFilledBoxHoriGrad(int x, int y, int w, int h)
{
	DfColour const &col1 = g_gui->m_frameColour1;
	DfColour const &col3 = g_gui->m_frameColour3;
	DfColour const &col5 = g_gui->m_frameColour5;
	DfColour const &col6 = col5 - Colour(30,30,30,0);
	
	DrawOutlineBox(x, y, w, h, col6);
	
	HLine(g_window->bmp, x + 1, y + 1, w - 2, col1);
	HLine(g_window->bmp, x + 1, y + h - 2, w - 2, col3);
	
	y += 2;
	h -= 4;
	
	float deltaR = (col3.r - col1.r) / (float)w;
	float deltaG = (col3.g - col1.g) / (float)w;
	float deltaB = (col3.b - col1.b) / (float)w;
	
	for (int i = 1; i < w-1; i++)
	{
		DfColour col = RgbaAddWithSaturate(col1, Colour(deltaR * i, deltaG * i, deltaB * i));
		VLine(g_window->bmp, x + i, y, h, col);
	}
}


void Widget::DrawFilledBoxVertGrad(int x, int y, int w, int h)
{
	DfColour const &col1 = g_gui->m_frameColour1;
	DfColour const &col3 = g_gui->m_frameColour3;
	DfColour const &col5 = g_gui->m_frameColour5;
	DfColour const &col6 = col5 - Colour(30,30,30,0);
	
	DrawOutlineBox(x, y, w, h, col6);
	
	VLine(g_window->bmp, x + 1,     y + 1, h - 2, col1);
	VLine(g_window->bmp, x + w - 2, y + 1, h - 2, col3);
	
	x += 2;
	w -= 4;
	
	float deltaR = (col3.r - col1.r) / (float)h;
	float deltaG = (col3.g - col1.g) / (float)h;
	float deltaB = (col3.b - col1.b) / (float)h;
	
	for (int i = 1; i < h-1; i++)
	{
		DfColour col = RgbaAddWithSaturate(col1, Colour(deltaR * i, deltaG * i, deltaB * i));
		HLine(g_window->bmp, x, y + i, w, col);
	}
}
