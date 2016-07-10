#pragma once


#include "container.h"


class ContainerHori: public Container
{
private:
    Widget *m_resizingWidget;      // Points to the Widget that we are currently resizing. NULL if not currently resizing

private:
    void AdvanceResizing();

public:
	ContainerHori(char const *name, Widget *parent);

	void AddWidget(Widget *w);

    virtual void Advance();
	virtual void Render();

	virtual void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);	// If x==-1 then stay the same size, but re-layout all sub widgets
};
