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
  if (fd < 0 || fd > INT_MAX)
  {
    return EBADF;
  }
  
  open_file *to_close = file_list_remove(curproc->open_files,fd);
  *ret = 0;

  if (to_close == NULL)
  {
    return EBADF;
  }
  else
  {
    vfs_close(to_close -> vfile);
    open_file_decref(to_close);
    
    return 0;
  }
}
