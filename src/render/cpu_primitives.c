#include <SDL2/SDL.h>


/*
Return the rectangle created by the overlap between two other rectangles.
Returns a zeroed struct if there is no intersection.
*/
SDL_Rect get_rect_intersection(SDL_Rect a, SDL_Rect b) {
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


void surf_draw_point(SDL_Surface *surf, int x, int y, Uint32 color) {
    // Check if the point is out bounds
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) {
        return;
    }

    Uint32 *pixels = (Uint32*) surf->pixels;
    pixels[y * surf->w + x] = color;
}


void surf_draw_rect(SDL_Surface *surf, int x, int y, int width, int height, Uint32 color) {
    // Compute the intersection of the desired rect and the viewport rect
    SDL_Rect draw_rect = get_rect_intersection((SDL_Rect) {x, y, width, height}, (SDL_Rect) {0, 0, surf->w, surf->h});

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


// TODO: don't try to draw pixels outside of the window
// if both points inside window, no change
// if one point outside and one point inside, then shift the outside point towards the inside point until the outside point intersects with the window edge
    // probably use ray segment intersection here to immediately determine the point
// if both points outside, then don't draw anything
void surf_draw_line(SDL_Surface* surf, int x1, int y1, int x2, int y2, Uint32 color) {
    Uint32 *pixels = (Uint32*) surf->pixels;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        if (x1 >= 0 && x1 < surf->w && y1 >= 0 && y1 < surf->h) {
            pixels[y1 * surf->w + x1] = color;
        }

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