/**
 *	\file		sprite.h
 *	\brief		Fichier en-tête pour les sprites
 *	\author		ARCELON Louis
 *	\date		30 janvier 2026
 *	\version	1.0
 */
#ifndef VOID_SPRITE_H
#define VOID_SPRITE_H

#include <stdio.h>
#include <ncurses.h>


#define MAX_SPRITE_WIDTH 512

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#define VO_LEFT 0
#define VO_CENTER 1
#define VO_RIGHT 2


typedef struct {

	char *fp;
	short height;
	short width;

} sprite_t;


int initSprite(sprite_t *sprite, char *fp);

int measureSprite(sprite_t *sprite);

void wdrawSprite(WINDOW *win, sprite_t *sprite, int y, int x, int flag);

static inline void drawSprite(sprite_t *sprite, int y, int x, int flag) {
	wdrawSprite(stdscr, sprite, y, x, flag);
}

#endif /* VOID_SPRITE_H */