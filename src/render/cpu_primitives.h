/*
Functions for drawing primitives on SDL2 Surfaces.
*/

#ifndef RENDER_CPU_PRIMITIVES_HEADER
#define RENDER_CPU_PRIMITIVES_HEADER

#include <SDL2/SDL.h>


// Draw a single pixel.
void surf_draw_point(SDL_Surface *surf, int x, int y, Uint32 color);

// Draw a filled rectangle.
void surf_draw_rect(SDL_Surface *surf, int x, int y, int width, int height, Uint32 color);

// Draw a line.
void surf_draw_line(SDL_Surface* surf, int x1, int y1, int x2, int y2, Uint32 color);


#endif