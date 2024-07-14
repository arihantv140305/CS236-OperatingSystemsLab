#include "alloc.h"

struct block
{
    int start;
    int end;
    int is_allocated;
    struct block *next;
};

class mem_manager
{
public:
    void *base;
    struct block *head;
    bool present;
    mem_manager(void *base) : base(base)
    {
        present = true;
        head = (struct block *)malloc(sizeof(struct block));
        head->start = 0;
        head->end = PAGESIZE - 1;
        head->next = NULL;
        head->is_allocated = 0;
    }
    mem_manager()
    {
        base = NULL;
        present = false;
        head = (struct block *)malloc(sizeof(struct block));
        head->start = 0;
        head->end = PAGESIZE - 1;
        head->next = NULL;
        head->is_allocated = 0;
    }

    void map_new_page(){
        base = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        present = true;
    }

    int cleanup()
    {
        return munmap(base, PAGESIZE);
        present = false;
    }
    char *alloc(int size)
    {
        if (size % MINALLOC != 0)
        {
            return NULL;
        }
        struct block *curr = head;
        while (curr != NULL)
        {
            if (curr->is_allocated == 1)
            {
                curr = curr->next;
                continue;
            }
            int curr_size = curr->end - curr->start + 1;
            if (curr_size > size)
            {
                struct block *newblock = (struct block *)malloc(sizeof(struct block));
                newblock->start = curr->start + size;
                newblock->end = curr->end;
                newblock->next = curr->next;
                newblock->is_allocated = 0;
                curr->end = curr->start + size - 1;
                curr->next = newblock;
                curr->is_allocated = 1;
                char *ret = (char *)(base + curr->start);
                return ret;
            }
            else if (curr_size == size)
            {
                curr->is_allocated = 1;
                char *ret = (char *)(base + curr->start);
                return ret;
            }
            curr = curr->next;
        }
        return NULL;
    }
    void dealloc(char *ptr)
    {
        struct block *curr = head;
        struct block *prev = NULL;
        while (curr != NULL)
        {
            if ((char *)(base + curr->start) == ptr)
            {
                curr->is_allocated = 0;
                if (prev != NULL && prev->is_allocated == 0)
                {
                    prev->end = curr->end;
                    prev->next = curr->next;
                    free(curr);
                    curr = prev;
                }
                if (curr->next != NULL && curr->next->is_allocated == 0)
                {
                    curr->end = curr->next->end;
                    struct block *temp = curr->next;
                    curr->next = curr->next->next;
                    free(temp);
                }
                return;
            }
            prev = curr;
            curr = curr->next;
        }
        return;
    }
};