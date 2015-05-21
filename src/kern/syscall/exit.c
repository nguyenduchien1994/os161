#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <proc.h>
#include <synch.h>
#include <linkedlist.h>
#include <current.h>

void _exit(int exitcode)
{
  Linked_List *files = curproc->open_files->files; 
  Linked_List_Node *runner = files->first;

  while(runner != NULL)
  {
    open_file_decref(runner->data);
    runner = runner->next;
  }
  curproc->exit_status = exitcode;
  curproc->cur_state = dead;
  lock_acquire(curproc->exit_lock);
  cv_broadcast(curproc->exit_cv, curproc->exit_lock);
  while(curproc->wait_count > 0)
  {
    lock_release(curproc->exit_lock);
    thread_yield();
    lock_acquire(curproc->exit_lock);
  }
  lock_release(curproc->exit_lock);

  thread_exit();
}
