#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "io_functions.h"

/* Write all bytes in the buffer
 * Returns 0 on success, -1 on error
 */
int send_bytes(int fd, const char *buf, int len) {
	while(len > 0) {
		errno = 0;
		int result = write(fd, buf, len);
		if(result==-1) {
			if(errno != EINTR)
				return -1;
		} else {
			len -= result;
			buf += result;
		}
	}

	return 0;
}

/* Read until len bytes have been received, or an error or EOF occurs
 * Return number of bytes received
 */
int receive_bytes(int fd, char *buf, int len) {
	int rcvd = 0;

	while(len > 0) {
		errno = 0;
		int result = read(fd, buf, len);
		if(result==-1) {
			if(errno != EINTR)
				break;
		} else if(result==0) {
			break;
		} else {
			len -= result;
			buf += result;
			rcvd += result;
		}
	}

	return rcvd;
}
