#include <types.h>
#include <syscall.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <usr_file.h>
#include <limits.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{ 
  lock_acquire(glbl_mngr->proc_sys_lk);
  struct proc* myproc = proc_mngr_get_from_pid(glbl_mngr, pid);
  

  if (status == NULL) //include error check for status
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return EFAULT;
  }
  else if(options != 0)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return EINVAL;
  } 
  else if(myproc == NULL || myproc->cur_state == dead)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return ESRCH;
  }
  
  else if (pid < PID_MIN)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    //return EINVAL;
    return ESRCH;
  }
  else if (pid > PID_MAX)
  {
    lock_release(glbl_mngr->proc_sys_lk);
    return ESRCH;
  }
  
  if(myproc != NULL || myproc->cur_state != dead)
  {
    lock_acquire(myproc->exit_lock);
    myproc->exit_sem->sem_count--;
    set_state(myproc, waiting);


    lock_release(glbl_mngr->proc_sys_lk);
    cv_wait(myproc->exit_cv, myproc->exit_lock);

    *status = myproc->exit_status;
    V(myproc->exit_sem);

    lock_release(myproc->exit_lock);
  }

  //(void)status;
  //(void)options;
  *ret = pid; 
  return 0;
}
