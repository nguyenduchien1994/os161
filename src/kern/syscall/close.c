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

int close(int fd, int *ret)
{
  lock_acquire(glbl_mngr->file_sys_lk);

  if (fd < 0 || fd > INT_MAX)
  {
    lock_release(glbl_mngr->file_sys_lk);
    return EBADF;
  }
  
  open_file *to_close = file_list_remove(curproc->open_files,fd);
  *ret = 0;

  if (to_close == NULL)
  {
    lock_release(glbl_mngr->file_sys_lk);
    return EBADF;
  }
  else
  {
    if (to_close -> vfile == NULL)
    {
      lock_release(glbl_mngr->file_sys_lk);
      return EBADF;
    }
    vfs_close(to_close -> vfile);
    open_file_decref(to_close);
    
    lock_release(glbl_mngr->file_sys_lk);
    return 0;
  }
}
