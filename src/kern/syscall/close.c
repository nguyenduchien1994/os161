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

int close(int fd, int *ret)
{
  open_file *to_close = file_list_remove(curproc->open_files,fd);
  *ret = 0;

  if (to_close == NULL)
  {
    // the file handle is not valid
    return EBADF;
  }
  else
  {
    vfs_close(to_close -> vfile);
    open_file_decref(to_close);
    
    return 0;
  }
}
