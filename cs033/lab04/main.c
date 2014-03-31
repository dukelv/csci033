#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "fibs.h"

//Forward declarations
void print_usage(FILE *stream, char *name);
int parse_unsigned_int(char *str, unsigned int *rptr);
void test_function(const char *name, void (*fptr)(), unsigned int argsize, void *args);

int main(int argc, char **argv) {
	srand(time(NULL));

	if(argc == 1) {
		fprintf(stderr, "Usage: %s <n> ...\n", argv[0]);
		return 1;
	}

	int arg;
	for(arg = 1; arg<argc; arg++) {
		unsigned int n;
		if(!parse_unsigned_int(argv[arg], &n)) {
			fprintf(stderr, "Could not parse argument \"%s\"\n", argv[arg]);
			return 1;
		}

		if(n > 47) {  //Overflow occurs starting at n=48
			fprintf(stderr, "n = %u: Out of range\n", n);
			continue;
		}

		printf("n = %u\n", n);
		test_function("fibs_recursive", (void (*)())&fibs_recursive, 1, (void*)(&n));
	}

	return 0;
}

/* Test the given function
 *
 * name    - A string representation of the function
 * fptr    - A function pointer
 * argsize - The number of words in the argument array
 * args    - A pointer to the start of an array of words to be copied onto the stack as arguments
 */
void test_function(const char *name, void (*fptr)(), unsigned int argsize, void *args) {
	pid_t pid = fork();
	if(!pid) {
		printf("%s: ", name);
		fflush(stdout);

		static unsigned int canary; /* Stack corruption test value */
		static unsigned int osp[2]; /* Array for saved stack pointers */
		static unsigned int reg[5]; /* Array for saved registers */

		/* Initialize test values */
		canary = (unsigned int) rand();

		int i;
		for(i = 2; i<5; i++)
			reg[i] = (unsigned int) rand();

		int space = 4 * (32 + (rand() % 11));  /* Extra space to protect the stack and make it unpredictable */

		static unsigned int info[6]; /* Array for various diagnostic values */

		unsigned int result; /* The value returned by the function */

		__asm__ __volatile__("movl     %6, %%eax \n\t" // Save current stack pointer to osp[0]
						 "movl     %%esp, (%%eax) \n\t"

						 "subl     %1, %%esp \n\t" // Make space on stack

						 "movl     %5, %%eax \n\t" // Push canary on stack
						 "pushl    (%%eax) \n\t"

						 "movl     %6, %%eax \n\t" // Save current stack pointer to osp[1]
						 "movl     %%esp, 0x4(%%eax) \n\t"

						 "movl     $0x0, %%ecx \n\t" // Push arguments on stack in reverse order
						 "movl     %3, %%eax \n\t"
						 "movl     %2, %%edx \n\t"
						 "imull    $0x4, %%edx \n\t"
						 "addl     %%edx, %%eax \n\t"
						 ".L0:"
						 "cmp      %2, %%ecx \n\t"
						 "jae      .L1\n\t"
						 "subl     $0x4, %%eax \n\t"
						 "pushl    (%%eax) \n\t"
						 "addl     $0x1, %%ecx \n\t"
						 "jmp      .L0 \n\t"
						 ".L1:"

						 "movl     %7, %%eax \n\t" // Prepare registers
						 "movl     %%ebp, (%%eax) \n\t" // Save %ebp to reg[0]
						 "movl     %%esp, 0x4(%%eax) \n\t" // Save %esp to reg[1]
						 "movl     0x8(%%eax), %%ebx \n\t" // Set %ebx to reg[2]
						 "movl     0xC(%%eax), %%edi \n\t" // Set %edi to reg[3]
						 "movl     0x10(%%eax), %%esi \n\t" // Set %esi to reg[4]

						 "movl	 %4, %%eax \n\t" // Call function
						 "call     *%%eax \n\t"
						 "movl     %%eax, %0 \n\t"

						 "movl     %8, %%eax \n\t" // Save current register states to info
						 "movl     %%ebp, (%%eax) \n\t"
						 "movl     %%esp, 0x4(%%eax) \n\t"
						 "movl     %%ebx, 0x8(%%eax) \n\t"
						 "movl     %%edi, 0xC(%%eax) \n\t"
						 "movl     %%esi, 0x10(%%eax) \n\t"

						 "movl     %7, %%eax \n\t" // Restore saved registers
						 "movl     (%%eax), %%ebp \n\t"
						 "movl     0x4(%%eax), %%esp \n\t"
						 "movl     0x8(%%eax), %%ebx \n\t"
						 "movl     0xC(%%eax), %%edi \n\t"
						 "movl     0x10(%%eax), %%esi \n\t"

						 "movl    %6, %%eax \n\t" // Set stack pointer to osp[1]
						 "movl    0x4(%%eax), %%esp \n\t"

						 "movl    %8, %%eax \n\t" // Save top of stack to info[5]
						 "popl    %%ecx \n\t"
						 "movl    %%ecx, 0x14(%%eax) \n\t"

						 "movl    %6, %%eax \n\t" // Set stack pointer to osp[0]
						 "movl    (%%eax), %%esp \n\t"
						 
						 : "=g" (result)  // 0
						 : "g" (space),   // 1
						   "g" (argsize), // 2
						   "g" (args),    // 3
						   "g" (fptr),    // 4
						   "i" (&canary), // 5
						   "i" (osp),     // 6
						   "i" (reg),     // 7
						   "i" (info)     // 8
						 : "eax", "ecx", "edx",
						   "ebx", "edi", "esi",
						   "cc", "memory");

		printf("%u\n", result);

		if(reg[0]!=info[0])
			printf("Warning: %%ebp corrupted (expected 0x%08x, got 0x%08x)\n", reg[0], info[0]);
		if(reg[1]!=info[1])
			printf("Warning: %%esp corrupted (expected 0x%08x, got 0x%08x)\n", reg[1], info[1]);
		if(reg[2]!=info[2])
			printf("Warning: %%ebx corrupted (expected 0x%08x, got 0x%08x)\n", reg[2], info[2]);
		if(reg[3]!=info[3])
			printf("Warning: %%edi corrupted (expected 0x%08x, got 0x%08x)\n", reg[3], info[3]);
		if(reg[4]!=info[4])
			printf("Warning: %%esi corrupted (expected 0x%08x, got 0x%08x)\n", reg[4], info[4]);
		if(canary!=info[5])
			printf("Warning: stack corrupted\n");

		fflush(stdout);
		exit(0);
	} else if(pid==-1) {
		perror("Fork failed");
	} else {
		siginfo_t info;
		waitid(P_PID, pid, &info, WEXITED); //Wait for the forked child to terminate
		if(info.si_code == CLD_KILLED) {
			if(info.si_status == SIGSEGV)
				printf("Segmentation fault\n");
			else
				printf("Process killed by signal %d\n", info.si_status);
			fflush(stdout);
		}
	}
}

int parse_unsigned_int(char *str, unsigned int *rptr) {
	char tmp;
	return (sscanf(str, " %u %c", rptr, &tmp) == 1);
}
