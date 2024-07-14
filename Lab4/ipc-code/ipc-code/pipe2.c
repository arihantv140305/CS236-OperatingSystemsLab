#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

int main(){
    const char* pipe1 = "/tmp/pipe1";
    const char* pipe2 = "/tmp/pipe2";
    int pipe_fd;
    int pipe2_fd;
    mkfifo(pipe1, 0666);
    mkfifo(pipe2, 0666);
    pipe_fd = open(pipe1, O_RDONLY);
    pipe2_fd = open(pipe2, O_WRONLY);
    char message[10];
    read(pipe_fd, message, 8);
    printf("%d\n", strlen(message));
    write(pipe2_fd, message, strlen(message));
}