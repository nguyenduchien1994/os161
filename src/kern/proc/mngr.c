#include <types.h>
#include <proc.h>
#include <thread.h>
#include <linkedlist.h>


proc_mngr* proc_mngr_create(void)
{
  return NULL;
}

void proc_mngr_destroy(proc_mngr *ptr)
{
  (void)ptr;
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
