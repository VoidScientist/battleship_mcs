#include <ncurses.h>
#include <logging.h>
#include "sprite.h"


#define drawSprite(sprite, y, x, flag) \
	wdrawSprite(stdscr, &sprite, y, x, flag);


int measureSprite(sprite_t *sprite) {

	FILE *fd;

	int p;
	short lineWidth = 0;
	short width = 0;
	short height = 0;

	fd = fopen(sprite->fp, "r");

	if (fd == NULL) {
		logMessage("An error occurred while opening sprite file.\n", ERROR);
		perror("fopen(): ");
		return -1;
	}

	while ( ( p = fgetc(fd) ) != EOF) {

		if (p == '\n') {
			if (lineWidth > width) {
				width = lineWidth;
			}
			lineWidth = 0;
			height++;
			continue;
		}

		lineWidth++;

	}

	sprite->width = width;
	sprite->height = height;

	fclose(fd);

	return 0;

}

int initSprite(sprite_t *sprite, char *fp) {

	sprite->fp = fp;

	if (measureSprite(sprite) == -1) 
		return -1;

	return 0;

}


void wdrawSprite(WINDOW *win, sprite_t *sprite, int y, int x, int flag) {

	FILE * fd;
	char line[MAX_SPRITE_WIDTH];
	int startX, startY;

	switch (flag) {


		case VO_RIGHT:
			startX = x;
			break;

		case VO_CENTER:
			startX = x - (sprite->width) / 2;
			break;

		case VO_LEFT:
		default:
			startX = x - sprite->width;
			break;

	}

	if (startX < 0) 
		startX = 0;

	startY = y;

	fd = fopen(sprite->fp, "r");

	while (! feof(fd) ) {
		fgets(line, 512, fd);
		mvprintw(startY++, startX, line);
	}

	fclose(fd);

}