#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <form.h>
#include "sprite.h"

// ne pas changer sauf en cas d'ajout manuel de valeurs dans les tableaux
// ci dessous. C'est une solution inélégante, mais par manque de temps et
// connaissances, j'y suis contraint.
#define FIELD_NUMBER 3
#define FIELD_Y_SEP 2

void listenInputs(FORM *);


int main()
{	
	FORM  *my_form;
	FIELD *field[FIELD_NUMBER + 1];

	sprite_t logo;
	int startx, starty;

	int fieldWidths[FIELD_NUMBER] = {
		15, 5, 10
	};
	char *labels[FIELD_NUMBER] = {
		"IP      ",
		"Port              ",
		"Pseudo       "
	};
	
	
	if (initSprite(&logo, RESOURCE_DIR "/logo.txt") == -1) {
		printf("Couldn't load sprite: logo.txt\n");
		return -1;
	};

	
	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);


	startx = COLS / 2;
	starty = LINES * 0.5;

	/* Set field options */
	for (int i = 0; i < FIELD_NUMBER; i++) {

		int x = startx - (fieldWidths[i] - strlen(labels[i])) / 2;

		field[i] = new_field(
			1, fieldWidths[i]
			, starty + i * FIELD_Y_SEP, x
			, 0, 0
		);

		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_AUTOSKIP);

	}

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	drawSprite(&logo, LINES * 0.05, COLS/2, VO_CENTER);
	
	for (int i = 0; i < FIELD_NUMBER; i++) {

		int x = startx - (fieldWidths[i] + strlen(labels[i])) / 2;
		
		mvprintw(
			starty + i * FIELD_Y_SEP
			, x
			, labels[i]
		);
	
	}
	
	refresh();

	listenInputs(my_form);

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 
	free_field(field[2]); 

	endwin();
	return 0;
}


void listenInputs(FORM *form) {

	int ch;

	/* Loop through to get user requests */
	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_DOWN:
				/* Go to next field */
				form_driver(form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_LEFT:
				form_driver(form, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(form, REQ_NEXT_CHAR);
				break;
			case KEY_BACKSPACE:
				form_driver(form, REQ_PREV_CHAR);
				form_driver(form, REQ_DEL_CHAR);
				break;
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(form, ch);
				break;
		}
	}

}