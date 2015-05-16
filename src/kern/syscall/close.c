#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <copyinout.h>
#include <vfs.h>
#include <kern/errno.h>
#include <current.h>
#include <vnode.h>

int close(int fd)
{
  KASSERT(curthread != NULL);
  proc * currentproc = proc_mngr_get_proc(glbl_mngr, curthread);

  KASSERT(currentproc != NULL);
  KASSERT(currentproc->cur_state == running);

  Linked_List * openfiles = currentproc->open_files;
  Linked_List_Node * thefile = openfiles->first;
  (void)thefile;

  if (fd > 0 || fd < openfiles->first->key)
    {
      return EBADF;
    }
  
  return 0;
}
