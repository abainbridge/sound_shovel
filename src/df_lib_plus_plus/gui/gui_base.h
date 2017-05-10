#pragma once


// Contrib headers
#include "df_colour.h"

// Project headers
#include "mouse_cursor.h"
#include "widget.h"


#define WIDGET_SPACER 4
#define GUI_MANAGER_NAME "GuiManager"


class ContainerVert;
typedef struct _DfFont DfFont;


class GuiBase: public Widget
{
private:
	bool m_exitRequested;

	Widget *m_modalWidget;
	Widget *m_previouslyFocussedWidget;	// Only used when a modal widget is present

public:
	DfColour m_backgroundColour;
	DfColour m_frameColour1;                 // Darkest
	DfColour m_frameColour2;
	DfColour m_frameColour3;
	DfColour m_frameColour4;
	DfColour m_frameColour5;                 // Brightest
	DfColour m_selectionBlockColour;
	DfColour m_selectionBlockUnfocussedColour;
	DfColour m_highlightColour;
	DfColour m_strongHighlightColour;
	DfColour m_textColourNormal;
	DfColour m_textColourFrame;
	DfColour m_focusBoxColour;

    DfFont *m_propFont;    // Proportional width font used by Menus, status bar etc.

	MouseCursor m_mouseCursor;
	Widget *m_focussedWidget;
	ContainerVert *m_mainContainer;
	bool m_exitAtEndOfFrame;	// True if something has requested that the application quit
    bool m_canSleep;

    char *m_aboutString;
    bool m_highlightFocussedWidget;

private:
	DfColour	GetColour(char const *name, DfColour const &defaultColour);
	bool		StringToColour(char const *str, DfColour *col);	// Returns true on success
	void		SetColours();

public:
	GuiBase();
    virtual void Initialise() = 0;  // Implement this in a derived class and create and populate m_mainContainer from there.

	void		About();

	void		FillBackground(int x, int y, int w, int h, bool highlighted) const;
	void		DrawFrame(int x, int y, int w, int h) const;
	char*		ExecuteCommand(char const *object, char const *command, char const *arguments);
	void		RequestExit();	// Ask to quit the app

	// Override Widget base methods
	virtual void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);
	virtual Widget*	GetWidgetAtPos(int x, int y);
    virtual Widget*	GetWidgetByName(char const *name);
	virtual void Show(char const *widgetToShow);
	virtual void Advance();
	virtual void Render();

	void		SetModalWidget(Widget *w);

    void        SetFocussedWidget(Widget *w);
};


extern GuiBase *g_gui;
