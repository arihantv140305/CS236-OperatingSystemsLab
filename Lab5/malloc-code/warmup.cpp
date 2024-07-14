#include<iostream>
#include<sys/mman.h>
#include<unistd.h>
#include<cstring>
using namespace std;

int main(){

    void *addr = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int n;
    cin >> n;
    memcpy(addr, &n, sizeof(n));
}