// Own header
#include "mouse_cursor.h"

// Contrib headers
#include "df_window.h"

// Platform headers
#include <windows.h>

// Standard headers
#include <stdlib.h>


MouseCursor::MouseCursor()
:	m_captured(NULL),
	m_type(CursorMain)
{
}


void MouseCursor::Advance()
{
	if (g_input.lmbUnClicked) 
		m_captured = NULL;
	if (m_captured == NULL)
		m_type = CursorMain;
}


void MouseCursor::RequestCursorType(int type)
{
	if (!m_captured && type != m_type)
		m_type = type;
}


void MouseCursor::Render(int x, int y)
{
    static char *CURSOR_NAMES[] = { IDC_ARROW, IDC_IBEAM, IDC_SIZEWE, IDC_SIZENS, IDC_SIZEALL };
    SetCursor(LoadCursor(NULL, CURSOR_NAMES[m_type]));
}
