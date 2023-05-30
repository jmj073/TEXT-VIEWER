#include "draw.h"
#include <string.h> // memset
#include <stdlib.h>

#include "font_data.h"

#define swap(a, b) ({\
	__typeof__(a) tmp = a;\
	a = b;\
	b = tmp;\
	(void)0;\
})

static inline
void drawpixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color) {
	// if (painter_check_oob(painter, x, y)) return;
	
	ssize_t xloc = painter->sys.x;
	ssize_t yloc = painter->sys.y;
	ssize_t width = painter->fb->var.xres; // real width
	// ssize_t height = painter->fb->var.yres;

	uint32_t* buf = painter->fb->start;

	buf[((yloc + y) * width) + (xloc + x)] = color;
}

// painter =============================================================

/* coordinate manipulation */

void painter_init(Painter* painter, FrameBuffer* fb) {
	painter->fb = fb;

	painter->sys.x = 0;
	painter->sys.y = 0;
	painter->sys.xsz = painter->fb->var.xres;
	painter->sys.ysz = painter->fb->var.yres / 2; // TODO
}

void painter_copy(Painter* dst, Painter* src) {
	*dst = *src;
}

void painter_translate(Painter* painter, ssize_t x, ssize_t y) {
	painter->sys.x += x;
	painter->sys.y += y;
}

/* draw */
void painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color) {
	drawpixel(painter, x, y, color);
}

void painter_draw_rect(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color) {
	ssize_t i, j;

	for (i = 0; i < ysz; ++i) {
		for (j = 0; j < xsz; ++j) {
			drawpixel(painter, x + j, y + i, color);
		}
	}
}

void painter_draw_line(Painter* painter, ssize_t x1, ssize_t y1, ssize_t x2, ssize_t y2, uint32_t color) {	
	if (x1 == x2) {
		if (y1 > y2) swap(y1, y2);
		ssize_t i, ysz = y2 - y1;
		for (i = 0; i < ysz; i++) {
			drawpixel(painter, x1, i + y1, color);
		}
		return;
	}
	
	ssize_t x = 0, y = 0;
	
	if (abs(x2 - x1) >= abs(y2 - y1)) {
		if (x1 > x2) {
			swap(x1, x2);
			swap(y1, y2);
		}
		ssize_t xsz = x2 - x1;
		for (; x < xsz; ++x) {
			drawpixel(painter, x1 + x, y1 + y / xsz, color);
			y += y2 - y1;
		}
	} else {
		if (y1 > y2) {
			swap(x1, x2);
			swap(y1, y2);
		}
		ssize_t ysz = y2 - y1;
		for (; y < ysz; ++y) {
			drawpixel(painter, x1 + x / ysz , y1 + y, color);
			x += x2 - x1;
		}
	}
}

void painter_draw_font(Painter* painter, ssize_t x, ssize_t y, char ch, uint32_t color, uint32_t bgcolor) {
	// unsigned char *font = fontdata_8x16 + 16 * (unsigned char)ch;
	unsigned char *font = fontdata_8x16[(unsigned char)ch];
	ssize_t r, c;

	for (r = 0; r < 16; r++) {
		for (c = 0; c < 8; c++) {
			drawpixel(painter, x + c, y + r, ((font[r] >> (7 - c)) & 1 ? color : bgcolor));
		}
	}
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

// texter ======================================================================

void texter_init(Texter* texter, Painter* painter) {
	texter->painter = painter;

	texter->cur.r = 0;
	texter->cur.c = 0;
}

void texter_set_cursor(Texter* texter, ssize_t row, ssize_t col) {
	texter->cur.r = row;
	texter->cur.c = col;
}

void texter_move_cursor(Texter* texter, ssize_t row, ssize_t col) {
	texter->cur.r += row;
	texter->cur.c += col;
}