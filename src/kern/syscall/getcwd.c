#include <types.h>
#include <kern/errno.h>
#include <syscall.h>
#include <vfs.h>
#include <copyinout.h>
#include <uio.h>
#include <kern/iovec.h>
#include <proc.h>
#include <current.h>

int __getcwd(char *buf, size_t buflen, int *ret)
{
  if(buf == NULL){
    return EFAULT;
  }
  //splhigh();
  struct iovec *db = kmalloc(sizeof(struct iovec));
  if(db == NULL){
    //spl0();
    return ENOMEM;
  }

  struct uio *name_uio = kmalloc(sizeof(struct uio));
  if(name_uio == NULL){
    //spl0();
    return ENOMEM;
  }

  uio_kinit(db, name_uio, buf, buflen, 0, UIO_READ);
  name_uio->uio_segflg = UIO_USERSPACE;
  name_uio->uio_space = curproc->p_addrspace;
  name_uio->uio_resid = buflen;

  int err = vfs_getcwd(name_uio);
  if(err){
    //spl0();
    return err;
  }
  //spl0();  
  *ret = name_uio->uio_offset;
  return 0;
}
