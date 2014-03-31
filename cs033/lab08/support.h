#ifndef _SUPPORT_H_
#define _SUPPORt_H_

#include "display.h"

#define ASC 1
#define DSC 0

int process_input(char *buffer, size_t n);
process_t *collect_data(int *n_procs);
void sort_data(process_t *proc_arr, size_t nmemb);

#endif
