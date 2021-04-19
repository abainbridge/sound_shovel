// Own header
#include "status_bar.h"

// Project headers
#include "gui_base.h"

#include "../preferences.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_time.h"
#include "df_colour.h"
#include "df_font.h"
#include "df_window.h"

#include "andy_string.h"
#include "filesys_utils.h"
#include "string_utils.h"

// Standard includes
#include <stdarg.h>


StatusBar *g_statusBar = NULL;


StatusBar::StatusBar(Widget *parent)
:   Widget("StatusBar", parent)
{
	g_commandSender.RegisterReceiver(this);

	m_growable = false;
	m_highlightable = false;
	m_height = 13;

	m_messageBuffer[0] = '\0';
    m_leftBuffer[0] = '\0';
    m_rightBuffer[0] = '\0';

    m_messageStartTime = 0.0;

	g_statusBar = this;
}


void StatusBar::ShowMessage(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(m_messageBuffer, MAX_MESSAGE_LEN, fmt, ap);
	m_messageStartTime = GetRealTime();
	m_messageIsError = false;
}


void StatusBar::ShowError(char const *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _vsnprintf(m_messageBuffer, MAX_MESSAGE_LEN, fmt, ap);
	m_messageStartTime = GetRealTime();
	m_messageIsError = true;
}


void StatusBar::SetLeftString(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(m_leftBuffer, MAX_MESSAGE_LEN, fmt, ap);
}


void StatusBar::SetRightString(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(m_rightBuffer, MAX_MESSAGE_LEN, fmt, ap);
}


void StatusBar::Advance()
{
	if (GetRealTime() > (m_messageStartTime + 1.0f))
	{
		if (g_input.lmbUnClicked || g_input.mmbUnClicked || g_input.rmbUnClicked)
		{
			m_messageBuffer[0] = '\0';
		}
	}

	if (GetRealTime() > m_messageStartTime + 10.0f)
	{
		m_messageBuffer[0] = '\0';
	}
}


void StatusBar::Render()
{
	int x = m_left;
	int y = m_top;
	int w = m_width;
	int h = m_height; 

	// Render message, if there is one
	if (m_messageBuffer[0] != '\0')
	{
		if (m_messageIsError)
			RectFill(g_window->bmp, x, y, w, h, Colour(200,0,0));
		else
			RectFill(g_window->bmp, x, y, w, h, g_gui->m_frameColour3);
		DrawTextSimple(g_gui->m_propFont, g_colourWhite, g_window->bmp, x + 10, y, m_messageBuffer);
		return;
	}

	// Now the usual status bar stuff
	RectFill(g_window->bmp, x, y, w, h, g_gui->m_frameColour2);
    DrawTextSimple(g_gui->m_propFont, g_gui->m_textColourFrame, g_window->bmp, x + 5, y, m_leftBuffer);
    DrawTextRight(g_gui->m_propFont, g_gui->m_textColourFrame, g_window->bmp, x + w - 5, y, m_rightBuffer);
}


char *StatusBar::ExecuteCommand(char const *object, char const *command, char const *arguments)
{
	if (COMMAND_IS("ShowMessage"))		ShowMessage(arguments);
	else if (COMMAND_IS("ShowError"))	ShowError(arguments);
	else
		return NULL;

	return COMMAND_RETURN_NOTHING;
}
