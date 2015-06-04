#include <types.h>
#include <syscall.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <usr_file.h>
#include <limits.h>
#include <current.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{ 
  if(status == NULL)
    return EFAULT;
  if(pid < PID_MIN)
    return EINVAL;
  if(pid > PID_MAX || options != 0)
    return ESRCH;

  lock_acquire(glbl_mngr->proc_sys_lk);
  struct proc* myproc = proc_mngr_get_from_pid(glbl_mngr, pid);
  
  if(myproc == NULL || myproc->cur_state == dead)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return ESRCH;
  }
  
  lock_acquire(myproc->exit_lock);
  myproc->exit_sem->sem_count--;
  set_state(curproc, waiting);
  
  //kprintf("\n%u waiting on: %u\n", curproc->pid, myproc->pid);

  lock_release(glbl_mngr->proc_sys_lk);
  cv_wait(myproc->exit_cv, myproc->exit_lock);
  
  *status = myproc->exit_status;
  V(myproc->exit_sem);
  
  lock_release(myproc->exit_lock);
  
  *ret = pid; 
  return 0;
}
