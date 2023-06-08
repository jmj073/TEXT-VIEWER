#include "draw.h"

#include <stdarg.h>
#include <stdio.h> // for texter_printf
#include <string.h> // memset
#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "font_data.h"

// binder ==============================================================================

/**
 * @brief 
 * 
 * @param binder 
 * @param fb 
 * @param origin_x
 * @param origin_y
 * @param width If 0, the default value is used
 * @param height If 0, the default value is used
 * @return int success: 1, fail: 0
 */
int binder_init(Binder* binder, FrameBuffer* fb, size_t origin_x, size_t origin_y, size_t width, size_t height) {
	if (!binder || !fb) return 0;

	binder->fb = fb;
	binder->x = origin_x;
	binder->y = origin_y;

	if (!binder_set_width(binder, width ?: fb->var.xres)) return 0;
	if (!binder_set_height(binder, height ?: fb->var.yres / 2)) return 0;

	return 1;
}

int binder_fill(Binder* binder, uint32_t color) {
	if (!binder) return 0;

	ssize_t xloc = binder->x;
	ssize_t yloc = binder->y;
	ssize_t width = binder->fb->var.xres; // real width
	// ssize_t height = painter->fb->var.yres;

	uint32_t* buf = binder->fb->start;

	size_t x, y;

	for (y = 0; y < binder->height; ++y) {
		for (x = 0; x < binder->width; ++x) {
			buf[((yloc + y) * width) + (xloc + x)] = color;
		}
	}

	return 1;
}

static inline
int __binder_oob(const Binder* binder, size_t x, size_t y) {
	return x >= binder->width || x + binder->x >= binder->fb->var.xres
		|| y >= binder->height || y + binder->y >= binder->fb->var.yres / 2;
}

static inline
int __binder_draw_pixel(Binder* binder, size_t x, size_t y, uint32_t color) {
	if (__binder_oob(binder, x, y)) return 0;

	ssize_t xloc = binder->x;
	ssize_t yloc = binder->y;
	ssize_t width = binder->fb->var.xres; // real width
	// ssize_t height = painter->fb->var.yres;

	uint32_t* buf = binder->fb->start;
	
	buf[((yloc + y) * width) + (xloc + x)] = color;

	return 1;
}

// painter ==============================================================================

int painter_init(Painter* painter, Binder* binder) {
	if (!painter || !binder) return 0;

	painter->binder = binder;

	painter->sys.x = 0;
	painter->sys.y = 0;

	return 1;
}

void painter_copy(Painter* dst, Painter* src) {
	*dst = *src;
}

static inline
void __painter_drawpixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color) {
	__binder_draw_pixel(painter->binder, x + painter->sys.x, y + painter->sys.y, color);
}

int painter_translate(Painter* painter, ssize_t x, ssize_t y) {
	if (!painter) return 0;
	painter->sys.x += x;
	painter->sys.y += y;
	return 1;
}

int painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color) {
	if (!painter) return 0;
	__painter_drawpixel(painter, x, y, color);
	return 1;
}

int painter_draw_fill_rect(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color) {
	if (!painter) return 0;

	ssize_t i, j;

	for (i = 0; i < ysz; ++i) {
		for (j = 0; j < xsz; ++j) {
			__painter_drawpixel(painter, x + j, y + i, color);
		}
	}

	return 1;
}

int painter_draw_line(Painter* painter, ssize_t x1, ssize_t y1, ssize_t x2, ssize_t y2, uint32_t color) {
	if (!painter) return 0;
	
	if (x1 == x2) {
		if (y1 > y2) _swap(y1, y2);
		ssize_t i, ysz = y2 - y1;
		for (i = 0; i < ysz; i++) {
			__painter_drawpixel(painter, x1, i + y1, color);
		}
		return 1;
	}
	
	ssize_t x = 0, y = 0;
	
	if (abs(x2 - x1) >= abs(y2 - y1)) {
		if (x1 > x2) {
			_swap(x1, x2);
			_swap(y1, y2);
		}
		ssize_t xsz = x2 - x1;
		for (; x < xsz; ++x) {
			__painter_drawpixel(painter, x1 + x, y1 + y / xsz, color);
			y += y2 - y1;
		}
	} else {
		if (y1 > y2) {
			_swap(x1, x2);
			_swap(y1, y2);
		}
		ssize_t ysz = y2 - y1;
		for (; y < ysz; ++y) {
			__painter_drawpixel(painter, x1 + x / ysz , y1 + y, color);
			x += x2 - x1;
		}
	}

	return 1;
}

// void draw_circle(unsigned int (*buf)[1280], int x, int y, int r, unsigned int color) {
// 	int i, j;
// 	for (i = 0; i < r * 2; i++) {
// 		for (j = 0; j < r * 2; j++) {
// 			int tmp = (i - r) * (i - r) + (j - r) * (j - r);
// 			if (tmp >= (r-1)*(r-1) && tmp <= r*r)
// 				buf[j + y][i + x] = color;
// 		}
// 	}
// }

// texter ==============================================================================

/* cursor position(row and column) to painter coordinate(x and y) */
static inline size_t row2y(size_t row) { return row * FONT_HEIGHT; }
static inline size_t col2x(size_t col) { return col * FONT_WIDTH; }

static inline
void __draw_font(Texter* texter, size_t row, size_t col, char ch) {
    ssize_t x = col2x(col), y = row2y(row);
    uint32_t color = texter_get_font_color(texter);
    uint32_t bgcolor = texter_get_background_color(texter);
    unsigned char *font = fontdata_8x16[(uint8_t)ch];

	ssize_t r, c;
	for (r = 0; r < FONT_HEIGHT; r++) {
		for (c = 0; c < FONT_WIDTH; c++) {
			__binder_draw_pixel(texter->binder, x + c, y + r,
                (font[r] >> (FONT_WIDTH - c - 1)) & 1 ? color : bgcolor);
		}
	}
}

static inline
int __cursor_is_valid(Texter* texter) {
	size_t width_round_up = texter->auto_newline ? 0 : FONT_WIDTH - 1;

	size_t colum_size = (texter->binder->width + width_round_up) / FONT_WIDTH;
	size_t row_size = (texter->binder->height + FONT_HEIGHT - 1) / FONT_HEIGHT;
	return texter->cur.r < row_size && texter->cur.c < colum_size;
}

static inline
int __cursor_next(Texter* texter) {
	size_t row = texter->cur.r;
	size_t col = texter->cur.c;

	if (texter->auto_newline) {
		size_t colum_size = texter->binder->width / FONT_WIDTH;
		size_t row_size = (texter->binder->height + FONT_HEIGHT - 1) / FONT_HEIGHT;

		if (col + 1 >= colum_size) {
			if (row >= row_size) return 0;
			texter->cur.r = row + 1;
			texter->cur.c = 0;
		} else {
			texter->cur.c = col + 1;
		}
	} else {
		size_t colum_size = (texter->binder->width + FONT_WIDTH - 1) / FONT_WIDTH;
		if (col >= colum_size) return 0;
		texter->cur.c = col + 1;
	}

	return 1;
}

int texter_init(Texter* texter, Binder* binder) {
	if (!texter || !binder) return 0;
	
	texter->binder = binder;

	/* cursor */
	texter->cur.r = 0;
	texter->cur.c = 0;

	/* color */
	texter->font_color = TEXTER_DEFAULT_FONT_COLOR;
	texter->background_color = TEXTER_DEFAULT_BACKGROUND_COLOR;

	/* flags */
	texter->auto_newline = 1;

	return 1;
}

int texter_set_cursor(Texter* texter, size_t row, size_t col) {
	if (!texter) return 0;
	texter->cur.r = row;
	texter->cur.c = col;
	return 1;
}

int texter_move_cursor(Texter* texter, size_t row, size_t col) {
	if (!texter) return 0;
	texter->cur.r += row;
	texter->cur.c += col;
	return 1;
}

int texter_putc(Texter* texter, char c) {
	switch(c) {
		case '\n': // line feed
			++texter->cur.r; // TODO 개선
			// no break
		case '\r': // carriage return
			texter->cur.c = 0;
			break;
		default:
			if (!texter_drawc(texter, c)) return 0;
			__cursor_next(texter);
	}
	return 1;
}

int texter_drawc(Texter* texter, char c) {
    return texter_pos_drawc(texter, c, texter_get_row(texter), texter_get_column(texter));
}

int texter_pos_drawc(Texter* texter, char ch, size_t r, size_t c) {
	if (!texter || !__cursor_is_valid(texter)) return 0;
    __draw_font(texter, r, c, ch);
	return 1;
}

ssize_t texter_write(Texter* texter, const char* buf, size_t count) {
	if (!texter || !buf) return -1;

	ssize_t cnt = 0;

	while (count-- && texter_putc(texter, *buf++)) {
		++cnt;
	}

	return cnt;
}

ssize_t texter_puts(Texter* texter, const char* str) {
	if (!texter || !str) return -1;

	ssize_t cnt = 0;

	while (*str && texter_putc(texter, *str++)) {
		++cnt;
	}

	return cnt;
}

ssize_t texter_printf(Texter* texter, const char* fmt, ...) {
	if (!texter) return -1;
	
	char buf[1024];
	va_list ap;
	va_start(ap, &fmt);
	int ret = vsnprintf(buf, sizeof(buf), fmt, ap); // TODO
	va_end(ap);

	if (ret < 0) return (ssize_t)ret;
	assert(ret != sizeof(buf));

	return texter_puts(texter, buf);
}

int texter_clear(Texter* texter) {
	if (!texter) return 0;
	Binder* binder = texter_get_binder(texter);
	if (!binder_fill(binder, texter_get_background_color(texter))) return 0;
	return texter_set_cursor(texter, 0, 0);
}