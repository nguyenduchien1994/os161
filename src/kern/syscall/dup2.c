
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

int dup2(int oldfd, int newfd, int *ret)
{
  open_file *to_dup = file_list_get(curproc -> open_files, oldfd);
  *ret = newfd;

  if (to_dup == NULL)
  {
    return EBADF;
  }
  else
  {
    int err = file_list_insert(curproc -> open_files, to_dup, newfd);
    if (err == -1)
    {
      return EBADF;
    }
    return 0;
  }
}
