// shm-posix-consumer.c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define FIFO_PATH_1 "/tmp/myfifo_p_to_c"
#define FIFO_PATH_2 "/tmp/myfifo_c_to_p"
#define FIFO_PATH_3 "/tmp/myfifo3"

int main()
{
    const int SIZE = 4096;
    const char *name = "OS";
    const char *free_str= "freeeee\0";

    mkfifo(FIFO_PATH_1, 0666); int fd1 = open(FIFO_PATH_1, O_RDONLY);
    mkfifo(FIFO_PATH_2, 0666); int fd2 = open(FIFO_PATH_2, O_WRONLY);
    mkfifo(FIFO_PATH_3, 0666); int fd3 = open(FIFO_PATH_3, O_WRONLY);

    /* open the shared memory segment */
    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("Shared memory failed\n");
        exit(-1);
    }

    /* now map the shared memory segment in the address space of the process */
    void* ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        exit(-1);
    }

    /* Consume 1000 strings */
    char* msg = (char*)malloc(8); // char msg[8];
    for (int i = 0; i < 1000; i++) {
        int slot;
        read(fd1, &slot, sizeof(int));
        sprintf(msg, "%s", ptr + slot * 8);
        // memcpy(msg, ptr + slot * 8, 8);
        write(fd2, &slot, sizeof(int));
        printf("[CONSUMER] Read %s from slot %d\n", msg, slot);
        sprintf(ptr + slot * 8, "%s", free_str); // Free up the slot
        sleep(1); // Digest the consumed string
    }

    write(fd3, "c", 1); // Signal producer to terminate

    /* remove the shared memory segment */
	if (shm_unlink(name) == -1) {
		printf("Error removing %s\n",name);
		exit(-1);
	}

    close(fd1);
    close(fd2);
    close(fd3);
    unlink(FIFO_PATH_1);
    unlink(FIFO_PATH_2);
    unlink(FIFO_PATH_3);
    return 0;
}