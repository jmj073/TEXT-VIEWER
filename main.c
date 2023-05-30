#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <sys/mman.h>

#include "videodev2.h"
#include "hdmi_api.h"
#include "hdmi_lib.h"
#include "s3c_lcd.h"

#include "draw.h"

#define FB_DEV	"/dev/fb0"

// 키보드 이벤트를 처리하기 위한 함수, Non-Blocking 입력을 지원
//  값이 없으면 0을 있으면 해당 Char값을 리턴
static int kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

static
int init(FrameBuffer* gfb) {
	int ret;
	unsigned int phyLCDAddr = 0;

	printf("Font Test Program Start\n");

	ret = fb_open(FB_DEV, gfb);
	if(ret < 0){
		printf("Framebuffer open error");
		perror("");
		return -1;
	}

	// get physical framebuffer address for LCD
	if (ioctl(ret, S3CFB_GET_LCD_ADDR, &phyLCDAddr) == -1)
	{
		printf("%s:ioctl(S3CFB_GET_LCD_ADDR) fail\n", __func__);
		return 0;
	}
	printf("phyLCD:%x\n", phyLCDAddr);

	hdmi_initialize();

	hdmi_gl_initialize(0);
	hdmi_gl_set_param(0, phyLCDAddr, 1280, 720, 0, 0, 0, 0, 1);
	hdmi_gl_streamon(0);

	unsigned int *pos = (unsigned int*)gfb->start;

	//Clear Screen(Black)
	memset(pos, 0x00, 1280*720*4);

	return 0;
}

static
void loop() {
	int endFlag = 0;
	printf("'q' is Quit\n");
	while (!endFlag) {
		usleep(10*1000);
		if (kbhit()) {
			switch ( getchar() ) {
				case 'q': endFlag = 1;
					break;
			}
		}
	}
}

static
void deinit(FrameBuffer* gfb) {
	hdmi_gl_streamoff(0);
	hdmi_gl_deinitialize(0);
	hdmi_deinitialize();

	fb_close(gfb);

}

int main() {
	Painter painter;
	FrameBuffer gfb;

	init(&gfb);
	painter_init(&painter, &gfb);
	painter_draw_ract(&painter, 50, 50, 100, 100, 0xFFFFFFFF);
	loop();
	deinit(&gfb);

	return 0;
}


