// Own header
#include "cursor_manager.h"

// Contrib headers
#include "df_input.h"
#include "df_window.h"

// Platform headers
#include <windows.h>

// Standard headers
#include <stdlib.h>


CursorManager::CursorManager()
:	m_captured(NULL),
	m_type(CursorMain)
{
}


void CursorManager::Advance()
{
	if (g_input.lmbUnClicked) 
		m_captured = NULL;
	if (m_captured == NULL)
		m_type = CursorMain;
}


void CursorManager::RequestCursorType(int type)
{
	if (!m_captured && type != m_type)
		m_type = type;
}


void CursorManager::Render(int x, int y)
{
    static char *CURSOR_NAMES[] = { IDC_ARROW, IDC_IBEAM, IDC_SIZEWE, IDC_SIZENS, IDC_SIZEALL };
    SetCursor(LoadCursor(NULL, CURSOR_NAMES[m_type]));
}
