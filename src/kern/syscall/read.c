#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <uio.h>
#include <lib.h>
#include <copyinout.h>
#include <current.h>
#include <usr_file.h>
#include <vfs.h>
#include <vnode.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <limits.h>

int read(int fd, void *buf, size_t buflen, ssize_t *ret)
{
  if (fd < 0 || fd > INT_MAX)
  {
    return EBADF;
  }

  void *buf_space = kmalloc(sizeof(*buf)*buflen);
  if (buf_space == NULL)
  {
    return EFAULT;
  }
  
  int err = 0;
  open_file *to_read = file_list_get(((proc*)curproc)->open_files,fd);

  if (to_read == NULL)
  {
    return EBADF;
  }
  else
  {
    if (to_read->flags & O_WRONLY)
    {
       return EBADF;
    } 
    else
    {
      lock_acquire(to_read->file_lk);
      struct uio *read_uio = kmalloc(sizeof(struct uio));
      struct iovec iov;

      uio_kinit(&iov,read_uio,(void*)buf,buflen,to_read->offset,UIO_READ);

      read_uio->uio_segflg = UIO_USERSPACE;
      read_uio->uio_space = curproc->p_addrspace;
      read_uio->uio_resid = buflen;

      while (!err && read_uio->uio_resid)
      {
	err = to_read->vfile->vn_ops->vop_read(to_read->vfile,read_uio);
	*ret = buflen - read_uio->uio_resid;
	to_read->offset = read_uio->uio_offset; 
      }
      lock_release(to_read->file_lk);
    }
  }
  return err;
}
