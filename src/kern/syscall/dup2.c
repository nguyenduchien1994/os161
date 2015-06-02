
#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <copyinout.h>
#include <vfs.h>
#include <kern/errno.h>
#include <current.h>
#include <vnode.h>
#include <usr_file.h>
#include <current.h>
#include <limits.h>

int dup2(int oldfd, int newfd, int *ret)
{
  lock_acquire(glbl_mngr->file_sys_lk);

  if (oldfd == newfd)
  {
    *ret = newfd;
    lock_release(glbl_mngr->file_sys_lk);
    return 0;
  }

  if (oldfd < 0 || oldfd >= OPEN_MAX || newfd < 0 || newfd >= OPEN_MAX)
  {
    lock_release(glbl_mngr->file_sys_lk);
    return EBADF;
  } 

  open_file *to_dup = file_list_get(curproc -> open_files, oldfd);
  *ret = newfd;

  if (to_dup == NULL)
  {
    lock_release(glbl_mngr->file_sys_lk);
    return EBADF;
  }
  else
  {
    int err = file_list_insert(curproc -> open_files, to_dup, newfd);
    lock_release(glbl_mngr->file_sys_lk);
    if (err == -1)
    {
      return EBADF;
    }
    return 0;
  }
}
