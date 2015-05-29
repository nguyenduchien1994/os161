/*
 * Copyright (c) 2013
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PROC_H_
#define _PROC_H_

/*
 * Definition of a process.
 *
 * Note: curproc is defined by <current.h>.
 */

#include <spinlock.h>
#include <thread.h> /* required for struct threadarray */
#include <synch.h>
#include <linkedlist.h>
#include <usr_file.h>

struct addrspace;
struct vnode;

typedef int pc_t;

typedef enum state{
              running,
	      ready,
	      waiting,
	      new,
	      dead
} state;

typedef enum run_type{
  kernel,
  interactive,
  background
} run_type;

/*
 * Process structure.
 */
typedef struct proc {
  char *p_name;			/* Name of this process */
  struct spinlock p_lock;		/* Lock for this structure */
  struct threadarray p_threads;	/* Threads in this process */
  
  /* VM */
  struct addrspace *p_addrspace;	/* virtual address space */
  
  /* VFS */
  struct vnode *p_cwd;		/* current working directory */
  
  
  pid_t pid;
  pc_t program_counter;
  state cur_state;
  struct trapframe *context;
  file_list *open_files;
  struct proc *parent;
  Linked_List *children;
  
  /* for waitpid and _exit */
  int exit_status;
  struct lock *exit_lock;
  struct cv *exit_cv;
  struct semaphore *exit_sem;
  
  run_type rt;

} proc;

/*
 * Process API
 * 
 * proc_create
 *     
 * 
 * proc_destroy
 * 
 *      
 * proc_copy
 *     
 * 
 * set_state
 *     Sets the given process' state to the given state.
 * 
 * set_p_cwd
 *     Sets the given process' current working directory to the given directory 
 */
proc* proc_create(const char *name);
void proc_destroy(struct proc *proc);
proc* proc_copy(void);

void set_state(proc *p, state s);

void set_p_cwd(proc *p, struct vnode *new);

void copy_context(struct trapframe *tf);

/*
 * Global manager to handle all user processes. Maps user process to kernel thread
 *
 */
#define MAX_PROCESSES = 256

typedef struct proc_mngr{
  proc **procs;//256 array
  struct thread **threads;//256 array
  multi_queue *ready_queue;
  stack *free_ids;
  struct lock *glbl_lk;
  int next_pid;
} proc_mngr;

proc_mngr *glbl_mngr;

struct spinlock syscall_lock;

/*
 * Global manager API
 * All functions assume given manager and processes and threads are not NULL
 *
 * proc_mngr_add 
 *      inserts the new process into the manager, mapping it to the given thread.
 *      Assumes: thread and process not already in manager. 
 *      Returns: 0 if not added (no space), 1 otherwise
 * 
 * proc_mngr_remove
 *      removes the given process from the manager and deallocates it.
 *      Assumes: process is "dead," thread and process are in manager
 * 
 * proc_mngr_get_thread
 *      Gets the thread associated with the given process.
 *      Assumes: process and thread in manager
 *      Returns: proper kernel thread
 * 
 * proc_mngr_get_proc
 *      Gets the user process associated with the given thread.
 *      Assumes: process and thread in manager
 *      Returns: proper user process
 * 
 * proc_mngr_get_lock
 *      Gets the global lock used for all processes (on run)
 *      Returns: the global lock
 * 
 * proc_mngr_make_ready
 *      Makes the given proc ready and queues for run.
 *      Assumes: proc is not already ready or dead
 *
 * proc_mngr_make_wait
 *      Makes the given process state "waiting," not in queue
 *      Assumes: current process is "running,"
 *      
 * 
 */

proc_mngr* proc_mngr_create(void);
void proc_mngr_destroy(proc_mngr *ptr);

int proc_mngr_add(proc_mngr *this, proc *p, struct thread *t);
void proc_mngr_remove(proc_mngr *this, pid_t p);

struct thread* proc_mngr_get_thread(proc_mngr *this, proc *p);
struct lock* proc_mngr_get_lock(proc_mngr *this);

proc* proc_mngr_get_from_pid(proc_mngr *this, pid_t pid);

void proc_mngr_make_ready(proc_mngr *this, proc *p);


/* This is the process structure for the kernel and for kernel-only threads. */
extern struct proc *kproc;

/* Call once during system startup to allocate data structures. */
void proc_bootstrap(void);

/* Create a fresh process for use by runprogram(). */
struct proc *proc_create_runprogram(const char *name);

/* Destroy a process. */
void proc_destroy(struct proc *proc);

/* Attach a thread to a process. Must not already have a process. */
int proc_addthread(struct proc *proc, struct thread *t);

/* Detach a thread from its process. */
void proc_remthread(struct thread *t);

/* Fetch the address space of the current process. */
struct addrspace *proc_getas(void);

/* Change the address space of the current process, and return the old one. */
struct addrspace *proc_setas(struct addrspace *);


#endif /* _PROC_H_ */
