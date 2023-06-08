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
#include <assert.h>
// #include <sys/mman.h>

#include "videodev2.h"
#include "hdmi_api.h"
#include "hdmi_lib.h"
#include "s3c_lcd.h"

#include "draw.h"
#include "line_container.h"
#include "util.h"

#define FB_DEV	"/dev/fb0"

static const char* FILE_NAME;
static FrameBuffer framebuffer;
static Binder bottom_bar_binder;
static Binder main_binder;

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
int display_init(FrameBuffer* gfb) {
	int ret;
	unsigned int phyLCDAddr = 0;

	printf("Font Test Program Start\n");

	ret = fb_open(FB_DEV, gfb);
	if(ret < 0){
		printf("Framebuffer open error");
		perror("");
		return 0;
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

	return 1;
}

static
void display_deinit(FrameBuffer* gfb) {
	hdmi_gl_streamoff(0);
	hdmi_gl_deinitialize(0);
	hdmi_deinitialize();

	fb_close(gfb);

}

static
void init_binder() {
	assert(binder_init(&main_binder, &framebuffer, 0, 0, 0, 0));
	size_t framebuffer_height = binder_get_height(&main_binder);
	assert(binder_set_height(&main_binder, framebuffer_height - TEXTER_DEFAULT_FONT_HEIGHT * 2));

	assert(binder_init(&bottom_bar_binder, &framebuffer, 0,
		framebuffer_height - TEXTER_DEFAULT_FONT_HEIGHT, 0, TEXTER_DEFAULT_FONT_HEIGHT));
}

static
void print_bottom_bar(int percentage) {
	Texter texter;
	assert(texter_init(&texter, &bottom_bar_binder));
	assert(binder_fill(&bottom_bar_binder, TEXTER_DEFAULT_BACKGROUND_COLOR));

	texter_printf(&texter, "file name: %s    up: w    down: s    quit: q    %3d%%",
		FILE_NAME, percentage);

	Painter painter;
	assert(painter_init(&painter, &bottom_bar_binder));
	painter_draw_line(&painter, 0, 0, binder_get_width(&bottom_bar_binder) - 1, 0, 0xFFFFFF00);
}

static
void print_lines(const Line* first, const Line* last, size_t start_num) {
	Texter texter;
	assert(texter_init(&texter, &main_binder));
	assert(binder_fill(&main_binder, TEXTER_DEFAULT_BACKGROUND_COLOR));

	char buf[32];
	const Line* it;
	for (it = first; it < last; ++it) {
		sprintf(buf, "%5zd: ", start_num++);
		texter_set_font_color(&texter, 0xFF00FFFF);
		texter_puts(&texter, buf);
		texter_set_font_color(&texter, TEXTER_DEFAULT_FONT_COLOR);
		texter_write(&texter, *it, cvector_size(*it));
		if (!texter_putc(&texter, '\n')) break;
	}
}

static
void event_loop(LineContainer container) {
	size_t start_num = 1;

	print_lines(cvector_begin(container), cvector_end(container), start_num);
	print_bottom_bar(start_num * 100 / cvector_size(container));

	int quit_flag = 0;

	printf("'q' is Quit\n");
	while (!quit_flag) {
		usleep(10*1000);
		if (!kbhit()) continue;

		int update_flag = 0;
		int c = getchar();

		switch ( c ) {
			case 'q':
				quit_flag = 1;
				break;
			case 's':
				start_num = _min(start_num + 1, cvector_size(container));
				update_flag = 1;
				break;
			case 'w':
				start_num = _max(start_num - 1, 1);
				update_flag = 1;
				break;
			default:
				printf("!%d\n", (int)c);
		}

		if (update_flag) {
			Line* first = cvector_begin(container) + start_num - 1;
			Line* last = cvector_end(container);
			print_lines(first, last, start_num);
			print_bottom_bar(start_num * 100 / cvector_size(container));
		}
	}
}

static
int run(LineContainer container) {
	assert(display_init(&framebuffer));
	init_binder();

	event_loop(container);

	display_deinit(&framebuffer);
	return 1;
}

int main(int argc, const char** argv) {
	FILE* file = NULL;

	if (argc == 2) {
		file = fopen(argv[1], "a+");
		if (!file) {
			perror("fopen");
			return 1;
		}
		FILE_NAME = argv[1];
	} else if (argc != 1) {
		fputs("invalid arguments", stderr);
		return EINVAL;
	}

	LineContainer container = NULL;
	if (file) container = line_container_from_file(file);
	if (!container) return 1;

	if (!run(container)) {
		fputs("some error occured", stderr);
		return 1;
	}

	line_container_destroy(container);

	fclose(file);

	return 0;
}


