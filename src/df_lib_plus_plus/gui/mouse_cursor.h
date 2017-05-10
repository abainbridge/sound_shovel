#pragma once


// ***********************************************
// Mainly controls the bitmap for the mouse cursor
// ***********************************************


class Widget;


class MouseCursor
{
public:
	enum
	{
		CursorMain,
		CursorText,
		CursorDragHori,
        CursorDragVert,
        CursorDragBoth,
		CursorNumCursors
	};

	Widget	*m_captured;		// Pointer to the Widget that is currently capturing mouse input. NULL, if no widget is capturing.

private:
	int		m_type;				// One of CursorMain, CursorText etc

public:
	MouseCursor();
	void Advance();
	void RequestCursorType(int type);
	void Render(int x, int y);
};
