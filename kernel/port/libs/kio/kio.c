/*
 * See copyright notice in LICENSE 
 */

#include <sys/filedesc.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/ktrace.h>
#include <sys/uio.h>
#include <sys/vfs_syscalls.h>
#include <sys/malloc.h>

#include "kio.h"

// Both functions below were copied from the NetBSD tree; the only change is
// that uio_vmspace is set to kernel space
static int dofilewrite_(int, struct file *, const void *, size_t, off_t *, int, 
                        register_t *);
static int dofileread_(int, struct file *, void *, size_t, off_t *, int,
                        register_t *);

int
kfclose(KFILE *fp)
{
	int ret = 0;

	if (fd_getfile(fp->fd) != NULL)
		ret = fd_close(fp->fd);

	kern_free(fp);

	return ret;
}

KFILE*
kfopen(const char *path, const char *mode)
{
	int err;
	int omode;
	KFILE *fp;

	if (!(fp = kern_malloc(sizeof(KFILE), M_WAITOK | M_ZERO)))
		//TODO handle errors
		;

	//TODO :'(
	if (*mode == 'r')
		omode = FREAD;
	else if (*mode == 'w')
		omode = FWRITE | O_CREAT;
	else if (*mode == 'a')
		omode = FWRITE | FREAD;
	else
		return NULL;

	//XXX replace ALLPERMS to something appropriate?
	if ((err = fd_open(path, omode, ALLPERMS, &fp->fd)) != 0)
		return NULL;

	fp->f = fd_getfile(fp->fd);

    return fp;
}

int
kfgetc(KFILE *fp)
{
	char ch;
	size_t n;

	if (!fp)
		return -1;

	return kfread(&ch, 1, fp);
}

int
kremove(const char *path)
{
	return do_sys_unlink(path, UIO_SYSSPACE);
}


size_t
kfwrite(const void *buf, size_t nbyte, KFILE *fp)
{
	size_t nwritten = 0;

	if ((fp->f->f_flag & FWRITE) == 0)
		return 0;

	dofilewrite_(fp->fd, fp->f, buf, nbyte, &fp->f->f_offset,
                        FOF_UPDATE_OFFSET, &nwritten);
	return nwritten;
}

size_t
kfread(void *buf, size_t nbyte, KFILE *fp)
{
	size_t nread = 0;

//	if ((fp->f->f_flag & FREAD) == 0)
//		return 0;

	dofileread_(fp->fd, fp->f, buf, nbyte, &fp->f->f_offset,
                       FOF_UPDATE_OFFSET, &nread);
	return nread;
}

static int
dofileread_(int fd, struct file *fp, void *buf, size_t nbyte, off_t *offset, 
            int flags, register_t *retval)
{
	struct iovec aiov;
	struct uio auio;
	size_t cnt;
	int error;

	aiov.iov_base = (void *)buf;
	aiov.iov_len = nbyte;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = nbyte;
	auio.uio_rw = UIO_READ;
	auio.uio_vmspace = vmspace_kernel();

	/*
	 * Reads return ssize_t because -1 is returned on error.  Therefore
	 * we must restrict the length to SSIZE_MAX to avoid garbage return
	 * values.
	 */
	if (auio.uio_resid > SSIZE_MAX) {
		error = EINVAL;
		goto out;
	}

	cnt = auio.uio_resid;
	error = (*fp->f_ops->fo_read)(fp, offset, &auio, fp->f_cred, flags);
	if (error)
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
	cnt -= auio.uio_resid;
	ktrgenio(fd, UIO_READ, buf, cnt, error);
	*retval = cnt;
 out:
	//fd_putfile(fd);
	return (error);
} 

static int
dofilewrite_(int fd, struct file *fp, const void *buf,
	size_t nbyte, off_t *offset, int flags, register_t *retval)
{
	struct iovec aiov;
	struct uio auio;
	size_t cnt;
	int error;

	aiov.iov_base = __UNCONST(buf);		/* XXXUNCONST kills const */
	aiov.iov_len = nbyte;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = nbyte;
	auio.uio_rw = UIO_WRITE;
	auio.uio_vmspace = vmspace_kernel();

	/*
	 * Writes return ssize_t because -1 is returned on error.  Therefore
	 * we must restrict the length to SSIZE_MAX to avoid garbage return
	 * values.
	 */
	if (auio.uio_resid > SSIZE_MAX) {
		error = EINVAL;
		goto out;
	}

	cnt = auio.uio_resid;
	error = (*fp->f_ops->fo_write)(fp, offset, &auio, fp->f_cred, flags);
	if (error) {
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		if (error == EPIPE && !(fp->f_flag & FNOSIGPIPE)) {
			mutex_enter(proc_lock);
			psignal(curproc, SIGPIPE);
			mutex_exit(proc_lock);
		}
	}
	cnt -= auio.uio_resid;
	ktrgenio(fd, UIO_WRITE, buf, cnt, error);
	*retval = cnt;
 out:
	//fd_putfile(fd);
	return (error);
}
