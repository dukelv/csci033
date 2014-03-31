#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "hashset.h"
#include "string_fns.h"

#define BUF_SIZE 1024

//Forward declarations
void print_help();
char *skip_ws(char *str);
char *skip_nonws(char *str);
void next_word(char **currp, char **nextp);
int add_file(HashSet *hashset, char *file);
int remove_file(HashSet *hashset, char *curr);

int main(int argc, char **argv) {
	char buf[BUF_SIZE];

	HashSet *hashset = hashset_new(&string_hashcode, &string_equals, &string_copy, &string_delete);
	if(!hashset) {
		fprintf(stderr, "hashset_new() returned NULL\n");
		return 1;
	}

	hashset_print(hashset, stdout, &string_print);
	printf("\n\n");

	while(1) {
		printf("> ");
		fflush(stdout);

		if(fgets(buf, BUF_SIZE, stdin)==NULL)
			break;

		char *curr;
		char *next = buf;
		next_word(&curr, &next);

		if(!*curr) {
			//Blank line -- do nothing
			continue;
		} else if(!strcasecmp(curr, "a")) {
			next_word(&curr, &next);

			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				if(hashset_add(hashset, (void *)curr))
					printf("Element added\n");
				else
					printf("Element already exists\n");
				hashset_print(hashset, stdout, &string_print);
				printf("\n");
			}
		} else if(!strcasecmp(curr, "c")) {
			next_word(&curr, &next);
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				printf(hashset_contains(hashset, (void *)curr) ? "true\n" : "false\n");
			}
		} else if(!strcasecmp(curr, "r")) {
			next_word(&curr, &next);
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				if(hashset_remove(hashset, (void *)curr))
					printf("Element removed\n");
				else
					printf("Element not found\n");
				hashset_print(hashset, stdout, &string_print);
				printf("\n");
			}
		} else if(!strcasecmp(curr, "x")) {
			next_word(&curr, &next);
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				hashset_clear(hashset);
				hashset_print(hashset, stdout, &string_print);
				printf("\n");
			}
		} else if(!strcasecmp(curr, "s")) {
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				printf("%d\n", hashset_size(hashset));
			}
		} else if(!strcasecmp(curr, "p")) {
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				hashset_print(hashset, stdout, &string_print);
				printf("\n");
			}
		} else if (!strcasecmp(curr, "fa")) {
			next_word(&curr, &next);
			
			if (*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				if (add_file(hashset, curr))
					printf("error loading file.\n");
				else {
					hashset_print(hashset, stdout, &string_print);
					printf("\n");
				}
			}
		} else if (!strcasecmp(curr, "fr")) {
			next_word(&curr, &next);
			
			if (*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				if (remove_file(hashset, curr))
					printf("error loading file.\n");
				else {
					hashset_print(hashset, stdout, &string_print);
					printf("\n");
				}
			}
		} else if(!strcasecmp(curr, "quit")) {
			if(*(skip_ws(next))) {
				printf("Too many arguments\n");
			} else {
				break;
			}
		} else if(!strcasecmp(curr, "h") || !strcasecmp(curr, "help")) {
			print_help();
		} else {
			printf("Invalid command\n");
		}

		printf("\n");
	}
	
	if(ferror(stdin))
		perror("fgets() failed");

	hashset_delete(hashset);

	return 0;
}

/* Add contents of file to hashset */
int add_file(HashSet *hashset, char *curr) {
	FILE *f = fopen(curr, "r");
	if(!f) {
		fprintf(stderr, "error opening file %s: %s\n", curr, strerror(errno));
		return 1;
	}
	
	char buf[BUF_SIZE];
	while(fgets(buf, BUF_SIZE, f)) {
		char *curr;
		char *next = buf;
		
		next_word(&curr, &next);
		while(*curr) {
			if(hashset_add(hashset, (void *)curr))
				printf("added \"%s\"\n", curr);
			next_word(&curr, &next);
		}
	}
		
	fclose(f);	
	return 0;
}

/* Add contents of file to hashset */
int remove_file(HashSet *hashset, char *curr) {
	FILE *f = fopen(curr, "r");
	if(!f) {
		fprintf(stderr, "error opening file %s: %s\n", curr, strerror(errno));
		return 1;
	}
	
	char buf[1024];
	while(fgets(buf, 1024, f)) {
		char *curr;
		char *next = buf;
		
		next_word(&curr, &next);
		while(*curr) {
			if(hashset_remove(hashset, (void *)curr))
				printf("removed \"%s\"\n", curr);
			next_word(&curr, &next);
		}
	}
		
	fclose(f);	
	return 0;
}

/* Print formatted help information about the given command */
void print_command(const char *name, const char *explanation) {
	printf("  %-9s  %s\n", name, explanation);
}

/* Print help information */
void print_help() {
	printf("The following commands are avaiable:\n");

	print_command("a <str>", "Add <str> to the set");
	print_command("c <str>", "Test if <str> is in the set");
	print_command("r <str>", "Remove <str> from the set");
	print_command("x", "Remove all elements from the set");

	print_command("s", "Print the set size");
	print_command("p", "Print a representation of the set");

	print_command("fa <file>", "Add each word of <file> to the set");
	print_command("fr <file>", "Remove each word of <file> from the set");

	print_command("help", "Print help information");
	print_command("quit", "Delete the set and exit the program");
}

/* Advance pointer until first non-whitespace character */
char *skip_ws(char *str) {
	while(isspace(*str))
		str++;
	return str;
}

/* Advance pointer until first whitespace or null character */
char *skip_nonws(char *str) {
	while(*str && !(isspace(*str)))
		str++;
	return str;
}

/* Advance to the next word and null-terminate */
void next_word(char **currp, char **nextp) {
	*currp = skip_ws(*nextp);
	*nextp = skip_nonws(*currp);
	if(**nextp) {
		**nextp = 0;
		(*nextp)++;
	}
}
