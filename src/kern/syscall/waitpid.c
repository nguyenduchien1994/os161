#include <types.h>
#include <syscall.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <usr_file.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{ 
  struct proc* myproc = proc_mngr_get_from_pid(glbl_mngr, pid);
 
  if(options != 0)
  {
    return EINVAL;
  }
  else if(myproc == NULL || myproc->cur_state == dead)
  {
    return ESRCH;
  }
  //include error check for status
  //set ret to ????
  int exitstatus;
  if(myproc != NULL || myproc->cur_state != dead)
  {
    lock_acquire(myproc->exit_lock);
    myproc->wait_count ++;
    set_state(myproc, waiting);
    cv_wait(myproc->exit_cv, myproc->exit_lock);
    exitstatus = myproc->exit_status;
    myproc->wait_count --;
    lock_release(myproc->exit_lock);
  }
  *status = exitstatus;
 
  //(void)status;
  //(void)options;
  *ret = pid; 
  return 0;
}
