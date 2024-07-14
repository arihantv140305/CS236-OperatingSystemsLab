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
    pipe_fd = open(pipe1, O_WRONLY);
    pipe2_fd = open(pipe2, O_RDONLY);
    char *message = "OSisFUN\0";
    write(pipe_fd, message, strlen(message));
    read(pipe2_fd, message, 8);
    printf("%s\n", message);
}