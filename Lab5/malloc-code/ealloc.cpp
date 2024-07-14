#include "mem_manager.h"
#include <map>
#define EMINALLOC 256

mem_manager manager[4];
std::map<char*, int> alloc_map;
void *base;

int init_alloc()
{
    for(int i = 0; i<4; i++){
        manager[i] = mem_manager();
    }
    return 0;
}

int cleanup()
{
    for(int i = 0; i<4; i++){
        manager[i] = mem_manager();
    }
    return 0;
}

char *alloc(int size)
{
    if (size % EMINALLOC != 0)
    {
        return NULL;
    }
    for(int i = 0; i<4; i++){
        if(manager[i].present){
            char *ptr = manager[i].alloc(size);
            if(ptr != NULL){
                alloc_map[ptr] = i;
                return ptr;
            }
        }
    }
    for(int i = 0; i<4; i++){
        if(!manager[i].present){
            manager[i].map_new_page();
            char *ptr = manager[i].alloc(size);
            if(ptr != NULL){
                alloc_map[ptr] = i;
                return ptr;
            }
        }
    }
    return NULL;
}

void dealloc(char *ptr)
{
    // implement dealloc along with merge
    int i = alloc_map[ptr];
    manager[i].dealloc(ptr);
    return; 
}
