#pragma once


#include "gui/gui_base.h"


class AppGui: public GuiBase
{
public:
	AppGui();
    void Initialise();

    // GuiManagerBase overrides:
    void Advance();
};
