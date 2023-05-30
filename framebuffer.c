#include "framebuffer.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h> // perror
#include <sys/ioctl.h>
#include <unistd.h> // open, close

int fb_open(const char* path, FrameBuffer *fb) {
	int fd;
	int ret;

	fd = open(path, O_RDWR);
	if(fd < 0){
		perror("FB Open");
		return -1;
	}
	ret = ioctl(fd, FBIOGET_FSCREENINFO, &fb->fix);
	if(ret < 0){
		perror("FB ioctl");
		close(fd);
		return -1;
	}
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &fb->var);
	if(ret < 0){
		perror("FB ioctl");
		close(fd);
		return -1;
	}
	fb->start = (unsigned char *)mmap (0, fb->fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb->start == NULL){
		perror("FB mmap");
		close(fd);
		return -1;
	}
	fb->length = fb->fix.smem_len;
	fb->fd = fd;
	return fd;
}

void fb_close(FrameBuffer *fb) {
	if(fb->fd > 0)
		close(fb->fd);
	if(fb->start > 0){
		msync(fb->start, fb->length, MS_INVALIDATE | MS_SYNC);
		munmap(fb->start, fb->length);
	}
}
