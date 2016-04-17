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
	char  *s;
	int   fd;
	int   i;

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

	i = 'a';
	while ((fd_open(tmp, O_CREAT|O_EXCL|O_RDWR, ALLPERMS, &fd)) != 0) {
		if (i == 'z')
			return NULL;
		*s = i++;
	}
	fd_close(fd);

	return tmp;
}
