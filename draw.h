#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdint.h>
#include <sys/types.h>
#include "framebuffer.h"

// declaration =======================================================

typedef struct Painter {
    FrameBuffer* fb;

    // coordinate system
    struct CoordSys {
        ssize_t x, y;
        ssize_t xsz, ysz;
    } sys;
} Painter;

typedef struct Texter {
    Painter* painter;
    struct Cursor {
        ssize_t r, c; // row, column
    } cur;
} Texter;

// painter ===========================================================

void painter_init(Painter* painter, FrameBuffer* fb);
void painter_copy(Painter* dst, Painter* src);

static inline
FrameBuffer* painter_get_framebuffer(Painter* painter) {
    return painter->fb;
}

static inline
ssize_t painter_get_width(const Painter* painter) {
    return painter->sys.xsz;
}
static inline
void painter_set_width(Painter* painter, ssize_t width) {
    painter->sys.xsz = width;
}

static inline 
size_t painter_get_height(const Painter* painter) {
    return painter->sys.ysz;
}
static inline
void painter_set_height(Painter* painter, ssize_t height) {
    painter->sys.ysz = height;
}

/* check out of bound */
static inline
int painter_check_oob(const Painter* painter, ssize_t x, ssize_t y) {
    return x < 0 || x >= painter_get_width(painter)
        || y < 0 || y >= painter_get_height(painter);
}

/* coordinate manipulation */
void painter_translate(Painter* painter, ssize_t x, ssize_t y);

/* draw */
void painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color);
void painter_draw_rect(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color);
void painter_draw_line(Painter* painter, ssize_t x1, ssize_t y1, ssize_t x2, ssize_t y2, uint32_t color);
void painter_draw_font(Painter* painter, ssize_t x, ssize_t y, char ch, uint32_t color, uint32_t bgcolor);

// texter ============================================================

void texter_init(Texter* texter, Painter* painter);

static inline
Painter* texter_get_painter(Texter* texter) {
    return texter->painter;
}

void texter_set_cursor(Texter* texter, ssize_t row, ssize_t col);
void texter_move_cursor(Texter* texter, ssize_t row, ssize_t col);



#endif /* _DRAW_H_ */
