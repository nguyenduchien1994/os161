#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <proc.h>
#include <synch.h>
#include <linkedlist.h>
#include <current.h>

void _exit(int exitcode)
{
  lock_acquire(curproc->exit_lock);

  Linked_List *files = curproc->open_files->files; 
  Linked_List_Node *runner = files->first;

  while(runner != NULL)
  {
    open_file_decref(runner->data);
    runner = runner->next;
  }

  runner = curproc->children->first;
  while(runner != NULL){
    ((proc*)runner->data)->parent = NULL;
    runner = runner->next;
  }
  
  if(curproc->parent){
    runner = curproc->parent->children->first;
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

  curproc->exit_status = exitcode;
  curproc->cur_state = dead;
  cv_broadcast(curproc->exit_cv, curproc->exit_lock);
  P(curproc->exit_sem);
  lock_release(curproc->exit_lock);

  thread_exit();
}
