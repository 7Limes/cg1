/*
    Functions for drawing primitives on SDL2 Surfaces.
*/

#ifndef RENDER_CPU_PRIMITIVES_HEADER
#define RENDER_CPU_PRIMITIVES_HEADER

#include <SDL2/SDL.h>


// Draw a single pixel.
static inline void surf_draw_point(SDL_Surface *surf, int x, int y, Uint32 color) {
    // Check if the point is out bounds
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) {
        return;
    }

    Uint32 *pixels = (Uint32*) surf->pixels;
    pixels[y * surf->w + x] = color;
}


/*
Return the rectangle created by the overlap between two other rectangles.
Returns a zeroed struct if there is no intersection.
*/
static inline SDL_Rect __get_rect_intersection(SDL_Rect a, SDL_Rect b) {
    // Find the leftmost right edge and the rightmost left edge
    int left = (a.x > b.x) ? a.x : b.x;
    int right = (a.x + a.w < b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    
    // Find the highest bottom edge and the lowest top edge
    int top = (a.y > b.y) ? a.y : b.y;
    int bottom = (a.y + a.h < b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    
    SDL_Rect result = {0};

    // Check if there is an intersection
    if (left < right && top < bottom) {
        result.x = left;
        result.y = top;
        result.w = right - left;
        result.h = bottom - top;
    }

    return result;
}


// Draw a filled rectangle.
static inline void surf_draw_rect(SDL_Surface *surf, int x, int y, int width, int height, Uint32 color) {
    // Compute the intersection of the desired rect and the viewport rect
    SDL_Rect draw_rect = __get_rect_intersection((SDL_Rect) {x, y, width, height}, (SDL_Rect) {0, 0, surf->w, surf->h});

    // No intersection found (desired rect is outside of viewport)
    if (!draw_rect.w) {
        return;
    }

    Uint32 *pixels = (Uint32*) surf->pixels;
    for (Uint32 i = draw_rect.y; i < draw_rect.y+draw_rect.h; i++) {
        for (Uint32 j = draw_rect.x; j < draw_rect.x+draw_rect.w; j++) {
            pixels[i * surf->w + j] = color;
        }
    }
}


// Region codes for Cohen-Sutherland
#define CS_INSIDE 0
#define CS_LEFT   1
#define CS_RIGHT  2
#define CS_BOTTOM 4
#define CS_TOP    8

// Helper to compute the region code for a point (x, y)
static inline int compute_region_code(int x, int y, int w, int h) {
    int code = CS_INSIDE;

    if (x < 0)           code |= CS_LEFT;
    else if (x >= w)     code |= CS_RIGHT;
    if (y < 0)           code |= CS_BOTTOM;
    else if (y >= h)     code |= CS_TOP;

    return code;
}

static inline int surf_clip_line(SDL_Surface* surf, int* x1, int* y1, int* x2, int* y2) {
    int w = surf->w;
    int h = surf->h;
    
    int code1 = compute_region_code(*x1, *y1, w, h);
    int code2 = compute_region_code(*x2, *y2, w, h);
    int accept = 0;

    while (true) {
        if (!(code1 | code2)) {
            // Both points inside window
            accept = 1;
            break;
        }
        else if (code1 & code2) {
            // Both points share an outside region (completely outside)
            break;
        }
        else {
            // Some segment is inside; calculate intersection
            int code_out = code1 ? code1 : code2;
            int x, y;

            // Standard line-intersection formulas
            if (code_out & CS_TOP) {
                x = *x1 + (*x2 - *x1) * (h - 1 - *y1) / (*y2 - *y1);
                y = h - 1;
            }
            else if (code_out & CS_BOTTOM) {
                x = *x1 + (*x2 - *x1) * (0 - *y1) / (*y2 - *y1);
                y = 0;
            }
            else if (code_out & CS_RIGHT) {
                y = *y1 + (*y2 - *y1) * (w - 1 - *x1) / (*x2 - *x1);
                x = w - 1;
            }
            else if (code_out & CS_LEFT) {
                y = *y1 + (*y2 - *y1) * (0 - *x1) / (*x2 - *x1);
                x = 0;
            }

            // Replace outside point with intersection point
            if (code_out == code1) {
                *x1 = x;
                *y1 = y;
                code1 = compute_region_code(*x1, *y1, w, h);
            }
            else {
                *x2 = x;
                *y2 = y;
                code2 = compute_region_code(*x2, *y2, w, h);
            }
        }
    }

    return accept;
}

// Draw a line.
static inline void surf_draw_line(SDL_Surface* surf, int x1, int y1, int x2, int y2, Uint32 color) {
    if (!surf_clip_line(surf, &x1, &y1, &x2, &y2)) {
        return;
    }

    Uint32 *pixels = (Uint32*) surf->pixels;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        pixels[y1 * surf->w + x1] = color;

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}


#endif