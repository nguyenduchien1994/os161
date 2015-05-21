#include <types.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <linkedlist.h>
#include <synch.h>
#include <limits.h>

proc_mngr* proc_mngr_create(void)
{
  proc_mngr *ret = kmalloc(sizeof(proc_mngr));
  KASSERT(ret != NULL);
  
  ret->next_pid = 2;

  ret->procs = kmalloc(sizeof(proc*) * 32767);
  KASSERT(ret->procs != NULL);
  for(int i = 2; i < 32768; i++){
    *(ret->procs + i*(sizeof(proc*))) = NULL;
  }

  ret->threads = kmalloc(sizeof(struct thread*) * 32767);
  KASSERT(ret->threads != NULL);

  int turns[3];
  turns[kernel] = 10;
  turns[interactive] = 5;
  turns[background] = 1;
  ret->ready_queue = multi_queue_create(turns, 3);
  KASSERT(ret->ready_queue != NULL);

  ret->free_ids = stack_create();
  KASSERT(ret->free_ids != NULL);
  ret->free_ids->limit = PID_MAX-1;
  
  ret->glbl_lk = lock_create("proc lock");
  KASSERT(ret->glbl_lk != NULL);
  
  return ret;
}

void proc_mngr_destroy(proc_mngr *ptr)
{
  KASSERT(ptr != NULL);

  kfree(ptr->glbl_lk);
  kfree(ptr->free_ids);
  kfree(ptr->ready_queue);
  kfree(ptr->threads);
  kfree(ptr->procs);
  kfree(ptr);
}

int proc_mngr_add(proc_mngr *this, proc *p, struct thread *t)
{
  KASSERT(this != NULL);
  
  if(p != kproc){
    lock_acquire(this->glbl_lk);
  }

  pid_t pid;
  if(this->free_ids->first == NULL){
    if(this->next_pid <= 32767){
      pid = this->next_pid++;
      *(this->procs + pid*(sizeof(proc*))) = p;
      *(this->threads + pid*(sizeof(struct thread*))) = t;
    }
    else{
      return -1;
    }
  }
  else{
    pid = *((pid_t*)stack_pop(this->free_ids));
    *(this->procs + pid*(sizeof(proc*))) = p;
    *(this->threads + pid*(sizeof(struct thread*))) = t;
  }

  p->pid = pid;

  if(p != kproc){
    lock_release(this->glbl_lk);
  }
  
  return pid;
}

void proc_mngr_remove(proc_mngr *this, pid_t pid)
{
  *(this->procs + pid*(sizeof(proc*))) = NULL;
  
  int *to_push = kmalloc(sizeof(int));
  *to_push = pid;
  stack_push(this->free_ids, to_push);
}

struct thread* proc_mngr_get_thread(proc_mngr *this, proc *p)
{
  KASSERT(this != NULL);

  lock_acquire(this->glbl_lk);
  Linked_List_Node *runner = this->free_ids->first;
  while(runner != NULL){
    KASSERT(p->pid != *((pid_t*) runner->data));
  }

  struct thread* ret = *(this->threads + (p->pid)*sizeof(struct thread*));
  lock_release(this->glbl_lk);
  
  return ret;
}

struct lock* proc_mngr_get_lock(proc_mngr *this)
{
  KASSERT(this != NULL);
  return this->glbl_lk;
}

void proc_mngr_make_ready(proc_mngr *this, proc *p)
{
  KASSERT(this != NULL);
  
  lock_acquire(this->glbl_lk);

  multi_queue_add(this->ready_queue,p, (int)p->rt);
  p->cur_state = ready;
  
  lock_release(this->glbl_lk);
}

proc* proc_mngr_get_from_pid(proc_mngr *this, pid_t pid)
{
  KASSERT(this != NULL);
  
  if(pid < 2 || pid > (int)PID_MAX){
    kprintf("invalid pid");
    return NULL;
  }
  
  proc *ret =  *(this->procs + pid*(sizeof(proc*)));
  
  return ret;
}
