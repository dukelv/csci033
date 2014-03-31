#ifndef WINDOW_H
#define WINDOW_H

#include <sys/types.h>

typedef struct Window Window;

/* Creates a window with the given title
 *
 * NOTE: This is not thread-safe and should only be
 * called from your main thread!
 *
 * char *title - The window title
 * char *script - The script to run in the window
 *                If this argument is NULL, the
 *                window will accept input from stdin.
 */
Window *window_new(char *title, char *script);

/* Deletes the given window */
void window_delete(Window *window);

/* Gets a command from the given window and stores it
 * in the given buffer.
 * Returns 1 on success, 0 on EOF or error
 */
int get_command(Window *window, char *command);

/* Sends the given response string to the given window
 * Returns 1 on success, 0 on error
 */
int send_response(Window *window, char *response);

#endif
