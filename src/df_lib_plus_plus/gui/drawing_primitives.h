#ifndef INCLUDED_DRAWING_PRIMITIVES
#define INCLUDED_DRAWING_PRIMITIVES


#include "df_rgba_colour.h"


void DrawOutlineBox(int x1, int y1, int w, int h, RGBAColour const &c);
void DrawRaisedBox(int x1, int y1, int w, int h, RGBAColour const &c1, RGBAColour const &c2);
void DrawFilledBox(int x1, int y1, int w, int h, RGBAColour const &c);
void DrawHLine(int x, int y, int len, RGBAColour const &c);


#endif
