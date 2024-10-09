#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(){
    pid_t pid;

    pid = fork();

    if(pid < 0){
        fprintf(stderr, "Fork failed\n");
        return 1;
    }else if(pid == 0){
        printf("This is child process. PID: %d\n", getpid());
        printf("Child process exitting\n");
        exit(0);
    }else{
        printf("This is parent process. PID: %d\n", getpid());
        printf("Sleeping for 30 second\n");
        wait(NULL);
        sleep(30);

        // Thu gom tien trinh zombie
        // printf("Parent process is collecting zombie process\n");
        printf("Finish sleeping\n");
    }

    return 0;
}