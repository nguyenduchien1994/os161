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

  if (buf == NULL)
  {
    return EBADF;
  }
  
  int err = 0;
  lock_acquire(glbl_mngr->file_sys_lk);

  open_file *to_read = file_list_get(((proc*)curproc)->open_files,fd);

  if (to_read == NULL)
  {
    lock_release(glbl_mngr->file_sys_lk);
    return EBADF;
  }
  else
  {
    if (to_read->flags & O_WRONLY)
    {
      lock_release(glbl_mngr->file_sys_lk);
      return EBADF;
    } 
    else
    {
      lock_acquire(to_read->file_lk);
      struct uio read_uio;
      struct iovec iov;

      uio_kinit(&iov,&read_uio,(void*)buf,buflen,to_read->offset,UIO_READ);

      read_uio.uio_segflg = UIO_USERSPACE;
      read_uio.uio_space = curproc->p_addrspace;
      read_uio.uio_resid = buflen;
      
      while (!err && read_uio.uio_resid)
      {
	//err = VOP_READ(to_read->vfile,&read_uio);
	err = to_read->vfile->vn_ops->vop_read(to_read->vfile,&read_uio);
	*ret = buflen - read_uio.uio_resid;
	to_read->offset = read_uio.uio_offset; 
      }
      lock_release(to_read->file_lk);
    }
  }
  lock_release(glbl_mngr->file_sys_lk);
  return err;
}
