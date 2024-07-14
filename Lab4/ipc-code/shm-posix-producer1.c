
// shm-posix-producer.c
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
    const char* name = "OS";
    const char* OS_str= "OSisFUN\0";
    const char* free_str= "freeeee\0";

    mkfifo(FIFO_PATH_1, 0666); int fd1 = open(FIFO_PATH_1, O_WRONLY);
    mkfifo(FIFO_PATH_2, 0666); int fd2 = open(FIFO_PATH_2, O_RDONLY);
    mkfifo(FIFO_PATH_3, 0666); int fd3 = open(FIFO_PATH_3, O_RDONLY);

    /* create and configure size of the shared memory segment */
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd,SIZE);

	/* now map the shared memory segment in the address space of the process */
    void *ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    /* Initialize shared memory with "freeeee" */
    for (int i = 0; i < SIZE / 8; i++) {
        sprintf(ptr + i * 8, "%s", free_str);
    }
    printf("[PRODUCER] Shared memory initialized with %d 'freeeee' slots.\n", SIZE / 8);

    /* Write initial strings */
    int strings_written = 0;
    while(strings_written < SIZE / 8){
        sprintf(ptr + strings_written * 8, "%s", OS_str);
        // memcopy(ptr + strings_written * 8, OS_str, 8);
        write(fd1, &strings_written, sizeof(int));
        strings_written++;
        printf("[PRODUCER] Written at slot: %d\n", strings_written);
    }
    printf("[PRODUCER] Initial OS_str written.\n");

    while (strings_written < 1000) {
        
        int slot;
        read(fd2, &slot, sizeof(int));
        if(slot < 0 || slot >= SIZE / 8){
            printf("[PRODUCER] Invalid slot: %d\n", slot);
            continue;
        }
        sprintf(ptr + slot * 8, "%s", OS_str);
        // memcopy(ptr + slot * 8, OS_str, 8);
        write(fd1, &slot, sizeof(int));
        printf("[PRODUCER] Written at slot: %d\n", slot);
        strings_written++;
    }

    char c;
    read(fd3, &c, sizeof(char)); // Wait for consumer to finish

    close(fd1);
    close(fd2);
    close(fd3);
    return 0;
}