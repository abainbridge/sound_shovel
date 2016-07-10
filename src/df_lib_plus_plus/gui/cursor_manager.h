#ifndef INCLUDED_CURSOR_H
#define INCLUDED_CURSOR_H


// ***************************************
// Manages the bitmap for the mouse cursor
// ***************************************


class Widget;


class CursorManager
{
public:
	// Cursor type enum
	enum
	{
		CursorMain,
		CursorText,
		CursorHoriDrag,
        CursorVertDrag,
		CursorNumCursors
	};

	Widget	*m_captured;		// Pointer to the Widget that is currently capturing mouse input. NULL, if no widget is capturing.

private:
	int		m_type;				// One of CursorMain, CursorText etc

public:
	CursorManager();
	void Advance();
	void RequestCursorType(int type);
	void Render(int x, int y);
};


#endif