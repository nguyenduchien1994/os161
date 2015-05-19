#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <thread.h>
#include <kern/errno.h>

static struct fork_param{
  proc *parent;
  pid_t *ret;
}

//ret 0 for child, pid of child for parent
int fork(bool parent, pid_t *ret, pid_t *pret)
{
  if(parent){
    struct fork_param;
    &fork_param->parent = curproc;
    &fork_param->pid_t = ret;
    
    //make function to copy registers from current trapframe to pass into syscall, change params to call fork(ret, false)
    int err = thread_fork("usr_spawn",
			  NULL,
			  syscall,
			  fork_param,
			  0);
    return err;
  }
  else{
    //child
    //enter new process? 
    proc *child = proc_copy(curproc);
    if(child == NULL){
      err = ENOMEM;
    }
    else{
      child->parent = curproc;
      child->state = ready;
      int add = proc_mngr_add(glbl_mngr, child, curthread);
      if(add < 0){
	err = ENPROC;
      }
      else{
	*pret = add;
	*ret = 0;
      }
    }
    return err;
  }
}
