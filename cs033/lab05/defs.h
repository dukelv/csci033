#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdlib.h>

#define RIDX(i,j,n) ((i)*(n)+(j))

typedef struct {
   unsigned short red;
   unsigned short green;
   unsigned short blue;
} pixel;

typedef void (*lab_test_func) (int, pixel*, pixel*);
typedef void (*lab_test_func_string) (char*);

void invert(int, pixel *, pixel *);
void rotate(int, pixel *, pixel *);
void reverse(char* string);

void register_rotate_functions(void);
void register_invert_functions(void);
void register_reverse_functions(void);
void add_invert_function(lab_test_func, char*);
void add_rotate_function(lab_test_func, char*);
void add_reverse_function(lab_test_func_string, char*);

#endif /* _DEFS_H_ */

