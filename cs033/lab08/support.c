#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "support.h"




typedef int (*comparator_t)(const void *, const void *);

typedef char *(*extract_str_t)(const void *);
typedef char (*extract_char_t)(const void *);
typedef unsigned long int (*extract_uli_t)(const void *);
typedef int (*extract_int_t)(const void *);

extract_str_t extract_str;
extract_char_t extract_char;
extract_uli_t extract_uli;
extract_int_t extract_int;

comparator_t comparator = NULL;

char *extract_user(const void *x) {
    return ((process_t *)x)->user;
}

char *extract_cmd(const void *x) {
    return ((process_t *)x)->cmd;
}

char *extract_time(const void *x) {
    return ((process_t *)x)->time;
}

char extract_state(const void *x) {
    return ((process_t *)x)->state;
}

unsigned long int extract_vsize(const void *x) {
    return ((process_t *)x)->vsize;
}

int extract_pid(const void *x) {
    return ((process_t *)x)->pid;
}

int extract_gpid(const void *x) {
    return ((process_t *)x)->gpid;
}

/*
-1 : a < b
0 : a == b
1 : a > b
*/
int compare_int_asc(const void *a, const void *b) {
    return extract_int(a) - extract_int(b);
}

int compare_uli_asc(const void *a, const void *b) {
    return extract_uli(a) - extract_uli(b);
}

int compare_char_asc(const void *a, const void *b) {
    return extract_char(a) - extract_char(b);
}

int compare_str_asc(const void *a, const void *b) {
    return strcmp(extract_str(a), extract_str(b));
}

int compare_int_dsc(const void *a, const void *b) {
    return -1 * compare_int_asc(a, b);
}

int compare_char_dsc(const void *a, const void *b) {
    return -1 * compare_char_asc(a, b);
}

int compare_uli_dsc(const void *a, const void *b) {
    return -1 * compare_uli_asc(a, b);
}

int compare_str_dsc(const void *a, const void *b) {
    return -1 * compare_str_asc(a, b);
}


void set_cmp(char c) {
    switch(c) {
    case 'c':
        extract_str = &extract_cmd;
        comparator = &compare_str_dsc;
        break;
    case 'C':
        extract_str = &extract_cmd;
        comparator = &compare_str_asc;
        break;
    case 'g':
        extract_int = &extract_gpid;
        comparator = &compare_int_dsc;
        break;
    case 'G':
        extract_int = &extract_gpid;
        comparator = &compare_int_asc;
        break;
    case 'p':
        extract_int = &extract_pid;
        comparator = &compare_int_dsc;
        break;
    case 'P':
        extract_int = &extract_pid;
        comparator = &compare_int_asc;
        break;
    case 'r':
        extract_str = &extract_time;
        comparator = &compare_str_dsc;
        break;
    case 'R':
        extract_str = &extract_time;
        comparator = &compare_str_asc;
        break;
    case 's':
        extract_char = &extract_state;
        comparator = &compare_char_dsc;
        break;
    case 'S':
        extract_char = &extract_state;
        comparator = &compare_char_asc;
        break;
    case 'u':
        extract_str = &extract_user;
        comparator = &compare_str_dsc;
        break;
    case 'U':
        extract_str = &extract_user;
        comparator = &compare_str_asc;
        break;
    case 'v':
        extract_uli = &extract_vsize;
        comparator = &compare_uli_dsc;
        break;
    case 'V':
        extract_uli = &extract_vsize;
        comparator = &compare_uli_asc;
        break;
    }
}

void print_proc_info(process_t *p) {
    printf("pid = %d, gpid = %d, vsize = %lu, state = %c, "
           "time = %s, user = %s, cmd = %s\n",
           p->pid, p->gpid, p->vsize, p->state,
           p->time, p->user, p->cmd);
}


void set_process_time_string(process_t *proc, unsigned long int utime,
                             unsigned long int stime) {
    time_t seconds = (utime + stime) / sysconf(_SC_CLK_TCK);
    struct tm *time_struct = gmtime(&seconds);
    strftime(&(proc->time[0]), TIME_LEN, "%H:%M:%S", time_struct);
    proc->time[TIME_LEN] = '\0';
}

int isInt(char *str) {
    int result = atoi(str);

    //if atoi returns 0 (which is the return value on error), then
    // unless str actually represents 0, return 0 to indicate an error
    if (result == 0 && strcmp(str, "0")) {
        return 0;
    }
    //otherwise return 1 to indicate success
    return 1;
}




int process_input(char *buffer, size_t n) {
    for (int i = 0; i < n; i++) {
        if (buffer[i] == 'q' || buffer[i] == 'Q') {
            return 1;
        } else if (buffer[i] == ',') {
            display_scroll_up();
        } else if (buffer[i] == '.') {
            display_scroll_down();
        }  else {
            set_cmp(buffer[i]);
            display_to_top();
        }
        return 0;
    }
    return 0;
}

void sort_data(process_t *proc_arr, size_t nmemb) {
    if (comparator != NULL) {
        qsort(proc_arr, nmemb, sizeof(process_t), comparator);
    }
}

process_t *collect_data(int *n_procs) {

    (*n_procs) = 0; //ensure we start with no processes
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        return NULL;
    }

    struct stat stat_buf;
    int status;
    status = stat("/proc", &stat_buf);
    if (status) { // if there was an error, return
        printf("error getting status of /proc");
        closedir(proc_dir);
        return NULL;
    }

    char path[1024];
    int char_loc = 0;
    strncpy(path, "/proc/", 6);
    char_loc += 6;

    int n_entries = stat_buf.st_nlink;
    process_t *proc_arr = malloc(n_entries * sizeof(process_t));


    struct dirent *dir_entry;
    int pcount = 0;
    while((dir_entry = readdir(proc_dir)) != NULL) {
        //if this entry is a directory and represents a process
        if (dir_entry->d_type == DT_DIR &&
                isInt(&(dir_entry->d_name[0]))) {

            //build path
            strncpy(path + char_loc, dir_entry->d_name, strlen(dir_entry->d_name));
            char_loc += strlen(dir_entry->d_name);
            strncpy(path + char_loc, "/stat", 5);
            char_loc += 5;
            path[char_loc] = '\0';

            FILE *f = fopen(path, "r");
            if (! f) {
                break;
            }

            //get user of this process
            status = stat(path, &stat_buf);
            if (status) { //if there was an error, return
                return NULL;
            }

            process_t *p = &(proc_arr[pcount]);

            struct passwd *user_info = getpwuid(stat_buf.st_uid);
            int k = 0;
            for (; k < strlen(user_info->pw_name) && k < UN_MAXLEN - 1; k++) {
                p->user[k] = user_info->pw_name[k];
            }
            p->user[k] = '\0';

            p->pid = atoi(dir_entry->d_name);

            int dummy;
            unsigned long int utime;
            unsigned long int stime;
            char cmd[CMD_MAXLEN + 2];
            int fscanf_result = fscanf(f, "%d %s %c %d %d %d %d %d %d %d %d %d"
                                       "%d %lu %lu %d %d %d %d %d %d %d %lu",
                                       &dummy, cmd, &(p->state),
                                       &dummy, &(p->gpid), &dummy, &dummy,
                                       &dummy, &dummy, &dummy, &dummy, &dummy,
                                       &dummy, &utime, &stime, &dummy,
                                       &dummy, &dummy, &dummy, &dummy, &dummy,
                                       &dummy, &(p->vsize));

            if (fscanf_result == EOF) {
                printf("fscanf hit end of file!\n");
                break;
            }

            //copy proc name sans parentheses
            size_t len = strlen(cmd) - 1;
            int j = 1;
            for ( ; j < len && j < CMD_MAXLEN; j++) {
                p->cmd[j-1] = cmd[j];
            }
            p->cmd[j] = '\0';

            set_process_time_string(p, utime, stime);

            //reset char_loc
            char_loc = 6;

            fclose(f);
            pcount ++;
        }
    }

    closedir(proc_dir);

    *n_procs = pcount;
    return proc_arr;
}
