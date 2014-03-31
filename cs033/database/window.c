#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "window.h"
#include "io_functions.h"

#define INTERFACE_EXEC "./interface"

struct Window {
	pid_t xpid;  /* PID of xterm process */
	pid_t ipid;  /* PID of interface */

	int cs_fd;  /* Client-to-server file descriptor */
	int sc_fd;  /* Server-to-client file descriptor */
};

/* Creates a window with the given title
 * If script is not NULL, the corresponding file
 * will be used as input.  Otherwise, the window
 * will accept user input from stdin.
 */
Window *window_new(char *title, char *script) {
	static long long uid = -1;
	static int window_count = 0;

	if(uid == -1) {
		uid = (long long) time(NULL);
		if(uid == -1) {
			perror("time");
			return NULL;
		}
	}

	Window *window = (Window *)malloc(sizeof(Window));

	if(!window) {
		perror("malloc");
		return NULL;
	}

	if(window_count >= 1000000) {  // Not likely to occur
		fprintf(stderr, "Could not create FIFO name\n");
		free(window);
		return NULL;
	}

	// Generate unique-ish file names
	// 32 = 9 for "/tmp/xxxx" + 16 digits for id + 6 digits for count + 1
	char cs_name[32];
	char sc_name[32];
	sprintf(cs_name, "/tmp/dbcs%016llx%6d", uid, window_count);
	sprintf(sc_name, "/tmp/dbsc%016llx%6d", uid, window_count);

	window_count++;

	// Create FIFOs
	unlink(cs_name);
	if(mkfifo(cs_name, 0600)==-1) {
		perror("mkfifo");
		free(window);
		return NULL;
	}

	unlink(sc_name);
	if(mkfifo(sc_name, 0600)==-1) {
		perror("mkfifo");
		free(window);
		return NULL;
	}

	// Create child process to run xterm
	pid_t pid = fork();
	if(pid==-1) {
		perror("fork");
		free(window);
		return NULL;
	} else if(pid==0) {
		// Launch xterm in child
		char *args[] = {"/usr/bin/xterm", "-T", title, "-n", title, "-ut",
					 "-geometry", "35x20", "-e", INTERFACE_EXEC, sc_name,
					 cs_name, script, NULL};
		execv(args[0], args);

		// The execv() call failed.  Take evasive action.
		perror("execv");

		int sc_fd = open(sc_name, O_RDONLY);
		int cs_fd = open(cs_name, O_WRONLY);

		pid_t fpid = 0;
		send_bytes(cs_fd, (char *)(&fpid), sizeof(pid_t));

		close(sc_fd);
		close(cs_fd);

		exit(1);
	} else {
		window->xpid = pid;

		// Open FIFOs (this will block until both ends are open)
		int sc_fd = open(sc_name, O_WRONLY);
		int cs_fd = open(cs_name, O_RDONLY);

		// Unlink FIFOs so they will be deleted on exit
		unlink(sc_name);
		unlink(cs_name);

		if (sc_fd == -1 || cs_fd == -1) {
			perror("open");

			// Terminate child process
			kill(pid, SIGTERM);
			waitpid(pid, NULL, 0);

			// Close FIFOs if open
			if(sc_fd != -1)
				close(sc_fd);
			if(cs_fd != -1)
				close(cs_fd);

			free(window);
			return NULL;
		} else {
			window->sc_fd = sc_fd;
			window->cs_fd = cs_fd;
		}

		// Get interface PID
		if(receive_bytes(cs_fd, (char *)(&(window->ipid)), sizeof(pid_t)) != sizeof(pid_t)) {
			if(errno)
				perror("receive_bytes");
			else
				fprintf(stderr, "receive_bytes: Unexpected EOF\n");

			// Terminate child process
			kill(pid, SIGTERM);
			waitpid(pid, NULL, 0);

			// Close FIFOs if open
			close(sc_fd);
			close(cs_fd);

			free(window);
			return NULL;
		}

		if(!window->ipid) {
			fprintf(stderr, "Child process failed\n");

			// Wait for child process to terminate
			waitpid(pid, NULL, 0);

			// Close FIFOs if open
			close(sc_fd);
			close(cs_fd);

			free(window);
			return NULL;
		}
	}

	return window;
}

/* Deletes the given window */
void window_delete(Window *window) {
	kill(window->ipid, SIGTERM);  // Terminate interface
	waitpid(window->xpid, NULL, 0);  // Reap child process
	close(window->cs_fd);  // Close client-server FIFO
	close(window->sc_fd);  // Close server-client FIFO
	free(window);  // Free struct
}

/* Gets a command from the given window and stores it
 * in the given buffer.
 * Returns 1 on success, 0 on EOF or error
 */
int get_command(Window *window, char *command) {
	int clen;
	if(receive_bytes(window->cs_fd, (char *)&clen, sizeof(int)) != sizeof(int)) {
		if(errno)
			perror("receive_bytes");
		else
			fprintf(stderr, "receive_bytes: Unexpected EOF\n");

		return 0;
	}

	if(clen == 0)  // EOF
		return 0;

	if(receive_bytes(window->cs_fd, command, clen) != clen) {
		if(errno)
			perror("receive_bytes");
		else
			fprintf(stderr, "receive_bytes: Unexpected EOF\n");

		return 0;
	}

	return 1; // Success
}

/* Sends the given response string to the given window
 * Returns 1 on success, 0 on error
 */
int send_response(Window *window, char *response) {
	int rlen = strlen(response)+1;

	if(send_bytes(window->sc_fd, (char *)&rlen, sizeof(int))) {
		perror("send_bytes");
		return 0;
	}

	if(send_bytes(window->sc_fd, response, rlen)) {
		perror("send_bytes");
		return 0;
	}

	return 1;
}
