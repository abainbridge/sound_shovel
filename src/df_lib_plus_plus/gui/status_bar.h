#pragma once


#include "widget.h"


int const c_statusBarHeight = 17;


class StatusBar: public Widget
{
private:
    enum { MAX_MESSAGE_LEN = 256 };

	char        m_messageBuffer[MAX_MESSAGE_LEN];
	float       m_messageStartTime;
	bool        m_messageIsError;
    
    // When there is no Message or Error to be displayed, then the following two
    // strings.
    char        m_leftBuffer[MAX_MESSAGE_LEN];
    char        m_rightBuffer[MAX_MESSAGE_LEN];

public:
    StatusBar(Widget *parent);

	void ShowMessage(char const *message, ...);
	void ShowError(char const *message, ...);

    void SetLeftString(char const *fmt, ...);
    void SetRightString(char const *fmt, ...);
	
	void Advance();
	void Render();
	char *ExecuteCommand(char const *object, char const *command, char const *arguments);
};


extern StatusBar *g_statusBar;
