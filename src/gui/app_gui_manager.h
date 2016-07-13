#pragma once


#include "gui/gui_manager_base.h"


class AppGuiManager: public GuiManagerBase
{
public:
	AppGuiManager();
    void Initialise();

    // GuiManagerBase overrides:
    void Advance();
};
