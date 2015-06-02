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

typedef struct fork_params{
  pid_t *pret;
  struct semaphore *sem;
} fork_params;


static void child_fork(void *params, unsigned long junk)
{
  (void)junk;
  
  lock_acquire(glbl_mngr->proc_sys_lk);

  pid_t pid = proc_mngr_add(glbl_mngr, curproc, curthread);
  *(((fork_params*)params)->pret) = pid;
  if(pid < 1){
    Linked_List_Node *runner = curproc->parent->children->first;
    while(runner->data != curproc){
      runner = runner->next;
      KASSERT(runner != NULL);
    }
    linkedlist_remove(curproc->parent->children, runner->key);
    lock_release(glbl_mngr->proc_sys_lk);
    thread_exit();
  }
  //proc_mngr_make_ready(glbl_mngr, curproc);

  V(((fork_params*)params)->sem);  

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

  proc *child = proc_copy();
  int err = 0;
  if(child == NULL){
    return ENOMEM;
  }

  err = linkedlist_prepend(curproc->children, child);
  if(err){

    new_proc_destroy(child);
    return ENOMEM;
  }
  child->parent = curproc;
  
  child->context = kmalloc(sizeof(struct trapframe));
  if(child->context == NULL){
    new_proc_destroy(child);
    return ENOMEM;
  }
  copy_context(child->context);
  
  err = as_copy(curproc->p_addrspace, &child->p_addrspace);
  
  if(err){
    new_proc_destroy(child);
    return err;
  }

  fork_params *fp = kmalloc(sizeof(struct fork_params));
  if(fp == NULL){
    new_proc_destroy(child);
    return ENOMEM;
  }

  fp->pret = pret;
  fp->sem = sem_create("fork ret sem", 0);
  if(fp->sem == NULL){
    kfree(fp);
    new_proc_destroy(child);
    return ENOMEM;
  }
  
  err = thread_fork("usr_spawn",
		    child,
		    child_fork,
		    (void*)fp,
		    0);
  if(err){
    return err;
  }

  P(fp->sem);
  kfree(fp->sem);
  kfree(fp);
  if(*pret < 0){
    return ENPROC;
  }
  return 0;
}
