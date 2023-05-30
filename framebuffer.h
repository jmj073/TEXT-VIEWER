#ifndef _FRAME_BUFFER_H_
#define _FRAME_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <linux/fb.h>

typedef struct FrameBuffer {
	int         fd;
	void        *start;
	size_t      length;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
} FrameBuffer;

int fb_open(const char* path, FrameBuffer *fb);
void fb_close(FrameBuffer *fb);

#endif /* _FRAME_BUFFER_H_ */
