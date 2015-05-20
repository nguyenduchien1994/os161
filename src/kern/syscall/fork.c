#include <types.h>
#include <syscall.h>
#include <proc.h>
#include <thread.h>
#include <kern/errno.h>
#include <machine/trapframe.h>
#include <current.h>
#include <addrspace.h>
#include <mips/specialreg.h>

typedef struct fork_params{
  pid_t *pret;
  struct semaphore *sem;
} fork_params;


static void child_fork(void *params, unsigned long junk)
{
  (void)junk;
  //give return back to parent
  pid_t pid = proc_mngr_add(glbl_mngr, curproc, curthread);
  *(((fork_params*)params)->pret) = pid;
  V(((fork_params*)params)->sem);
  if(pid < 1){
    Linked_List_Node *runner = curproc->parent->children->first;
    while(runner->data != curproc){
      runner = runner->next;
      KASSERT(runner != NULL);
    }
    linkedlist_remove(curproc->parent->children, runner->key);
    thread_exit();
  }
  //proc_mngr_make_ready(glbl_mngr, curproc);

  curproc->context->tf_epc += 4;
  curproc->context->tf_a3 = 0;
  curproc->context->tf_v0 = 0;
  
  as_activate();
  
  struct trapframe tf;
  copy_context(&tf);
  tf.tf_status = CST_IRQMASK | CST_IEp | CST_KUp;

  mips_usermode(&tf);
}


//ret 0 for child, pid of child for parent
int fork(pid_t *pret)
{
  proc *child = proc_copy();
  
  linkedlist_prepend(curproc->children, child);
  child->parent = curproc;
  
  child->context = kmalloc(sizeof(struct trapframe));
  copy_context(child->context);

  int err;
  as_copy(curproc->p_addrspace, &child->p_addrspace);
  /*
  if(err){
    kprintf("Fork error - %d", err);
    return err;
  }
  */
  fork_params *fp = kmalloc(sizeof(struct fork_params));
  fp->pret = pret;
  fp->sem = sem_create("fork ret sem", 0);
  if(fp->sem == NULL){
    return ENOMEM;
  }
  
  err = thread_fork("usr_spawn",
	      child,
	      child_fork,
	      (void*)fp,
	      0);
  if(err){
    kprintf("Fork error - %d", err);
    return err;
  }

  P(fp->sem);
  kfree(fp->sem);
  kfree(fp);
  if(*pret < 1){
    kprintf("Too many processes? %d", *pret);
    return ENPROC;
  }
  return 0;
}
