#include "utils.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
void die(const char* msg) {
    perror(msg);
    exit(1);
}
int read_all(int fd, char* buf, int n) {
    int total=0;
    while (total<n)
    {
        int r=read(fd, buf+total, n-total);
        if (r<=0) return -1;
        total+=r;
    }
    return 0;
}
int write_all(int fd, const char* buf, int n) {
    int total=0;
    while (total<n)
    {
        int w=write(fd, buf+total, n-total);
        if (w<=0) return -1;
        total+=w;
    }
    return 0;
}