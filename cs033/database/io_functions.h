#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUF_LEN 256  // Maximum message length

/* Write all bytes in the buffer
 * Returns 0 on success, -1 on error
 */
int send_bytes(int fd, const char *buf, int len);

/* Read until len bytes have been received, or an error or EOF occurs
 * Return number of bytes received
 *
 * If number of bytes returned is less than specified length,
 * program can check errno to determine if problem was EOF or error
 */
int receive_bytes(int fd, char *buf, int len);

#endif
