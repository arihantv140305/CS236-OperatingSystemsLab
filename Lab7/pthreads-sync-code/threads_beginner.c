#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 

int counter = 0;
pthread_mutex_t lock;
pthread_cond_t cond[10];
int ready[10];

void* increment(void* arg){
    int* i = (int*) arg;
    for(int j = 0; j < 5; j++){
        pthread_mutex_lock(&lock);
        while(ready[(*i + 9)%10] == 0){
            pthread_cond_wait(&cond[(*i + 9)%10], &lock);
        }
        printf("Thread %d\n", *i);
        ready[(*i + 9)%10] = 0;
        ready[*i] = 1;
        pthread_cond_signal(&cond[*i]);
        pthread_mutex_unlock(&lock);
    }
}

int main(){
    int n = 10;
    pthread_t threads[n];
    int id[n];
    for(int i = 0; i < 10; i++){
        pthread_cond_init(&cond[i], NULL);
        ready[i] = 0;
    }
    ready[9] = 1;
    for(int i = 0; i < n; i++){
        id[i] = i;  
        pthread_create(&threads[i], NULL, (void *) &increment, (void *)&id[i]);
    }
    for(int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
    }
    printf("I am the main thread\n");
}