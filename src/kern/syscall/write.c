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

int write(int fd, const void *buf, size_t nbytes, ssize_t *ret) 
{
  int err = 0;
  open_file *f = file_list_get(((proc*)curproc)->open_files, fd);
  if(f == NULL){
    err = EBADF;
  }
  else{
    if(f->flags & O_RDONLY){
      err = EBADF;
    }
    else{
      lock_acquire(f->file_lk);
      struct uio write_uio;
      struct iovec iov;
      uio_kinit(&iov, 
		&write_uio, 
		(void*)buf, 
		nbytes, 
		f->offset, 
		UIO_WRITE);
      write_uio.uio_segflg = UIO_USERSPACE;
      write_uio.uio_space = curproc->p_addrspace;
      write_uio.uio_resid = nbytes;
      
      while(!err && write_uio.uio_resid)
      {  
	err = f->vfile->vn_ops->vop_write(f->vfile, &write_uio);
	
	*ret = nbytes - write_uio.uio_resid;
	f->offset = write_uio.uio_offset;
      }
      lock_release(f->file_lk);
    }
  }
  return err;
}
