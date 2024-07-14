/**
 * Simple program demonstrating shared memory in POSIX systems.
 *
 * This is the consumer process
 *
 * Figure 3.18
 *
 * @author Gagne, Galvin, Silberschatz
 * Operating System Concepts - Ninth Edition
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include<string.h>

#define STRING_SIZE 8
#define NUM_STR 512
#define FREE_STRING "freeeee\0"
#define FIFO_PATH_3 "/tmp/myfifo3"


int main()
{
	const int SIZE = 4096;
    const char *name = "OS";
    const char *pipe1 = "/tmp/pipe1";
	const char *pipe2 = "/tmp/pipe2";
	const char *free_str = "freeeee";

	int shm_fd;
    int pipe1_fd;
	int pipe2_fd;
    void *ptr;
    int i;
    char buffer[STRING_SIZE];
    int location;

	/* open the shared memory segment */
	shm_fd = shm_open(name, O_RDWR, 0666);

	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		exit(-1);
	}

	/* now read from the shared memory region */
	mkfifo(pipe1, 0666);
	mkfifo(pipe2, 0666);
	mkfifo(FIFO_PATH_3, 0666); 
	int fd3 = open(FIFO_PATH_3, O_WRONLY);
	pipe1_fd = open(pipe1, O_RDONLY);
	pipe2_fd = open(pipe2, O_WRONLY);
	for (i = 0; i < 1000; i++) {
		read(pipe1_fd, &location, sizeof(int));
		memcpy(buffer, ptr + location * STRING_SIZE, STRING_SIZE);
		printf("%s %d\n", buffer, location);
		memcpy(ptr + location * STRING_SIZE, free_str, STRING_SIZE);
		fflush(stdout);
		sleep(0.1);
		write(pipe2_fd, &location, sizeof(int));
	}
	close(pipe1_fd);
	close(pipe2_fd);
	write(fd3, "c", 1);
	close(fd3);
	/* remove the shared memory segment */
	if (shm_unlink(name) == -1) {
		printf("Error removing %s\n",name);
		exit(-1);
	}

	return 0;
}
