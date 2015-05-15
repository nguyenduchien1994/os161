#include <types.h>
#include <kern/errno.h>
#include <syscall.h>
#include <vfs.h>
#include <copyinout.h>
#include <uio.h>
#include <kern/iovec.h>
#include <proc.h>

int __getcwd(char *buf, size_t buflen)
{
  spinlock_acquire(&syscall_lock);
  char name_buf[buflen];

  struct iovec *db = kmalloc(sizeof(struct iovec));
  if(db == NULL){
    spinlock_release(&syscall_lock);
    return ENOMEM;
  }

  struct uio *name_uio = kmalloc(sizeof(struct uio));
  if(name_uio == NULL){
    spinlock_release(&syscall_lock);
    return ENOMEM;
  }

  uio_kinit(db, name_uio, name_buf, buflen, 0, UIO_READ);
  
  int err = vfs_getcwd(name_uio);
  if(!err){
    spinlock_release(&syscall_lock);
    return err;
  }
  
  err = copyout(name_buf, (userptr_t)buf, name_uio->uio_offset);
  spinlock_release(&syscall_lock);
  return err;
}
