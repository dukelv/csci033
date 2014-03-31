#ifndef TIMEOUT_H
#define TIMEOUT_H

typedef struct Timeout Timeout;

/* Create a new Timeout struct
 *
 * Launches a new thread to handle the timeout.
 * This thread will sleep until the timeout is activated
 * by a call to timeout_activate().
 *
 * Arguments:
 *   wait_s           - The duration of the timeout, in seconds
 *   timeout_function - A function to call when the timeout expires
 *   timeout_arg      - An argument to be passed to the timeout function
 */
Timeout *timeout_new(int wait_s, void (*timeout_function)(void *), void *timeout_arg);

/* Delete a Timeout struct
 *
 * Terminates the timeout thread and destroys the timeout struct
 */
void timeout_delete(Timeout *timeout);

/* Reset the deadline and activate the timeout */
void timeout_activate(Timeout *timeout);

/* Deactivate the timeout
 *
 * The timeout can be reactivated by another call to timeout_activate()
 */
void timeout_deactivate(Timeout *timeout);

#endif
