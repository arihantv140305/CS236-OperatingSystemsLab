#include "mem_manager.h"

mem_manager manager;
void *base;

int init_alloc()
{

    base = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    manager = mem_manager(base);
    return 0;
}

int cleanup()
{
    return manager.cleanup();
}

char *alloc(int size)
{
    return(manager.alloc(size));
}

void dealloc(char *ptr)
{
    // implement dealloc along with merge
    manager.dealloc(ptr);
    return; 
}
