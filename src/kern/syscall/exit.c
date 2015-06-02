#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <proc.h>
#include <synch.h>
#include <linkedlist.h>
#include <current.h>

void _exit(int exitcode)
{
  lock_acquire(glbl_mngr->proc_sys_lk);

  proc_shutdown(curproc);
  curproc->cur_state = dead; 

  lock_acquire(curproc->exit_lock);

  if(curproc->parent){
    Linked_List_Node *runner = curproc->parent->children->first;
    bool found_in_parent = false;
    while(runner != NULL && !found_in_parent){
      if(runner->data == curproc){
	linkedlist_remove(curproc->parent->children, runner->key);
	found_in_parent = true;
      }
      else{
	runner = runner->next;
      }
    }
    KASSERT(found_in_parent);
  }
  lock_release(glbl_mngr->proc_sys_lk);

  curproc->exit_status = exitcode;
  cv_broadcast(curproc->exit_cv, curproc->exit_lock);
  P(curproc->exit_sem);
  lock_release(curproc->exit_lock);

  thread_exit();
}
