#pragma once


#include "command.h"


class Widget: public CommandReceiver
{
public:
	enum
	{
		HideStateShown,
		HideStateHidden,
		HideStateNewlyShown     // In the process of becoming shown. Will become shown after the next SetRect in its container
	};

	// The four variables below hold the bounding rectangle of the widget. The container 
	// object should call SetRect whenever the bounds change.
	int			m_width;
	int			m_height;
	int			m_left;
	int			m_top;
    char        *m_name;
    int         m_hideState;
	bool		m_growable;	// True if the widget can stretch/shrink. False if it must have the specified size.
	bool		m_highlightable;
    bool        m_ghosted;
    Widget      *m_parent;

protected:
	void DrawFilledBoxHoriGrad(int x, int y, int w, int h);
	void DrawFilledBoxVertGrad(int x, int y, int w, int h);

public:
	Widget(char const *name, Widget *parent, int w=20, int h=20);
    virtual ~Widget();

	virtual void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);
	bool IsMouseInBounds();
	virtual Widget *GetWidgetAtPos(int x, int y);	// Recursively search all child widgets - return the deepest Widget that hits the specified point
    virtual Widget *GetWidgetByName(char const *name); // Recursively finds a widget

    virtual Widget *GetLeftWidget(Widget *caller);
    virtual Widget *GetRightWidget(Widget *caller);

	virtual void Advance();
	virtual void Render() = 0;

	virtual void ToggleHide();
    virtual void Hide();
    virtual void Show(char const *widgetToShow);

	virtual char *ExecuteCommand(char const *object, char const *command, char const *arguments);
};
