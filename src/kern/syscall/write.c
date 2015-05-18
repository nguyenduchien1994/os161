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

ssize_t write(int fd, const void *buf, size_t nbytes, ssize_t *ret) 
{
  int err = 0;
  if(fd <= 0){
    err = EBADF;
  }
  else{
    if(fd == 1 || fd == 2){
      void *dest = kmalloc(nbytes);
      copyin((const_userptr_t)buf, dest, nbytes);
      
      *ret = 0;
      unsigned i = 0;
      while(i < nbytes && *((char*)(dest + i)) != '\0'){
	*ret += kprintf(dest+i);
      }
      
      kfree(dest);
    }
    else{
      open_file *f = file_list_get(((proc*)curproc)->open_files, fd);
      if(f == NULL){
	err = EBADF;
      }
      else{
	lock_acquire(f->file_lk);
	while(!err && (unsigned)(*ret) < nbytes)
	{
	    
	  struct uio *write_uio = kmalloc(sizeof(struct uio));
	  struct iovec iov;
	  uio_kinit(&iov, 
		    write_uio, 
		    (void*)buf, 
		    nbytes, 
		    f->offset, 
		    UIO_WRITE);
	  write_uio->uio_segflg = UIO_USERSPACE;
	  write_uio->uio_resid = nbytes;

	  err = f->vfile->vn_ops->vop_write(f->vfile, write_uio);
	 
	  *ret = nbytes - write_uio->uio_resid;
	  f->offset = write_uio->uio_offset;
	}
	lock_release(f->file_lk);
      }
    }
  }
return err;
}
