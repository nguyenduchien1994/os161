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

int close(int fd)
{
  open_file *to_close = file_list_remove(curproc->open_files,fd);

  if (to_close == NULL)
  {
    // the file handle is not valid
    return -1;
  }
  else
  {
    vfs_close(to_close -> vfile);
    open_file_destroy(to_close);
    
    return 0;
  }
}
