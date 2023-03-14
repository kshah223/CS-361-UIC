#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <dirent.h>
#include <errno.h>
#include<sys/wait.h>

void redirectOut(char session[]) {
	int outfile = open(session, O_CREAT | O_WRONLY, 0644);
    dup2(outfile, 1);
	close(outfile);
}

void redirectErr(char session[]) {
	int errfile = open(session, O_CREAT | O_WRONLY, 0644);
    dup2(errfile, 2);
	close(errfile);
}
	

void exec_command(char* command) {
    char* program = strtok(command," ");
    char *args[16]={program};
	int i=1;
	int func;
	char* redirectInOutErr;
	
    while((args[i]=strtok(NULL," "))) {
		if (strcmp(args[i], "<") == 0) {
            redirectInOutErr = strtok(NULL," ");
            func = open(redirectInOutErr, O_RDONLY);
            dup2(func, 0);
        } else if (strcmp(args[i], ">") == 0) {
            redirectInOutErr = strtok(NULL," ");
            func = open(redirectInOutErr, O_CREAT | O_WRONLY , 0644);
            dup2(func, 1);
        } else if (strcmp(args[i], "2>") == 0) {
            redirectInOutErr = strtok(NULL," ");
            func = open(redirectInOutErr, O_CREAT | O_WRONLY , 0644);
            dup2(func, 2);
        } else i++;
	}
	char* path = getenv("PATH");
	char* token = strtok(path, ":");
	char* temp2;
	char temp1[256];
	char* path2;
    while(token) {
		strcpy(temp1, token);
		temp2 = strcat(temp1, "/");
		path2 = strcat(temp2, program);
		execve(path2,args,NULL);
		token=strtok(NULL,":");
	}
    fprintf(stderr,"dsh: command not found: %s\n",program);
    exit(0);
}

void run(char*);
void run_pipeline(char* head, char* tail) {
	
	int pipefd[2];
	pipe(pipefd);
	
	int cpid = fork();
    if (cpid == 0) {
        dup2(pipefd[1],STDOUT_FILENO);
		close(pipefd[1]);
		close(pipefd[0]);
        exec_command(head);
        exit(1);
	} else {
		wait(0);
		dup2(pipefd[0],STDIN_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);
		run(tail);
	}
}

void run_sequence(char* head, char* tail) {
	run(head);
	if(tail != NULL) run_sequence(tail,NULL);
}

void run(char *line) {
    char *sep;
    if((sep=strstr(line,";"))) {
        *sep=0;
        run_sequence(line,sep+1);
    }
    else if((sep=strstr(line,"|"))) {
        *sep=0;        
        run_pipeline(line,sep+1);
    }
	else {
        if(!fork())
			exec_command(line);
        else 
			wait(0);        
    }
}

int main(int argc, char** argv) {
    char *line = 0;
    size_t size = 0;
    int Nsession = 0, N = 0;

    char folder[100];
    snprintf(folder,100,"%s/.dsh",getenv("HOME"));
    mkdir(folder,0755);

    // need to create the appropriate session folder
    // to put our <N>.stdout and <N>.stderr files in.
    while (1) {
        snprintf(folder,100,"%s/.dsh/%d",getenv("HOME"), Nsession);
        if (mkdir(folder,0755) == 0) {
            break;
        }
        Nsession++;
    }
    
    printf("dsh> ");

    int origin=dup(0);
    int origout=dup(1);
    int origerr=dup(2);
    char session[120];

    while(getline(&line,&size,stdin) > 0) {

        // temporarily redirect stdio fds to
        // files. This will be inherited by children.
        snprintf(session, 120, "%s/%d.stdout", folder, N);
        redirectOut(session);
        snprintf(session, 120, "%s/%d.stderr", folder, N);
		redirectErr(session);
        

        line[strlen(line)-1]=0; // kill the newline
        run(line); 

        // restore the stdio fds before interacting
        // with the user again
        dup2(origin, 0);
        dup2(origout, 1);
        dup2(origerr, 2);

        N++;
        printf("dsh> ");
    }
}
