#include <types.h>
#include <kern/errno.h>
#include <syscall.h>
#include <vfs.h>
#include <copyinout.h>
#include <uio.h>
#include <kern/iovec.h>
#include <proc.h>

int __getcwd(char *buf, size_t buflen, int *ret)
{
  (void)ret;
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

  int err = vfs_getcwd(name_uio);
  if(err){
    //spl0();
    return -1;
  }
  //spl0();  
  return name_uio->uio_offset;
}
