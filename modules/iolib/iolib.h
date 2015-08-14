#ifndef _IOLIB_H
#define _IOLIB_H

int kclose(int);
file_t* kfopen(const char*, int, int, int*);
int kopen(const char*, int, int, int*);
int kwrite(int, void*, size_t, size_t*);
int kread(int, void*, size_t, size_t*);

#endif
