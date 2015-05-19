#include <types.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <linkedlist.h>
#include <synch.h>


proc_mngr* proc_mngr_create(void)
{
  proc_mngr *ret = kmalloc(sizeof(proc_mngr));
  KASSERT(ret != NULL);
  
  ret->usr_procs = kmalloc(sizeof(proc*) * 256);
  KASSERT(ret->usr_procs != NULL);

  ret->threads = kmalloc(sizeof(struct thread*) * 256);
  KASSERT(ret->threads != NULL);

  int turns[2] = {3,1};
  ret->ready_queue = multi_queue_create(turns, 2);
  KASSERT(ret->ready_queue != NULL);

  ret->free_ids = stack_create();
  KASSERT(ret->free_ids != NULL);

  ret->glbl_lk = lock_create("proc run");
  KASSERT(ret->glbl_lk != NULL);
  
  return ret;
}

void proc_mngr_destroy(proc_mngr *ptr)
{
  kfree(ptr->glbl_lk);
  kfree(ptr->free_ids);
  kfree(ptr->ready_queue);
  kfree(ptr->threads);
  kfree(ptr->usr_procs);
  kfree(ptr);
}

int proc_mngr_add(proc_mngr *this, proc *p, struct thread *t)
{
  (void)this;
  (void)p;
  (void)t;
  return 0;
}

void proc_mngr_remove(proc_mngr *this, proc *p)
{
  (void)this;
  (void)p;
}

struct thread* proc_mngr_get_thread(proc_mngr *this, proc *p)
{
  (void)this;
  (void)p;
  return NULL;
}

proc* proc_mngr_get_proc(proc_mngr *this, struct thread *t)
{
  (void)this;
  (void)t;
  return NULL;
}

void proc_mngr_get_lock(proc_mngr *this)
{
  (void)this;
}

void proc_mngr_make_ready(proc_mngr *this, proc *p)
{
  (void)this;
  (void)p;
}

void proc_mngr_make_wait(proc_mngr *this, proc *p)
{
  (void)this;
  (void)p;
}
