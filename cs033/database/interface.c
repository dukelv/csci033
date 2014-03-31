/* Program to run in an xterm window to interact with the capital cities
 * database program.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "io_functions.h"

int main(int argc, char **argv) {
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "Usage: %s infile outfile [script] \n", argv[0]);
		exit(1);
	}

	// Ignore SIGPIPE
	struct sigaction ign;
	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &ign, NULL);

	// Open FIFOs
	int sc_fd;
	int cs_fd;

	if((sc_fd = open(argv[1], O_RDONLY)) == -1) {
		perror("open");
		sleep(5);
		exit(1);
	}

	if((cs_fd = open(argv[2], O_WRONLY)) == -1) {
		perror("open");
		sleep(5);
		exit(1);
	}

	// Unblock SIGINT
	sigset_t sigint;
	sigemptyset(&sigint);
	sigaddset(&sigint, SIGINT);
	sigprocmask(SIG_UNBLOCK, &sigint, NULL);

	// Open script file, if applicable
	FILE *infile;
	if(argc==4) {
		if(!(infile = fopen(argv[3], "r"))) {
			fprintf(stderr, "Could not open script file %s: %s\n",
				   argv[3], strerror(errno));
			sleep(5);
			exit(1);
		}
	} else {
		infile = stdin;
	}
	int infile_fd = fileno(infile);

	// Send PID
	pid_t pid = getpid();
	if(send_bytes(cs_fd, (char *)&pid, sizeof(pid_t))==-1) {
		perror("send_bytes");
		sleep(5);
		exit(1);
	}

	char buffer[BUF_LEN];
	int buf_len;
	int result;
	
	// Alternate between reading input and sending it,
	// and receiving responses and printing them
	while(1) {
		printf(">> ");
		fflush(stdout);

		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(infile_fd, &rset);
		result = select(infile_fd+1, &rset, NULL, NULL, NULL);

		if(result==-1) {
			perror("select");
			sleep(5);
			exit(1);
		}

		// Read a line of input
		char *r = fgets(buffer, BUF_LEN, infile);
		
		if(!r)
			break;

		// Remove trailing newline
		buf_len = strlen(buffer);
		if(buf_len && buffer[buf_len-1]=='\n')
			buffer[buf_len-1] = 0;
		else
			buf_len++;

		// If reading from file, echo input
		if(infile_fd)
			printf("%s\n", buffer);

		// Send size of message
		if(send_bytes(cs_fd, (char *)&buf_len, sizeof(int))) {
			perror("send_bytes");
			sleep(5);
			exit(1);
		}

		// Send message contents
		if(send_bytes(cs_fd, buffer, buf_len)) {
			perror("send_bytes");
			sleep(5);
			exit(1);
		}

		// Receive size of response
		if(receive_bytes(sc_fd, (char *)&buf_len, sizeof(int)) != sizeof(int)) {
			if(errno)
				perror("receive_bytes");
			else
				fprintf(stderr, "receive_bytes: Unexpected EOF");

			sleep(5);
			exit(1);
		}

		// Receive response contents
		if(receive_bytes(sc_fd, buffer, buf_len) != buf_len) {
			if(errno)
				perror("receive_bytes");
			else
				fprintf(stderr, "receive_bytes: Unexpected EOF");

			sleep(5);
			exit(1);
		}

		// Output response
		printf("%.*s\n", buf_len, buffer);
	}

	if(feof(infile)) {
		// Send 0 as message length to signal EOF
		buf_len = 0;
		if(send_bytes(cs_fd, (char *)&buf_len, sizeof(int)) == -1) {
			perror("send_bytes");
			sleep(5);
			exit(1);
		}
	} else {
		perror("fgets");
		sleep(5);
		exit(1);
	}

	printf("\nGoodbye\n");

	// Sleep until SIGTERM or window is closed
	while(1) {
		pause();
	}

	return 0;
}
