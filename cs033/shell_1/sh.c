#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#define BUFFER 1024

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
	for(ct = 0; ct < BUFFER/2; ct++)
	{
	    argv[ct] = NULL;
	}// wipe the old argv just in case
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
	    break;
	}
	// scan through for redirection chars and paths, replacing and saving in the process
	while(buf[ct]!='\0')
	{
	    if(buf[ct]=='<')
	    {
		inct++;
		buf[ct] = ' ';
		while(buf[ct]==' ')
		{
		    ct++; // skip leading whitespace
		}
		int i = ct;
		while((buf[i]!=' ')&&(buf[i]!='<')&&(buf[i]!='>')&&(buf[i]!='\0')&&(buf[i]!='\n'))
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
		if((ct+1)<BUFFER)
		{
		    if(buf[ct+1]=='>')
		    {
			append = 1; // >> output
			buf[ct] = ' ';// skip leading whitespace
			ct++;
		    }
		}
		buf[ct] = ' ';
		while(buf[ct]==' ')
		{
		    ct++; // skip leading whitespace
		}
		int i = ct;
		while((buf[i]!=' ')&&(buf[i]!='<')&&(buf[i]!='>')&&(buf[i]!='\0')&&(buf[i]!='\n'))
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
	ct = 0;
	while(buf[ct]!='\0')
	{
	    if((buf[ct]==' ')||(buf[ct]=='\n'))
	    {
		while((buf[ct]==' ')||(buf[ct]=='\n'))
		{
		    ct++; // skip whitespace
		}
	    }
	    else
	    {
		int i = ct;
		char *arg = &buf[i];
		while((buf[i]!=' ')&&(buf[i]!='\0')&&(buf[i]!='\n'))
		{
		    i++;
		}
		buf[i] = '\0';
		ct = i+1;
		argv[argc] = arg; // save argument
		argc++;
	    }
	}
	//executing command
	if(argc==0)
	    continue;// no command -> prompt again
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
				int status, pid;
				if((pid = fork()) == 0)
				{
				    dup2(fdi,STDIN_FILENO);
				    dup2(fdo,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
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
				int status, pid;
				if((pid = fork()) == 0)
				{
				    dup2(fdi,STDIN_FILENO);
				    dup2(fdo,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
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
			    int status, pid;
			    if((pid = fork()) == 0)
			    {
				dup2(fd,STDIN_FILENO);
				execv(argv[0],argv);
				char *e = strerror(errno);
				write(STDERR_FILENO, e, strlen(e));
				write(STDOUT_FILENO, "\n", 1);
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
				int status, pid;
				if((pid = fork()) == 0)
				{
				    dup2(fd,STDOUT_FILENO);
				    execv(argv[0],argv);
				    char *e = strerror(errno);
				    write(STDERR_FILENO, e, strlen(e));
				    write(STDOUT_FILENO, "\n", 1);
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
		    int status, pid;
		    if((pid = fork()) == 0)
		    {
			execv(argv[0],argv);
			char *e = strerror(errno);
			write(STDERR_FILENO, e, strlen(e));
			write(STDOUT_FILENO, "\n", 1);
		    }
		    waitpid(pid, &status, 0);
		}
	}
    }
    return 0;
}
