#include "container.h"


Container::Container(char const *name, Widget *parent, int w, int h)
:   Widget(name, parent, w, h)
{
	m_highlightable = false;
}


Widget *Container::GetWidgetAtPos(int x, int y)
{
	for (int i = 0; i < m_widgets.Size(); ++i)
	{
        Widget *w = m_widgets[i]->GetWidgetAtPos(x, y);
	    if (w) return w;
    }
    
    return Widget::GetWidgetAtPos(x, y);
}


Widget *Container::GetWidgetByName(char const *name)
{
    for (int i = 0; i < m_widgets.Size(); ++i)
    {
        Widget *child = m_widgets[i];
        Widget *w = child->GetWidgetByName(name);
        if (w) 
			return w;
    }

    Widget *w = Widget::GetWidgetByName(name);
    return w;
}


void Container::Show(char const *name)
{
    for (int i = 0; i < m_widgets.Size(); ++i)
    {
        Widget *child = m_widgets[i];
		Widget *w = child->GetWidgetByName(name);
        if (w) 
		{
			child->Show(name);
			return;
		}
    }

	DebugAssert(0);
}
