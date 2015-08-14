#include <sys/filedesc.h>
#include <sys/fcntl.h>

#include "mkstemp.h"

int
mkstemp(char *tmp)
{
	register char *s;
	register unsigned long rnd;
	register int i;
	int fd;

	rnd = random();
	s = tmp;
	while (*s++)
    	;

	s--;
	while (*--s == 'X') {
		*s = (rnd % 10) + '0';
		rnd /= 10;
	}
	s++;
	i = 'a';

	while ((fd_open(tmp, O_CREAT|O_EXCL|O_RDWR, 0600, &fd)) != 0) {
		if (i == 'z')
			return(-1);
		*s = i++;
	}

	return fd;
}  
