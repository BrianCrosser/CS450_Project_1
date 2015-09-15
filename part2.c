#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int pfd1[2];
    if(pipe(pfd1) == -1){
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }
    

    pid_t first_childPID;
    switch(first_childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
        case 0:
            if(dup2(pfd1[1], 1 ) == -1 || close(pfd1[0]) == -1)
                fprintf(stderr, "dup2 or close Failed");
            execlp("w", "w", NULL);
    }
    
    pid_t second_childPID;
    switch(second_childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
        case 0:
            if(dup2(pfd1[0], 0 ) == -1 || close(pfd1[1]) == -1)
                fprintf(stderr, "dup2 or close Failed");
            execlp("wc", "wc", "-l", NULL);
    }
    
    
    return 0;
}
