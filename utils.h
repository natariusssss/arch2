#ifndef UTILS_H
#define UTILS_H
int read_all(int fd, char* buf, int n);
int write_all(int fd, const char* buf, int n);
void die(const char* msg);
#endif