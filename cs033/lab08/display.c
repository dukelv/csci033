/*
 * This should be the display program only for the "top" assignment.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "display.h"

#define WINDOW_ROWS 20

/*	indicates the vertical offset of the ncurses window	*/
int V_OFFSET;

/*	indicates the number of proceses currently in the display	*/
int NUM_PROCESSES;

/*	indicates the current set of process data	*/
process_t *PROCESS_DATA;


/*
 * display_init()
 *
 * Sets up the ncurses display; should be called before any other display functions.
 */
void display_init(void) {
	#ifndef DEBUG_FLAG
	
	#ifdef MOVE_STDERR
	int stderr_closed = 1;
	if (close(STDERR_FILENO)) {
		perror("error closing stderr");
		stderr_closed = 0;
	}
	if (stderr_closed)
		open("./stderr", O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	#endif

	V_OFFSET = 0;
	NUM_PROCESSES = 0;
	PROCESS_DATA = NULL;

	initscr(); // initialize the screen
	scrollok(stdscr, TRUE);
	keypad(stdscr, TRUE); // allow the keyboard

	nonl(); // necessary ncurses parameters. 
	cbreak();
	echo();

	// Colors!!
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_RED);

    // set up header strings/lines	process_t proc; // temporary variable to hold the current process struct
	attron(COLOR_PAIR(1));
	mvaddnstr(0, 0, "           Welcome to MEMTOP!             ", 41);
	mvaddnstr(1, 0, "You can enter commands on the next line:  ", 41);
	attroff(COLOR_PAIR(1));
	move(2, 0);
	refresh();

	#endif
}


/*
 * display_finish()
 * 
 * Takes down and cleans up ncurses. 
 */
void display_finish(void) {
	#ifndef DEBUG_FLAG

	delwin(stdscr);
	endwin();
	refresh();

	if (PROCESS_DATA != NULL)
		free(PROCESS_DATA);

	#endif	
	return;
}


void refresh_display() {
	#ifndef DEBUG_FLAG

	int len = (NUM_MAXLEN * 3) + TIME_LEN + UN_MAXLEN + CMD_MAXLEN + 12; // compute length of string 
	char buf[len + 1]; // declare buffer
	attron(COLOR_PAIR(2)); // color
	mvaddnstr(3, 0, "  PID  USER        GROUP   VIRT  S  RUNTIME   COMMAND  ", 55);
	attroff(COLOR_PAIR(2)); // turn color off
	char vs[NUM_MAXLEN + 1];
	long unsigned int tmp;

	int i;
	for (i = 0; i < WINDOW_ROWS; i++) {
		int index = i + V_OFFSET;

		if (PROCESS_DATA[index].vsize / 100000 > 0) {
			tmp = (PROCESS_DATA[index].vsize + 999) / 1000;
			if (tmp / 10000 > 0) {
				tmp = (tmp + 999) / 1000;
				if (tmp / 10000 > 0) {
					tmp = (tmp + 999) / 1000;
					sprintf(vs, "%lug", tmp);
				} else {
					sprintf(vs, "%lum", tmp);
				}
			} else {
				sprintf(vs, "%luk", tmp);
			}
		} else {
			sprintf(vs, "%lu", PROCESS_DATA[index].vsize);
		}

		snprintf(buf, len, "%*d  %-*s  %*d  %*s  %c  %*s  %s",  NUM_MAXLEN,
			PROCESS_DATA[index].pid, UN_MAXLEN, PROCESS_DATA[index].user, NUM_MAXLEN, PROCESS_DATA[index].gpid,
			NUM_MAXLEN, vs, PROCESS_DATA[index].state, TIME_LEN, PROCESS_DATA[index].time, PROCESS_DATA[index].cmd);
		move(i + Y_OFFSET, 0);
		clrtoeol(); // clear line
		mvaddnstr(i + Y_OFFSET, 0, buf, len); // add string
	}

	move(2, 0); // return curser to previous position
	touchwin(stdscr);
	refresh(); // repaint the window

	#endif
}


/*
 * dislpay_data()
 * Arguments:
 * const struct process_t* data - an array of structs representing processes
 * int num - the number of structs in the above array
 * 
 * Loops through an array of struct process_t's, displaying them one by one in
 * the ncurses window. This should be the only function that you need to call
 * to output data to the window; the other functions need never be called more
 * than once in your program (display_init() and display_finish()).
 */
void display_data(const process_t* data, int num) {
	#ifndef DEBUG_FLAG

	NUM_PROCESSES = num;
	if (V_OFFSET + WINDOW_ROWS > num)
		V_OFFSET = num - WINDOW_ROWS;

	if (PROCESS_DATA != NULL)
		free(PROCESS_DATA);
    PROCESS_DATA = malloc(NUM_PROCESSES*sizeof(process_t));
    memcpy(PROCESS_DATA, data, NUM_PROCESSES*sizeof(process_t));

	refresh_display();
	#endif
	return;
}

void display_scroll_down(void) {
	#ifndef DEBUG_FLAG

	if (V_OFFSET + WINDOW_ROWS < NUM_PROCESSES) {
		V_OFFSET++;
		refresh_display();
	}

	#endif
}

void display_scroll_up(void) {
	#ifndef DEBUG_FLAG

	if (V_OFFSET > 0) {
		V_OFFSET--;
		refresh_display();
	}

	#endif
}

void display_to_top(void) {
	#ifndef DEBUG_FLAG

	if (V_OFFSET > 0) {
		V_OFFSET = 0;
		refresh_display();
	}

	#endif
}
