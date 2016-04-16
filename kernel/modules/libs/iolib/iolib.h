#ifndef _IOLIB_H
#define _IOLIB_H

#include <sys/types.h>
#include <sys/file.h>

typedef struct kfile {
	file_t *f;
	int    fd;
} KFILE;

KFILE* kfopen(const char*, const char*);
int kfclose(KFILE*);
int kclose(int fd);

size_t kfwrite(const void*, size_t, KFILE*);
size_t kfread (void*, size_t, KFILE*);

int kfgetc(KFILE*);

int kremove(const char*);

#endif
