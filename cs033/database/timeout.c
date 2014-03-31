#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

#include "timeout.h"

struct Timeout {
	pthread_mutex_t mutex;  // Lock for the struct
	pthread_cond_t cond;  // Condition variable

	int wait_s;  // Timeout interval in seconds

	struct timeval deadline;  // Absolute time at which client should time out
	int active;  // Whether the timeout is currently active

	void (*timeout_function)(void *);  // Function to call on timeout
	void *timeout_arg;  // Argument to pass to timeout function

	pthread_t thread;  // Thread maintaining the timeout
};

static void *watchdog_function(void *arg);

/* Create a new Timeout */
Timeout *timeout_new(int wait_s, void (*timeout_function)(void *), void *timeout_arg) {
	Timeout *timeout = (Timeout *)malloc(sizeof(Timeout));

	if(!timeout)
		return NULL;

	pthread_mutex_init(&(timeout->mutex), NULL);
	pthread_cond_init(&(timeout->cond), NULL);

	timeout->active = 0;

	timeout->wait_s = wait_s;

	timeout->timeout_function = timeout_function;
	timeout->timeout_arg = timeout_arg;

	pthread_create(&(timeout->thread), NULL, &watchdog_function, timeout);

	return timeout;
}

/* Delete a Timeout */
void timeout_delete(Timeout *timeout) {
	int oldstate;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);  //Disable cancellation in calling thread while joining timeout thread

	pthread_cancel(timeout->thread);
	pthread_join(timeout->thread, NULL);
	pthread_cond_destroy(&(timeout->cond));
	pthread_mutex_destroy(&(timeout->mutex));
	free(timeout);

	pthread_setcancelstate(oldstate, NULL);  // Restore old cancel state
}

/* Reset the deadline and activate the timeout */
void timeout_activate(Timeout *timeout) {
	struct timeval ndeadline;
	gettimeofday(&ndeadline, NULL);
	ndeadline.tv_sec += timeout->wait_s;

	pthread_mutex_lock(&(timeout->mutex));
	timeout->deadline = ndeadline;
	if(!timeout->active) {
		timeout->active = 1;
		pthread_cond_broadcast(&(timeout->cond));
	}
	pthread_mutex_unlock(&(timeout->mutex));
}

/* Deactivates the timeout */
void timeout_deactivate(Timeout *timeout) {
	pthread_mutex_lock(&(timeout->mutex));
	timeout->active = 0;
	pthread_mutex_unlock(&(timeout->mutex));
}

/* Cleanup wrapper for pthread_mutex_unlock() */
static void cw_pthread_mutex_unlock(void *arg) {
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}

/* Function to run in the timeout thread */
static void *watchdog_function(void *arg) {
	Timeout *timeout = (Timeout *)arg;

	struct timeval time_left;
	while(1) {
		pthread_mutex_lock(&(timeout->mutex));

		pthread_cleanup_push(cw_pthread_mutex_unlock, &(timeout->mutex));

		// Wait for timeout to be activated
		while(!timeout->active)
			pthread_cond_wait(&(timeout->cond), &(timeout->mutex));

		pthread_cleanup_pop(0);

		struct timeval now;
		gettimeofday(&now, NULL);

		// Check if deadline has passed
		if(timercmp(&now, &(timeout->deadline), <)) {
			timersub(&(timeout->deadline), &now, &time_left);
			pthread_mutex_unlock(&(timeout->mutex));
		} else {
			// Deadline has expired
			// Call timeout function and exit loop
			pthread_mutex_unlock(&(timeout->mutex));
			timeout->timeout_function(timeout->timeout_arg);
			break;
		}

		struct timespec wait_time;
		wait_time.tv_sec = time_left.tv_sec;
		wait_time.tv_nsec = 1000 * time_left.tv_usec;

		if(nanosleep(&wait_time, NULL) == -1 && errno != EINTR) {
			perror("nanosleep");
			break;
		}
	}

	return NULL;
}
