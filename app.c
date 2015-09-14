#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if(argc != 2){
        fprintf(stderr, "File name required\n");
        exit(1);
    }
    
    int readFD;
    int pfd[2];
    printf("%s\n", argv[1]);
    /*
    if(pipe(pfd) == -1){
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }
    */

    pid_t childPID;
    switch(childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
        case 0:
            if((readFD = open(argv[1], O_RDONLY)) < 0)
                fprintf(stderr, "Open Failed");
            if(dup2(readFD, 0) == -1)
                fprintf(stderr, "dup2 or clsoe Failed");
            execlp("wc", "wc", "-l", NULL);
    }
    
    /*
    if(close(pfd[0]) == -1 || close(pfd[1]) == -1)
        fprintf(stderr, "Parent pipes failed to close");
    while(wait(NULL) != -1)
        ;
    */
    return 0;
}
