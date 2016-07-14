#ifndef INCLUDED_DRAWING_PRIMITIVES
#define INCLUDED_DRAWING_PRIMITIVES


#include "df_colour.h"


void DrawOutlineBox(int x1, int y1, int w, int h, DfColour const &c);
void DrawRaisedBox(int x1, int y1, int w, int h, DfColour const &c1, DfColour const &c2);
void DrawFilledBox(int x1, int y1, int w, int h, DfColour const &c);
void DrawHLine(int x, int y, int len, DfColour const &c);


#endif
