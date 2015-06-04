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

  lock_acquire(glbl_mngr->file_sys_lk);

  struct iovec db;
  struct uio name_uio;
  
  uio_kinit(&db, &name_uio, buf, buflen, 0, UIO_READ);
  name_uio.uio_segflg = UIO_USERSPACE;
  name_uio.uio_space = curproc->p_addrspace;
  name_uio.uio_resid = buflen;

  int err = vfs_getcwd(&name_uio);
  *ret = name_uio.uio_offset;

  lock_release(glbl_mngr->file_sys_lk);
  return err;
}
