#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdint.h>
#include <sys/types.h>
#include "framebuffer.h"

typedef struct Binder {
    FrameBuffer* fb;
    size_t x, y;
    size_t width, height;
} Binder;

typedef struct Painter {
    Binder* binder;

    // coordinate system
    struct CoordSys {
        ssize_t x, y;
    } sys;
} Painter;

typedef struct Texter {
    Binder* binder;
    struct Cursor {
        size_t r, c; // row, column
    } cur;

    uint32_t font_color;
    uint32_t background_color;

    /* flags */
    // char auto_increment;
    uint8_t auto_newline;
} Texter;

// binder ================================================================

int binder_init(Binder* binder, FrameBuffer* fb, size_t origin_x, size_t origin_y, size_t width, size_t height);

static inline
int binder_set_width(Binder* binder, size_t width) {
    if (!width) return 0;
    binder->width = width;
    return 1;
}
static inline
size_t binder_get_width(const Binder* binder) {
    return binder->width;
}
static inline
int binder_set_height(Binder* binder, size_t height) {
    if (!height) return 0;
    binder->height = height;
    return 1;
}
static inline
size_t binder_get_height(const Binder* binder) {
    return binder->height;
}

// painter ==================================================================

int painter_init(Painter* painter, Binder* binder);
void painter_copy(Painter* dst, Painter* src);

static inline
Binder* painter_get_binder(Painter* painter) {
    return painter->binder;
}

/* coordinate manipulation */
int painter_translate(Painter* painter, ssize_t x, ssize_t y);

/* draw */
int painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color);
int painter_draw_rect(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color);
int painter_draw_line(Painter* painter, ssize_t x1, ssize_t y1, ssize_t x2, ssize_t y2, uint32_t color);

// texter ========================================================================

int texter_init(Texter* texter, Binder* binder);

static inline
Binder* texter_get_binder(Texter* texter) {
    return texter->binder;
}

int texter_set_cursor(Texter* texter, size_t row, size_t col);
int texter_move_cursor(Texter* texter, size_t row, size_t col);

static inline
size_t texter_get_row(Texter* texter) {
    return texter->cur.r;
}
static inline
size_t texter_get_column(Texter* texter) {
    return texter->cur.c;
}

static inline
int texter_set_font_color(Texter* texter, uint32_t color) {
    if (!texter) return 0;
    texter->font_color = color;
    return 1;
}
static inline
uint32_t texter_get_font_color(const Texter* texter) {
    return texter->font_color;
}
static inline
int texter_set_background_color(Texter* texter, uint32_t color) {
    if (!texter) return 0;
    texter->background_color = color;
    return 1;
}
static inline
uint32_t texter_get_background_color(const Texter* texter) {
    return texter->background_color;
}

static inline
uint8_t texter_get_auto_newline(const Texter* texter) {
    if (!texter) return 0;
    return texter->auto_newline;
}
static inline
int texter_set_auto_newline(Texter* texter, uint8_t auto_newline) {
    if (!texter) return 0;
    texter->auto_newline = auto_newline;
    return 1;
}

int texter_putc(Texter* texter, char c); // return: refer to fputc(3)
int texter_drawc(Texter* texter, char c);
int texter_pos_drawc(Texter* texter, char ch, size_t r, size_t c);
ssize_t texter_write(Texter* texter, const char* buf, size_t count);
ssize_t texter_puts(Texter* texter, const char* str);

#endif /* _DRAW_H_ */
