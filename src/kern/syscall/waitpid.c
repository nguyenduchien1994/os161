#include <types.h>
#include <syscall.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <usr_file.h>
#include <limits.h>
#include <current.h>
#include <copyinout.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{ 
  if(pid < PID_MIN){
    return ESRCH;
  }
  if(pid > PID_MAX){
    return ESRCH;
  }
  if(options != 0)
    return EINVAL;

  if(status != NULL){
    int dest;
    int err = copyin((const_userptr_t)status, &dest, sizeof(int));
    if(err)
      return err;
  }

  lock_acquire(glbl_mngr->proc_sys_lk);
  struct proc* myproc = proc_mngr_get_from_pid(glbl_mngr, pid);
 
  if(myproc == NULL || myproc->cur_state == dead)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return ESRCH;
  }

  if(myproc->parent != curproc)
    return ECHILD;
  
  lock_acquire(myproc->exit_lock);
  myproc->exit_sem->sem_count--;
  set_state(curproc, waiting);
  
  //kprintf("\n%u waiting on: %u\n", curproc->pid, myproc->pid);

  lock_release(glbl_mngr->proc_sys_lk);
  cv_wait(myproc->exit_cv, myproc->exit_lock);
  
  if(status != NULL){
    *status = myproc->exit_status;
  }
  else {
    return EFAULT;
  }
  V(myproc->exit_sem);
  
  lock_release(myproc->exit_lock);
  
  *ret = pid; 
  return 0;
}
