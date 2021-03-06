#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <thread.h>
#include <kern/errno.h>
#include <machine/trapframe.h>
#include <current.h>
#include <addrspace.h>
#include <mips/specialreg.h>
#include <vnode.h>

static void child_fork(void *params, unsigned long junk)
{
  (void)params;
  (void)junk;
  
  lock_acquire(glbl_mngr->proc_sys_lk);

  curproc->context->tf_epc += 4;
  curproc->context->tf_a3 = 0;
  curproc->context->tf_v0 = 0;
  
  as_activate();
  
  struct trapframe tf;
  copy_context(&tf);
  tf.tf_status = CST_IRQMASK | CST_IEp | CST_KUp;
  
  lock_release(glbl_mngr->proc_sys_lk);
  
  mips_usermode(&tf);
}

static void new_proc_destroy(proc *proc)
{
    if (proc->p_cwd) {
      VOP_DECREF(proc->p_cwd);
      proc->p_cwd = NULL;
    }
    
    /* VM fields */
    if (proc->p_addrspace) {
      struct addrspace *as;
      as = proc->p_addrspace;
      proc->p_addrspace = NULL;
      as_destroy(as);
    }
    
    
    file_list_destroy(proc->open_files);
    linkedlist_destroy(proc->children);
    
    spinlock_cleanup(&proc->p_lock);
    
    //lock_destroy(curproc->exit_lock);
    //cv_destroy(curproc->exit_cv);
    
    kfree(proc->p_name);
    kfree(proc);
}

//ret 0 for child, pid of child for parent
int fork(pid_t *pret)
{
  //kheap_printstats();

  //kprintf("proc: %d", sizeof(proc));
  
  lock_acquire(glbl_mngr->proc_sys_lk);

  proc *child = proc_copy();
  int err = 0;
  if(child == NULL){
    lock_release(glbl_mngr->proc_sys_lk);
    return ENOMEM;
  }

  err = as_copy(curproc->p_addrspace, &child->p_addrspace);
  
  if(err){
    lock_release(glbl_mngr->proc_sys_lk);
    new_proc_destroy(child);
    return err;
  }
  err = linkedlist_prepend(curproc->children, child);

  lock_release(glbl_mngr->proc_sys_lk);

  if(err){
    new_proc_destroy(child);
    return ENOMEM;
  }
  child->parent = curproc;
    
  err = thread_fork("usr_spawn",
		    child,
		    child_fork,
		    0,
		    0);
  if(err>0){
    unsigned throw;
    linkedlist_remove_head(curproc->children, &throw);
    new_proc_destroy(child);
    return err;
  }else{
    *pret = -err;
  }

  return 0;
}
