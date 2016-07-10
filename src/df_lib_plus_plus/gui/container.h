#pragma once


#include "containers/llist.h"
#include "widget.h"


class Container: public Widget
{
protected:
    LList <Widget*> m_widgets;

public:
	Container(char const *name, Widget *parent, int w=20, int h=20);

	virtual Widget *GetWidgetAtPos(int x, int y);
    virtual Widget *GetWidgetByName(char const *name);
	virtual void Show(char const *widgetToShow);
};
