#pragma once


#include "widget.h"


int const c_statusBarHeight = 17;


class StatusBar: public Widget
{
private:
	char        *m_messageBuffer;
	float       m_messageStartTime;
	bool        m_messageIsError;

public:
    StatusBar(Widget *parent);

	void ShowMessage(char const *message, ...);
	void ShowError(char const *message, ...);
	
	void Advance();
	void Render();
	char *ExecuteCommand(char const *object, char const *command, char const *arguments);
};


extern StatusBar *g_statusBar;
