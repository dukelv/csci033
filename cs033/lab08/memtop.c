#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "display.h"
#include "support.h"

#define BUFFER_SIZE 256


void run_memtop(void) {

    //TODO: initialization for read and select
    fd_set *readfs = malloc(sizeof(fd_set));
    struct timeval *timeout = malloc(sizeof(struct timeval));
    char buf[BUFFER_SIZE];
    //to avoid unused variable warning. You may remove it once you use buf.
    buf[0] = 0;
    while(1) {
        //TODO: call select, read, and process_data
	FD_ZERO(readfs);
	FD_SET(STDIN_FILENO,readfs);
	timeout->tv_sec = 0;
	timeout->tv_usec = 0;
	int s = select(1, readfs, NULL, NULL, timeout);
	if(s==-1)
	{
	    puts("ERROR 1");
	    continue;
	}
	int f = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	if(f==-1)
	{
	    puts("ERROR 2");
	    continue;
	}
	if(FD_ISSET(STDIN_FILENO, readfs))
	{
	    int r = read(STDIN_FILENO, buf, BUFFER_SIZE);
	    if(r==-1)
	    {
		//puts("ERROR 3");
		continue;
	    }
	    int p = process_input(buf,r);
	    if(p!=0)
	    {
		puts("ERROR 3");
		break;
	    }
	}
        //read proc filesystem
        int n_procs = 0;
        process_t *proc_arr = collect_data(&n_procs);

        if (proc_arr == NULL) {
            continue;
        }

        sort_data(proc_arr, n_procs);
        //update display
        display_data(proc_arr, n_procs);
                //free proc_data
        free(proc_arr);


    }
    free(timeout);
    free(readfs);
}


int main(int argc, char *argv[]) {
    display_init();
    run_memtop();
    display_finish();
    return 0;
}

