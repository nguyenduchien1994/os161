#include <types.h>
#include <spl.h>
#include <syscall.h>
#include <proc.h>
#include <current.h>

pid_t getpid(void)
{
  // spinlock_acquire(&syscall_lock);
  splhigh();
  KASSERT(curthread != NULL);
  
  /*get proc from global manager */
  // proc * userproc = proc_mngr_get_proc(glbl_mngr, curthread); 
  
  KASSERT(curproc != NULL);
  KASSERT(curproc->cur_state == running);
  
  pid_t pid = curproc->pid;
  
  // spinlock_release(&syscall_lock);
  spl0();
  return pid;
}
