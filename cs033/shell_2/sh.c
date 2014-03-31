#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include "jobs.h"
#define BUFFER 1024

void sigquit_h(int sig);
void sigint_h(int sig);
void sigstp_h(int sig);
void sigchi_h(int sig);
int install_h(int sig, void (*handler)(int));
int fg;
int back;
job_list_t *list;
int jid;

int main() 
{
    int run = 0;
    int inct;
    char infile[BUFFER];
    int outct;
    char outfile[BUFFER];
    int append;
    int ct;
    int argc;
    back = 0;
    jid = 0;
    list = init_job_list();
    sigset_t old;
    sigset_t full;
    sigfillset(&full);
    // Ignore signals while installing handlers
    sigprocmask(SIG_SETMASK, &full, &old);
    //Install signal handlers
    install_h(SIGINT, &sigint_h);
    install_h(SIGTSTP, &sigstp_h);
    install_h(SIGQUIT, &sigquit_h);
    install_h(SIGCHLD, &sigchi_h);
    // Restore signal mask to previous value
    sigprocmask(SIG_SETMASK, &old, NULL);

    while(run==0)
    {
	inct = 0;
	outct = 0;
	ct = 0;
	append = 0;
	argc = 0;
	outfile[0] = '\0';
	infile[0] = '\0';
	char *argv[BUFFER/2];
	argv[0] = '\0';
	
	#ifndef NO_PROMPT
	write(STDOUT_FILENO,"$ ",2);
	#endif
	char buf[BUFFER];
	for(ct = 0; ct < BUFFER; ct++)
	{
	    buf[ct] = ' ';
	}// wipe the old buf just in case
	ct = 0;
	buf[0] = '\0';
	read(STDIN_FILENO, buf, BUFFER);
	if(buf[0]==0)// control-d handling
	{
	    write(STDOUT_FILENO, "\n", 1);
	    break;
	}
	// scan through for redirection chars and paths, replacing and saving in the process
	while(buf[ct]!='\0')
	{
	    if(buf[ct]=='<')
	    {
		inct++;
		buf[ct] = ' ';
		while(isspace(buf[ct])!=0)//buf[ct]==' ')
		{
		    ct++; // skip leading whitespace
		}
		int i = ct;
		while((isspace(buf[i])==0)&&(buf[i]!='<')&&(buf[i]!='>')&&(buf[i]!='\0')&&(buf[i]!='\n'))
		{
		    infile[i-ct] = buf[i]; // save input path
		    i++;
		}
		infile[i-ct] = '\0';
		int j;
		for(j = ct; j < i; j++)
		{
		    buf[j] = ' '; // replace with whitespace
		}
		ct = i-1;
	    }
	    else if(buf[ct]=='>')
	    {
		outct++;
		if((ct+1)<(BUFFER-1))
		{
		    if(buf[ct+1]=='>')
		    {
			append = 1; // >> output
			buf[ct] = ' ';// replace with whitespace
			ct++;
		    }
		}
		buf[ct] = ' ';
		while(isspace(buf[ct])!=0)//buf[ct]==' ')
		{
		    ct++; // skip leading whitespace
		}
		int i = ct;
		while((isspace(buf[i])==0)&&(buf[i]!='<')&&(buf[i]!='>')&&(buf[i]!='\0')&&(buf[i]!='\n'))
		{
		    outfile[i-ct] = buf[i];// save output path
		    i++;
		}
		outfile[i-ct] = '\0';
		int j;
		for(j = ct; j < i; j++)
		{
		    buf[j] = ' '; // replace with whitespace
		}
		ct = i-1;
	    }
	    ct++;
	}
	
	if(outct > 0)
	{
	    printf("inct = %i, infile = %s\n", outct, outfile);
	}
	
	//Basic error checking
	if((inct>1)||(outct>1))
	{
	    char *e = "ERROR: Can't have two input/output redirects on one line.\n";
	    write(STDERR_FILENO, e, strlen(e));
	    continue;
	}
	if(((inct==1)&&(infile[0]=='\0'))||((outct==1)&&(outfile[0]=='\0')))
	{
	    char *e = "ERROR: Missing redirect path.\n";
	    write(STDERR_FILENO, e, strlen(e));
	    continue;
	}
	//making argv
	
	for(ct = 0; ct < BUFFER/2; ct++)
	{
	    argv[ct] = NULL;
	}// wipe the old argv just in case
	ct = 0;
	while(buf[ct]!='\0')
	{
	    if(isspace(buf[ct])!=0)//(buf[ct]==' ')||(buf[ct]=='\n'))
	    {
		while(isspace(buf[ct])!=0)//(buf[ct]==' ')||(buf[ct]=='\n'))
		{
		    ct++; // skip whitespace
		}
	    }
	    else
	    {
		int i = ct;
		char *arg = &buf[i];
		while((isspace(buf[i])==0)&&(buf[i]!='\0'))//(buf[i]!=' ')&&(buf[i]!='\0')&&(buf[i]!='\n'))
		{
		    i++;
		}
		buf[i] = '\0';
		ct = i+1;
		argv[argc] = arg; // save argument
		argc++;
	    }
	}
	
	if(argc>=2)
	{
	    if(strcmp(argv[argc-1],"&")==0)
	    {
		argv[argc-1] = NULL;
		argc = argc - 1;
		back = 1;
	    }
	}
	
	//executing command
	if(argc==0)
	{
	    continue;// no command -> prompt again
	}
	if(strcmp("cd",argv[0])==0)
	{
	    if(argc<2)
	    {
		char *e = "ERROR: Missing path.\n";
		write(STDERR_FILENO, e, strlen(e));
		continue;
	    }
	    else
	    {
		int success = chdir(argv[1]);
		if(success!=0)
		{
		    char *e = strerror(errno);
		    write(STDERR_FILENO, e, strlen(e));
		    write(STDOUT_FILENO, "\n", 1);
		}
	    }
	}
	else if(strcmp("ln",argv[0])==0)
	{
	    if(argc<3)
	    {
		char *e = "ERROR: Missing at least one path.\n";
		write(STDERR_FILENO, e, strlen(e));
		continue;
	    }
	    else
	    {
		int success = link(argv[1],argv[2]);
		if(success!=0)
		{
		    char *e = strerror(errno);
		    write(STDERR_FILENO, e, strlen(e));
		    write(STDOUT_FILENO, "\n", 1);
		}
	    }
	}
	else if(strcmp("rm",argv[0])==0)
	{
	    if(argc<2)
	    {
		char *e = "ERROR: Missing target.\n";
		write(STDERR_FILENO, e, strlen(e));
		continue;
	    }
	    else
	    {
		int success = unlink(argv[1]);
		if(success!=0)
		{
		    char *e = strerror(errno);
		    write(STDERR_FILENO, e, strlen(e));
		    write(STDOUT_FILENO, "\n", 1);
		}
	    }
	}
	else if(strcmp("exit",argv[0])==0)
	{
	    break;
	}
	else if(strcmp("jobs",argv[0])==0)
	{
	    jobs(list);
	}
	else if(strcmp("bg",argv[0])==0)
	{
	    if(argc<2)
	    {
		char *e = "ERROR: Missing PID or JID\n";
		write(STDERR_FILENO, e, strlen(e));
		continue;
	    }
	    else
	    {
		if(*argv[1]=='%')//JID
		{
		    argv[1]++;//skip identifier
		    int id = get_job_pid(list,atoi(argv[1]));
		    if(id!=-1)
		    {
			kill(id,SIGCONT);
			back = 1;
		    }
		    else
		    {
			char *e = "ERROR: Invalid JID\n";
			write(STDERR_FILENO, e, strlen(e));
			continue;
		    }
		}
		else//PID
		{
		    int check = kill(atoi(argv[1]),SIGCONT);
		    back = 1;
		    if(check==-1)
		    {
			back = 0;
			char *e = strerror(errno);
			write(STDERR_FILENO, e, strlen(e));
			write(STDOUT_FILENO, "\n", 1);
			continue;
		    }
		}
	    }
	}
	else if(strcmp("fg",argv[0])==0)
	{
	    if(argc<2)
	    {
		char *e = "ERROR: Missing PID or JID\n";
		write(STDERR_FILENO, e, strlen(e));
		continue;
	    }
	    else
	    {
		int status;
		if(*argv[1]=='%')//JID
		{
		    argv[1]++;//skip identifier
		    int id = get_job_pid(list,atoi(argv[1]));
		    if(id!=-1)
		    {
			kill(id,SIGCONT);
			fg = id;
			waitpid(id, &status, 0);
			if(WIFCONTINUED(status)==1)
			{
			    waitpid(get_job_pid(list,atoi(argv[1])), &status, 0);
			}
		    }
		    else
		    {
			char *e = "ERROR: Invalid JID\n";
			write(STDERR_FILENO, e, strlen(e));
			continue;
		    }
		}
		else//PID
		{
		    int check = kill(atoi(argv[1]),SIGCONT);
		    if(check==-1)
		    {
			char *e = strerror(errno);
			write(STDERR_FILENO, e, strlen(e));
			write(STDOUT_FILENO, "\n", 1);
		    }
		    else
		    {
			fg = atoi(argv[1]);
			waitpid(atoi(argv[1]), &status, 0);
			if(WIFCONTINUED(status)==1)
			{
			    waitpid(get_job_pid(list,atoi(argv[1])), &status, 0);
			}
		    }
		}
	    }
	}
	else// Not built-in unix command
	{
	    if((inct==1)||(outct==1)) // input or output redirect
		{
		    if((inct==1)&&(outct==1)) // input & output redirect
		    {
			if(append==1) // >> output
			{
			    int fdi = open(infile,O_RDONLY);
			    int fdo = open(outfile,O_APPEND|O_CREAT|O_WRONLY,S_IWOTH|S_IROTH|S_IRUSR|S_IWUSR);
			    if((fdi==-1)||(fdo==-1))
			    {
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
			    }
			    else
			    {
				//int status, 
				int pid, status;
				if((pid = fork()) == 0)
				{
				    dup2(fdi,STDIN_FILENO);
				    dup2(fdo,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    exit(WEXITSTATUS(status));
				    continue;
				}
				waitpid(pid, &status, 0);
				int success = close(fdi);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    continue;
				}
				success = close(fdo);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				}
			    }
			}
			else // > output
			{
			    int fdi = open(infile,O_RDONLY);
			    int fdo = open(outfile,O_TRUNC|O_CREAT|O_WRONLY,S_IWOTH|S_IROTH|S_IRUSR|S_IWUSR);
			    if((fdi==-1)||(fdo==-1))
			    {
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
			    }
			    else
			    {
				//int status, 
				int pid, status;
				if((pid = fork()) == 0)
				{
				    dup2(fdi,STDIN_FILENO);
				    dup2(fdo,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    exit(WEXITSTATUS(status));
				    continue;
				}
				waitpid(pid, &status, 0);
				int success = close(fdi);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    continue;
				}
				success = close(fdo);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				}
			    }
			}
		    }
		    else if(inct==1) //  input redirect
		    {
			int fd = open(infile,O_RDONLY);
			if(fd==-1)
			{
			    char *e = strerror(errno);
			    write(STDERR_FILENO, e, strlen(e));
			    write(STDOUT_FILENO, "\n", 1);
			}
			else
			{
			    //int status, 
			    int pid, status;
			    if((pid = fork()) == 0)
			    {
				dup2(fd,STDIN_FILENO);
				execv(argv[0],argv);
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
				exit(WEXITSTATUS(status));
				continue;
			    }
			    waitpid(pid, &status, 0);
			    int success = close(fd);
			    if(success!=0)
			    {
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
			    }
			}
		    }
		    else // output redirect
		    {
			if(append==1) // >> output
			{
			    int fd = open(outfile,O_APPEND|O_CREAT|O_WRONLY,S_IWOTH|S_IROTH|S_IRUSR|S_IWUSR);
			    if(fd==-1)
			    {
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
			    }
			    else
			    {
				//int status, 
				int pid, status;
				if((pid = fork()) == 0)
				{
				    dup2(fd,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    exit(WEXITSTATUS(status));
				    continue;
				}
				waitpid(pid, &status, 0);
				int success = close(fd);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				}
			    }
			}
			else // > output
			{
			    int fd = open(outfile,O_TRUNC|O_CREAT|O_WRONLY,S_IWOTH|S_IROTH|S_IRUSR|S_IWUSR);
			    if(fd==-1)
			    {
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
			    }
			    else
			    {
				int status, pid;
				if((pid = fork()) == 0)
				{
				    dup2(fd,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				    exit(WEXITSTATUS(status));
				    continue;
				}
				waitpid(pid, &status, 0);
				int success = close(fd);
				if(success!=0)
				{
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
				}
			    }
			}
		    }
		}
		else // no redirect
		{
		    //int status = 0;
		    int pid, status;
		    if((pid = fork()) == 0)
		    {
			execv(argv[0],argv);
			char *e = strerror(errno);
			write(STDERR_FILENO, e, strlen(e));
			write(STDOUT_FILENO, "\n", 1);
			exit(WEXITSTATUS(status));
		    }
		    add_job(list, jid, pid, _STATE_RUNNING, argv[0]);
		    jid++;
		    if(setpgid(pid,pid)!=0)
		    {
			char *e = strerror(errno);
			write(STDERR_FILENO, e, strlen(e));
			write(STDOUT_FILENO, "\n", 1);
		    }
		    if(back==0)
		    {
			fg = pid;
			waitpid(pid, &status, WUNTRACED|WCONTINUED);
			if(WIFEXITED(status)==1)
			{
			    printf("Job [%i] (%i) finished.\n", get_job_jid(list, pid), pid);
			    remove_job_pid(list, pid);
			}
			if(WIFSIGNALED(status)==1)//ctrl-c and ctrl-slash
			{
			    printf("Job [%i] (%i) terminated by signal %i.\n", get_job_jid(list, pid), pid, WTERMSIG(status));
			    remove_job_pid(list, pid);
			}
			if(WIFSTOPPED(status)==1)
			{
			    if(WSTOPSIG(status)==SIGTSTP)// ctrl-z
			    {
				printf("Job [%i] (%i) stopped by signal %i.\n", get_job_jid(list, pid), pid, WSTOPSIG(status));
				update_job_pid(list, pid, _STATE_STOPPED);
			    }
			}
		    }
		}
	}
    }
    return 0;
}

int install_h(int sig, void (*handler)(int))
{
    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT);
    sigaddset(&act.sa_mask, SIGTSTP);
    sigaddset(&act.sa_mask, SIGQUIT);
    act.sa_flags = SA_RESTART;
    return sigaction(sig, &act, NULL);
}

void sigchi_h(int sig)
{
    int status;
    int pid = get_next_pid(list);
    while((pid!=-1)&&(pid!=fg))
    {
	waitpid(pid, &status, WNOHANG|WUNTRACED|WCONTINUED);
	if(back==1)
	{
	    printf("\n");
	}
	if(WIFEXITED(status)==1)
	{
	    printf("Job [%i] (%i) finished.\n", get_job_jid(list, pid), pid);
	    remove_job_pid(list, pid);
	}
	if(WIFSIGNALED(status)==1)//ctrl-c and ctrl-slash
	{
	    printf("Job [%i] (%i) terminated by signal %i.\n", get_job_jid(list, pid), pid, WTERMSIG(status));
	    remove_job_pid(list, pid);
	}
	if(WIFSTOPPED(status)==1)
	{
	    if(WSTOPSIG(status)==SIGTSTP)// ctrl-z
	    {
		printf("Job [%i] (%i) stopped by signal %i.\n", get_job_jid(list, pid), pid, WSTOPSIG(status));
		update_job_pid(list, pid, _STATE_STOPPED);
	    }
	}
	if(WIFCONTINUED(status)==1)
	{
	    printf("Job [%i] (%i) continued by signal.\n", get_job_jid(list, pid), pid);
	    fg = pid;
	    update_job_pid(list, pid, _STATE_RUNNING);
	}
	pid = get_next_pid(list);
	back = 0;
    }
    sig += 0;//just for compiler
}

void sigint_h(int sig)
{
    if(kill(fg,sig)!=0)
    {
	char *e = strerror(errno);
	write(STDERR_FILENO, e, strlen(e));
	write(STDOUT_FILENO, "\n", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
}

void sigstp_h(int sig)
{
    if(kill(fg,sig)==-1)
    {
	char *e = strerror(errno);
	write(STDERR_FILENO, e, strlen(e));
	write(STDOUT_FILENO, "\n", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
}

void sigquit_h(int sig)
{
    if(kill(fg,sig)!=0)
    {
	char *e = strerror(errno);
	write(STDERR_FILENO, e, strlen(e));
	write(STDOUT_FILENO, "\n", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
}
