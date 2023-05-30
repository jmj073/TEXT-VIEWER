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
	ssize_t xloc = painter->loc.x;
	ssize_t yloc = painter->loc.y;
	ssize_t width = painter->fb->var.xres;
	// ssize_t height = painter->fb->var.yres;

	uint32_t* buf = painter->fb->start;

	buf[((yloc + y) * width) + (xloc + x)] = color;
}

// location ==============================================================

static inline
void loc_init(Location* loc) {
    memset(loc, 0, sizeof(Location));
}

static inline
void loc_translate(Location* loc, ssize_t x, ssize_t y) {
    loc->x += x;
    loc->y += y;
}

// painter =============================================================

/* coordinate manipulation */

void painter_init(Painter* painter, FrameBuffer* fb) {
	loc_init(&painter->loc);
	painter->fb = fb;
}

void painter_translate(Painter* painter, ssize_t x, ssize_t y) {
	loc_translate(&painter->loc, x, y);
}

/* draw */

void painter_draw_pixel(Painter* painter, ssize_t x, ssize_t y, uint32_t color) {
	drawpixel(painter, x, y, color);
}

void painter_draw_ract(Painter* painter, ssize_t x, ssize_t y, ssize_t xsz, ssize_t ysz, uint32_t color) {
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

// void drawText(FrameBuffer *gfb, int x, int y, char *msg, unsigned int color, unsigned int bgcolor)
// {
//     while (*msg) {
// 		drawFont(gfb, x, y, *msg++, color, bgcolor);
// 		x += 8;
// 	}
// }

void painter_draw_font(Painter* painter, ssize_t x, ssize_t y, char ch, uint32_t color, uint32_t bgcolor) {
	unsigned char *font = fontdata_8x16 + 16 * (unsigned char)ch;
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
