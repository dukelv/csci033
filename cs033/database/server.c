#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include "window.h"
#include "io_functions.h"
#include "db.h"

/* Struct encapsulating information about a client */
typedef struct Client {
	Window *window;  // The client window
	pthread_t thread;
} Client;

// Global variables
Database *db;
char *scriptname;

typedef struct node {
    pthread_t thread;
    struct node *next;
} node;

node *root;
void LLadd(pthread_t new);
int LLrem(Client *client);

// Forward declarations
Client *client_new(int id);
void client_delete(Client *client);
void *run_client(void *client);
void process_command(char *command, char *response);
int threadcount;
int stopped;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mute = PTHREAD_MUTEX_INITIALIZER;

/***********************************
 * Main function
 ***********************************/

int main(int argc, char **argv) {
	if(argc == 1) {
		scriptname = NULL;
	} else if(argc == 2) {
		int len = strlen(argv[1]);
		scriptname = malloc(len+1);
		strncpy(scriptname, argv[1], len+1);
	} else {
		fprintf(stderr, "Usage: %s [scriptname]\n", argv[0]);
		exit(1);
	}
	threadcount = 0;
	stopped = 0;
	// Ignore SIGPIPE
	struct sigaction ign;
	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &ign, NULL);

	db = db_new();

	char test[1024];
	int i = 0;
	for(i = 0; i < 1024; i++)
	{
	    test[i] = '\0';
	}
	int reset = 0;
	Client *client;
	int counter = 0;
	while(reset==0)
	{
	    int check = read(STDIN_FILENO, test, 1024);
	    if(check==0)
	    {
		break;
	    }
	    if(test[0]=='\n')
	    {
		if(!(client = client_new(counter))) {
			fprintf(stderr, "Could not create client\n");
			exit(1);
		}
		counter++;
	    }
	    else
	    {
		if(strlen(test)==2)
		{
		    if(test[0]=='s')
		    {
			if(stopped==1)
			{
			    puts("already stopped");
			}
			else
			{
			    pthread_mutex_lock(&mute);
			    stopped = 1;
			    pthread_cond_broadcast(&cond);
			    pthread_mutex_unlock(&mute);
			    puts("stop");
			}
		    }
		    else if(test[0]=='g')
		    {
			if(stopped==1)
			{
			    pthread_mutex_lock(&mute);
			    stopped = 0;
			    pthread_cond_broadcast(&cond);
			    pthread_mutex_unlock(&mute);
			    puts("go");
			}
			else
			{
			    puts("not stopped");
			}
		    }
		    else
		    {
			puts("not a commmand");
		    }
		}
		else
		{
		    puts("not a commmand");
		}
	    }
	}
	
	while(threadcount > 0)
	{
	    //do nothing;
	}
	db_delete(db);
	free(scriptname);
	pthread_exit(0);
}

void LLadd(pthread_t new)
{
    node *newnode = malloc(sizeof(node));
    newnode->thread = new;
    newnode->next = NULL;
    if(root)
    {
	node *iter;
	iter = root;
	while(iter->next)
	{
	    iter = iter->next;
	}
	iter->next = newnode;
    }
    else
    {
	root = newnode;
    }
}

int LLrem(Client *client)
{
    pthread_t del = client->thread;
    node *iter = root->next;
    node *prev = root;
    if(root->thread==del)
    {
	root = iter;
	free(prev);
	return 0;
    }
    else
    {
	while(iter)
	{
	    if(iter->thread==del)
	    {
		prev->next = iter->next;
		free(iter);
		return 0;
	    }
	    else
	    {
		iter = iter->next;
		prev = prev->next;
	    }
	}
	puts("Thread not found in Linked List, SHOULD NEVER HAPPEN!");
	return 1;
    }
}

/***********************************
 * Client handling functions
 ***********************************/

/* Create a new client */
Client *client_new(int id) {
	Client *client = (Client *)malloc(sizeof(Client));
	if(!client) {
		perror("malloc");
		return NULL;
	}
	pthread_t test;
	// Create a window and set up a communication channel with it
	char title[20];
	sprintf(title, "Client %d", id);
	if(!(client->window = window_new(title, scriptname))) {
		free(client);
		return NULL;
	}
	pthread_create(&test, 0, run_client, client);
	client->thread = test;
	LLadd(test);
	pthread_detach(test);
	threadcount++;
	return client;
}

/* Delete a client and all associated resources */
void client_delete(Client *client) {
	window_delete(client->window);
	LLrem(client);
	free(client);
}

/* Function executed for a given client */
void *run_client(void *client) {
	char command[BUF_LEN];
	char response[BUF_LEN];

	// Main loop of the client: fetch commands from window, interpret
	// and handle them, and send results to window.
	while(get_command(((Client *)client)->window, command)) {
		pthread_mutex_lock(&mute);
		while(stopped!=0)
		    pthread_cond_wait(&cond, &mute);
		pthread_mutex_unlock(&mute);
		process_command(command, response);
		if(!send_response(((Client *)client)->window, response))
			break;
	}
	fprintf(stderr, "Quitting Client\n");
	client_delete(((Client *)client));
	threadcount--;
	return NULL;
}

/***********************************
 * Command processing functions
 ***********************************/

char *skip_ws(char *str);
char *skip_nonws(char *str);
void next_word(char **curr, char **next);

/* Process the given command and produce an appropriate response */
void process_command(char *command, char *response) {
	char *curr;
	char *next = command;
	next_word(&curr, &next);

	if(!*curr) {
		strcpy(response, "no command");
	} else if(!strcmp(curr, "a")) {
		next_word(&curr, &next);
		char *name = curr;
		next_word(&curr, &next);

		if(!*curr || *(skip_ws(next))) {
			strcpy(response, "ill-formed command");
		} else if(db_add(db, name, curr)) {
			strcpy(response, "added");
		} else {
			strcpy(response, "already in database");
		}
	} else if(!strcmp(curr, "q")) {
		next_word(&curr, &next);

		if(!*curr || *(skip_ws(next))) {
			strcpy(response, "ill-formed command");
		} else if(!db_query(db, curr, response, BUF_LEN)) {
			strcpy(response, "not in database");
		}
	} else if(!strcmp(curr, "d")) {
		next_word(&curr, &next);

		if(!*curr || *(skip_ws(next))) {
			strcpy(response, "ill-formed command");
		} else if(db_remove(db, curr)) {
			strcpy(response, "deleted");
		} else {
			strcpy(response, "not in database");
		}
	} else if(!strcmp(curr, "p")) {
		next_word(&curr, &next);

		if(!*curr || *(skip_ws(next))) {
			strcpy(response, "ill-formed command");
		} else {
			FILE *foutput = fopen(curr, "w");
			if (foutput) {
				db_print(db, foutput);
				fclose(foutput);
				strcpy(response, "done");
			} else {
				strcpy(response, "could not open file");
			}
		}
	} else if(!strcmp(curr, "f")) {
		next_word(&curr, &next);

		if(!*curr || *(skip_ws(next))) {
			strcpy(response, "ill-formed command");
		} else {
			FILE *finput = fopen(curr, "r");
			if(finput) {
				while(fgets(command, BUF_LEN, finput) != 0)
					process_command(command, response);

				fclose(finput);

				strcpy(response, "file processed");
			} else {
				strcpy(response, "could not open file");
			}
		}
	} else {
		strcpy(response, "invalid command");
	}
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
void next_word(char **curr, char **next) {
	*curr = skip_ws(*next);
	*next = skip_nonws(*curr);
	if(**next) {
		**next = 0;
		(*next)++;
	}
}
