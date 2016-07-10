#include "tooltip_manager.h"

#include "drawing_primitives.h"
#include "gui_manager.h"
#include "menu.h"

#include "df_hi_res_time.h"
#include "df_input.h"
#include "df_text_renderer.h"
#include "df_window_manager.h"

#include <stdarg.h>
#include <stdio.h>


TooltipManager g_tooltipManager;


void TooltipManager::Advance()
{	
// 	m_h = g_propFont.GetTextHeight();
// 	int mx = MOUSE_X;
// 	int my = MOUSE_Y;
// 	if (mx < m_x || mx > (m_x + m_w) || my < m_y || my > (m_y+m_h))
// 		m_buf[0] = '\0';
	if (abs(g_inputManager.mouseVelX) > 1 || abs(g_inputManager.mouseVelY) > 1)
		m_showTime = GetHighResTime() + 1.0f;
}


void TooltipManager::Render()
{
    Widget *menu = g_guiManager->GetWidgetByName(MENU_BAR_NAME);
    if (g_guiManager->m_focussedWidget == menu)
        return;

	if (m_buf[0] != '\0')
	{
		float now = GetHighResTime();
		if (now > m_showTime)
		{
			int h = g_defaultTextRenderer->charHeight;
			int w = GetTextWidth(g_defaultTextRenderer, m_buf);
			RectFill(g_window->bmp, m_x, m_y, w, h, g_guiManager->m_backgroundHighlightColour);
			DrawOutlineBox(m_x-1, m_y-1, w+2, h+2, g_guiManager->m_frameColour4);
            DrawTextSimple(g_defaultTextRenderer, m_col, g_window->bmp, m_x, m_y, m_buf);
		}
	}
}


void TooltipManager::ShowColouredString(int x, int y, int w, int h, RGBAColour &col, char const *fmt, ...)
{
	int mx = g_inputManager.mouseX;
	int my = g_inputManager.mouseY;
	if (mx < x || mx > (x + w) || my < y || my > (y+h))
		return;

    va_list ap;
    va_start (ap, fmt);
    _vsnprintf(m_buf, sizeof(m_buf), fmt, ap);

	m_x = x;
	m_y = y;
	m_col = col;
}
