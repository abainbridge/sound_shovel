#include "drawing_primitives.h"
#include "df_colour.h"
#include "df_window.h"


void DrawOutlineBox(int x1, int y1, int w, int h, DfColour const &c)
{
    HLine(g_window->bmp, x1, y1, w, c);
    HLine(g_window->bmp, x1, y1 + h - 1, w, c);
    VLine(g_window->bmp, x1, y1, h, c);
    VLine(g_window->bmp, x1 + w - 1, y1, h, c);
}


void DrawRaisedBox(int x1, int y1, int w, int h, DfColour const &c1, DfColour const &c2)
{
    DrawOutlineBox(x1, y1, w, h, c1);
}


void DrawFilledBox(int x1, int y1, int w, int h, DfColour const &c)
{
    RectFill(g_window->bmp, x1, y1, w, h, c);
}


void DrawHLine(int x, int y, int len, DfColour const &c)
{
    HLine(g_window->bmp, x, y, len, c);
}