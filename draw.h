#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdint.h>
#include <sys/types.h>
#include "framebuffer.h"

typedef struct Location {
    ssize_t x, y;
    // ssize_t xsz, ysz;
} Location;

typedef struct Painter {
    FrameBuffer* fb;
    Location loc;
} Painter;

typedef struct Texter {

} Texter;

void painter_init(Painter* painter, FrameBuffer* fb);

static inline
FrameBuffer* painter_get_framebuffer(Painter* painter) {
    return painter->fb;
}
static inline
ssize_t painter_get_width(const Painter* painter) {
    return painter->fb->var.xres;
}
static inline 
size_t painter_get_height(const Painter* painter) {
    return painter->fb->var.yres / 2; // TODO
}

/* coordinate manipulation */
void painter_translate(Painter* painter, ssize_t x, ssize_t y);

/* draw */
void painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color);
void painter_draw_ract(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color);
void painter_draw_line(Painter* painter, ssize_t x1, ssize_t y1, ssize_t x2, ssize_t y2, uint32_t color);
void painter_draw_font(Painter* painter, ssize_t x, ssize_t y, char ch, uint32_t color, uint32_t bgcolor);

#endif /* _DRAW_H_ */
