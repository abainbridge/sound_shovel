#pragma once


#include "df_rgba_colour.h"


class TooltipManager
{
private:
	char m_buf[256];
	int  m_x;
	int  m_y;
	int  m_w;
	int  m_h;
	RGBAColour m_col;
	float m_showTime;

public:
	void Advance();
	void Render();

	void ShowColouredString(int x, int y, int w, int h, RGBAColour &col, char const *fmt, ...);
};


extern TooltipManager g_tooltipManager;
