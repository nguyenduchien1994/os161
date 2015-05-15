#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <current.h>

pid_t getpid(void)
{
  // spinlock_acquire(
  
  KASSERT(curthread != NULL);
  
  /*get proc from global manager */
  proc * userproc = proc_mngr_get_proc(glbl_mngr, curthread); 
  
  KASSERT(userproc != NULL);
  KASSERT(userproc->cur_state == running);
  
  pid_t pid = userproc->pid;

  return pid;
}
