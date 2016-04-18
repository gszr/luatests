/*
 * See copyright notice in LICENSE
 */

#include <sys/filedesc.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "ktmpnam.h"

char*
ktmpnam(char *tmp)
{
	long  rnd;
	char  ch, *s;
	int   fd;

	s   = tmp;
	rnd = random();

	while (*s++)
    	;

	s--;
	while (*--s == 'X') {
		*s = (rnd % 10) + '0';
		rnd /= 10;
	}
	s++;

	ch = 'a';
	while ((fd_open(tmp, O_CREAT|O_EXCL|O_RDWR, ALLPERMS, &fd)) != 0) {
		if (ch == 'z')
			return NULL;
		*s = ch++;
	}
	if (fd_getfile(fd) != NULL)
		fd_close(fd);

	return tmp;
}
