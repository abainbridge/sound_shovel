#pragma once


// Contrib headers
#include "df_rgba_colour.h"

// Project headers
#include "cursor_manager.h"
#include "widget.h"


#define WIDGET_SPACER 4
#define OUTPUT_WINDOW_NAME "OutputWindow"
#define PYTHON_OUTPUT_TAB "Python Output"
#define FIND_RESULTS_TAB "Find Results"
#define GUI_MANAGER_NAME "GuiManager"


class MenuBar;
class StatusBar;
class ContainerVert;


class GuiManager: public Widget
{
private:
	bool			m_exitRequested;

	Widget			*m_modalWidget;
	Widget			*m_previouslyFocussedWidget;	// Only used when a modal widget is present

public:
	RGBAColour	m_backgroundHighlightColour;
	RGBAColour	m_backgroundShadowColour;
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
	RGBAColour	m_textColourComment;
	RGBAColour	m_textColourKeyword;
	RGBAColour	m_textColourString;
	RGBAColour	m_textColourNumber;
	RGBAColour	m_textColourFrame;
	RGBAColour  m_focusBoxColour;

	CursorManager	m_cursorManager;
	Widget			*m_focussedWidget;
	ContainerVert	*m_mainContainer;
	bool			m_exitAtEndOfFrame;	// True if something has requested that the application quit

private:
	RGBAColour	GetColour(char const *name, RGBAColour const &defaultColour);
	bool		StringToColour(char const *str, RGBAColour *col);	// Returns true on success
	void		SetColours();
	void		RenderBoxedWidget(Widget *widget);

public:
	GuiManager();
	void		Initialise();

	void		About();

	void		FillBackground(int x, int y, int w, int h, bool highlighted) const;
	void		DrawFrame(int x, int y, int w, int h) const;
	char*		ExecuteCommand(char const *object, char const *command, char const *arguments);
	void		RequestExit();	// Ask to quit the app

	// Override Widget base methods
	virtual void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);
	virtual Widget*	GetWidgetAtPos(int x, int y);
    virtual Widget*		GetWidgetByName(char const *name);
	virtual void		Show(char const *widgetToShow);
	virtual void		Advance();
	virtual void		Render();

	void		SetModalWidget(Widget *w);

    void        SetFocussedWidget(Widget *w);
};


extern GuiManager *g_guiManager;
