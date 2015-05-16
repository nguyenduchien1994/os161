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

int dup2(int oldfd, int newfd)
{
  open_file *to_dup = file_list_get(curproc -> open_files, oldfd);

  file_list_insert(curproc -> open_files, to_dup, newfd);
  return 0;
}
