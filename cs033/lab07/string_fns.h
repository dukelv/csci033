#ifndef STRING_FNS_H
#define STRING_FNS_H

#include <stdio.h>

/* The following function signatures will be used for
 * sets containing strings, where all void * pointers
 * are really char *s.
 */

/* Get a hashcode for the given string */
int string_hashcode(void *arg);

/* Test two strings for equality */
int string_equals(void *arg1, void *arg2);

/* Create a dynamically-allocated copy of the
 * given string, and return a pointer to it */
void *string_copy(void *arg);

/* Free the given string */
void string_delete(void *arg);

/* Print a representation of the given string
 * to the given stream */
void string_print(FILE* stream, void *arg);

#endif
