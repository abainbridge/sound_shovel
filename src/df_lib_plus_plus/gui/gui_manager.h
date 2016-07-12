#pragma once


// Contrib headers
#include "df_rgba_colour.h"

// Project headers
#include "cursor_manager.h"
#include "widget.h"


#define WIDGET_SPACER 4
#define GUI_MANAGER_NAME "GuiManager"


class ContainerVert;
typedef struct _TextRenderer TextRenderer;


class GuiManager: public Widget
{
private:
	bool m_exitRequested;

	Widget *m_modalWidget;
	Widget *m_previouslyFocussedWidget;	// Only used when a modal widget is present

public:
	RGBAColour	m_backgroundColour;
	RGBAColour	m_frameColour1;                 // Darkest
	RGBAColour	m_frameColour2;
	RGBAColour	m_frameColour3;
	RGBAColour	m_frameColour4;
	RGBAColour	m_frameColour5;                 // Brightest
	RGBAColour	m_selectionBlockColour;
	RGBAColour  m_selectionBlockUnfocussedColour;
	RGBAColour  m_highlightColour;
	RGBAColour  m_strongHighlightColour;
	RGBAColour	m_textColourNormal;
	RGBAColour	m_textColourFrame;
	RGBAColour  m_focusBoxColour;

    TextRenderer *m_propFont;    // Proportional width font used by Menus, status bar etc.

	CursorManager m_cursorManager;
	Widget *m_focussedWidget;
	ContainerVert *m_mainContainer;
	bool m_exitAtEndOfFrame;	// True if something has requested that the application quit

private:
	RGBAColour	GetColour(char const *name, RGBAColour const &defaultColour);
	bool		StringToColour(char const *str, RGBAColour *col);	// Returns true on success
	void		SetColours();

public:
	GuiManager();
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


extern GuiManager *g_guiManager;
