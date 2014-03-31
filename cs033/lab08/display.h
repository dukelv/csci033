#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#define UN_MAXLEN 11
#define CMD_MAXLEN 20
#define NUM_MAXLEN 5
#define TIME_LEN 8
#define Y_OFFSET 4

// process struct - represents a process
typedef struct process {
	int pid;
	int gpid;
	unsigned long int vsize;
	char state;
	char time[TIME_LEN + 1];
	char user[UN_MAXLEN];
	char cmd[CMD_MAXLEN];
} process_t;

void display_init(void); // sets up ncurses, and installs signal handler
void display_finish(void); // clean everything up
void display_data(const process_t* data, int num); // display the processes
void display_scroll_down(void);
void display_scroll_up(void);
void display_to_top(void);

#endif
