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
#include <string.h>

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
	const char *message = "OSisFUN\0";
	const char *free_str = "freeeee\0";


	int shm_fd;
    int pipe1_fd;
	int pipe2_fd;
    void *ptr;
    int i;
    char buffer[STRING_SIZE];
    int location = 0;

	/* open the shared memory segment */
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SIZE);

	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}
	for (i = 0; i < NUM_STR; i++) {
		memcpy(ptr + location * STRING_SIZE, free_str, STRING_SIZE);
    }
	/* now read from the shared memory region */
	mkfifo(FIFO_PATH_3, 0666); int fd3 = open(FIFO_PATH_3, O_RDONLY);
	mkfifo(pipe1, 0666);
	mkfifo(pipe2, 0666);
	pipe1_fd = open(pipe1, O_WRONLY);
	pipe2_fd = open(pipe2, O_RDONLY);
	
	for (i = 0; i < 1000; i++) {
		if(i < NUM_STR) location = i;
		else read(pipe2_fd, &location, sizeof(int));
		memcpy(ptr + location * STRING_SIZE, message, STRING_SIZE);
		printf("Writing at %d\n", location);
		write(pipe1_fd, &location, sizeof(int));
	}
	/* remove the shared memory segment */
	char c;
	// sleep(3);
	read(fd3, &c, sizeof(char)); // Wait for consumer to finish
	close(pipe1_fd);
	close(pipe2_fd);
	close(fd3);
	return 0;
}
