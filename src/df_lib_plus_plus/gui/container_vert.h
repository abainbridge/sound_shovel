#pragma once


#include "container.h"


class ContainerVert: public Container
{
private:
    Widget      *m_resizingWidget;      // Points to the Widget that we are currently resizing. NULL if not currently resizing

private:
    void AdvanceResizing();

public:
	ContainerVert(char const *name, Widget *parent, int x=20, int y=20);

	void AddWidget(Widget *w);
	void RemoveWidget(char const *name);

    void Advance();
	void Render();

	void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);	// If x==-1 then stay the same size, but re-layout all sub widgets
};
