// Own header
#include "cursor_manager.h"

// Contrib headers
#include "df_input.h"
#include "df_window_manager.h"

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
	if (g_inputManager.lmbUnClicked) 
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
//	if (g_window->IsMouseInWindow())    // TODO - Implement IsMouseInWindow
    {
        switch(m_type)
        {
            case CursorMain:
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                break;
            case CursorText:
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                break;
            case CursorHoriDrag:
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                break;
            case CursorVertDrag:
                SetCursor(LoadCursor(NULL, IDC_SIZENS));
                break;
        }
    }
}
