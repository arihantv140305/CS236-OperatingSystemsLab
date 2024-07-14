#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


// Signal handler for SIGINT
void sigint_handler(int signum)
{
    printf("I will run forever!\n");
}

int main()
{
    int p = fork();
    if(p != 0){
        kill(-getpid(), SIGTERM);
        printf("message printed by parent after killing\n");
        wait(NULL);
    }
    else{
        sleep(100);
        printf("message printed by child, this shouldn't be printed since child was terminated prematurely\n");
    }
    return 0;
}
