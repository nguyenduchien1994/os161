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

/*
 * Process support.
 *
 * There is (intentionally) not much here; you will need to add stuff
 * and maybe change around what's already present.
 *
 * p_lock is intended to be held when manipulating the pointers in the
 * proc structure, not while doing any significant work with the
 * things they point to. Rearrange this (and/or change it to be a
 * regular lock) as needed.
 *
 * Unless you're implementing multithreaded user processes, the only
 * process that will have more than one thread is the kernel process.
 */

#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <linkedlist.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <machine/trapframe.h>
#include <test.h>

/*
 * The process for the kernel; this holds all the kernel-only threads.
 */
struct proc *kproc;

/*
 * Create a proc structure.
 */
proc *
proc_create(const char *name)
{
	struct proc *proc;

	proc = kmalloc(sizeof(*proc));
	if (proc == NULL) {
		return NULL;
	}
	proc->p_name = kstrdup(name);
	if (proc->p_name == NULL) {
		kfree(proc);
		return NULL;
	}

	threadarray_init(&proc->p_threads);
	spinlock_init(&proc->p_lock);

	/* VM fields */
	proc->p_addrspace = NULL;

	/* VFS fields */
	proc->p_cwd = NULL;
	
	proc->pid = 0;
	proc->context = NULL;
	proc->parent = NULL;
	
	// Complete
	proc->program_counter = 0;
	proc->cur_state = new;
	proc->rt = BACKGROUND_K;

	proc->open_files = file_list_create();
	if(proc->open_files == NULL){
	  kfree(proc->p_name);
	  kfree(proc);
	}
	
	proc->children = linkedlist_create();
	if(proc->children == NULL){
	  file_list_destroy(proc->open_files);
	  kfree(proc->p_name);
	  kfree(proc);
	}

	proc->exit_lock = lock_create("exit");
	if(proc->exit_lock == NULL){
	  linkedlist_destroy(proc->children);
	  file_list_destroy(proc->open_files);
	  kfree(proc->p_name);
	  kfree(proc);
	}
	proc->exit_cv = cv_create("exit");
	if(proc->exit_cv == NULL){
	  lock_destroy(proc->exit_lock);
	  linkedlist_destroy(proc->children);
	  file_list_destroy(proc->open_files);
	  kfree(proc->p_name);
	  kfree(proc);
	}
	proc->exit_sem = sem_create("exit", 1);
	if(proc->exit_sem == NULL){
	  cv_destroy(proc->exit_cv);
	  lock_destroy(proc->exit_lock);
	  linkedlist_destroy(proc->children);
	  file_list_destroy(proc->open_files);
	  kfree(proc->p_name);
	  kfree(proc);
	}

	return proc;
}

/*
 * Destroy a proc structure.
 *
 * Note: nothing currently calls this. Your wait/exit code will
 * probably want to do so.
 */
void proc_destroy(struct proc *proc)
{ 
	/*
	 * You probably want to destroy and null out much of the
	 * process (particularly the address space) at exit time if
	 * your wait/exit design calls for the process structure to
	 * hang around beyond process exit. Some wait/exit designs
	 * do, some don't.
	 */

        KASSERT(proc != NULL);
	KASSERT(proc != kproc);

	/*
	 * We don't take p_lock in here because we must have the only
	 * reference to this structure. (Otherwise it would be
	 * incorrect to destroy it.)
	 */

	/* VFS fields */
	if (proc->p_cwd) {
		VOP_DECREF(proc->p_cwd);
		proc->p_cwd = NULL;
	}

	/* VM fields */
	if (proc->p_addrspace) {
		/*
		 * If p is the current process, remove it safely from
		 * p_addrspace before destroying it. This makes sure
		 * we don't try to activate the address space while
		 * it's being destroyed.
		 *
		 * Also explicitly deactivate, because setting the
		 * address space to NULL won't necessarily do that.
		 *
		 * (When the address space is NULL, it means the
		 * process is kernel-only; in that case it is normally
		 * ok if the MMU and MMU- related data structures
		 * still refer to the address space of the last
		 * process that had one. Then you save work if that
		 * process is the next one to run, which isn't
		 * uncommon. However, here we're going to destroy the
		 * address space, so we need to make sure that nothing
		 * in the VM system still refers to it.)
		 *
		 * The call to as_deactivate() must come after we
		 * clear the address space, or a timer interrupt might
		 * reactivate the old address space again behind our
		 * back.
		 *
		 * If p is not the current process, still remove it
		 * from p_addrspace before destroying it as a
		 * precaution. Note that if p is not the current
		 * process, in order to be here p must either have
		 * never run (e.g. cleaning up after fork failed) or
		 * have finished running and exited. It is quite
		 * incorrect to destroy the proc structure of some
		 * random other process while it's still running...
		 */
		struct addrspace *as;

		if (proc == curproc) {
			as = proc_setas(NULL);
			as_deactivate();
		}
		else {
			as = proc->p_addrspace;
			proc->p_addrspace = NULL;
		}
		as_destroy(as);
	}


	file_list_destroy(proc->open_files);
	linkedlist_destroy(proc->children);

	spinlock_cleanup(&proc->p_lock);
	
	lock_destroy(proc->exit_lock);
	cv_destroy(proc->exit_cv);
	sem_destroy(proc->exit_sem);

	kfree(proc->p_name);
	kfree(proc);
}

/*
 * Partially destroy a process, except for the exit status, locks and memory deallocation
 */
void proc_shutdown(struct proc *proc)
{ 
        KASSERT(proc != NULL);
	KASSERT(proc != kproc);

	/* VFS fields */
	if (proc->p_cwd) {
		VOP_DECREF(proc->p_cwd);
		proc->p_cwd = NULL;
	}

	/* VM fields */
	if (proc->p_addrspace) {
		struct addrspace *as;

		if (proc == curproc) {
			as = proc_setas(NULL);
			as_deactivate();
		}
		else {
			as = proc->p_addrspace;
			proc->p_addrspace = NULL;
		}
		as_destroy(as);
	}

	Linked_List_Node *runner = proc->children->first;
	while(runner != NULL)
	{
	  ((struct proc*)runner->data)->parent = NULL;
	  runner = runner->next;
	}

	file_list_destroy(proc->open_files);
	linkedlist_destroy(proc->children);

	kfree(proc->context);
}

void usr_proc_destroy (struct proc *proc)
{
  spinlock_cleanup(&proc->p_lock);

  lock_destroy(proc->exit_lock);
  cv_destroy(proc->exit_cv);
  sem_destroy(proc->exit_sem);
       
  kfree(proc->p_name);
  kfree(proc);
}

/*
 * Create the process structure for the kernel.
 */
void
proc_bootstrap(void)
{
	kproc = proc_create("[kernel]");
	if (kproc == NULL) {
		panic("proc_create for kproc failed\n");
	}
}

/*
 * Create a fresh proc for use by runprogram.
 *
 * It will have no address space and will inherit the current
 * process's (that is, the kernel menu's) current directory.
 */
struct proc *
proc_create_runprogram(const char *name)
{
	struct proc *newproc;

	newproc = proc_create(name);
	if (newproc == NULL) {
		return NULL;
	}

	/* VM fields */

	newproc->p_addrspace = NULL;

	/* VFS fields */

	/*
	 * Lock the current process to copy its current directory.
	 * (We don't need to lock the new process, though, as we have
	 * the only reference to it.)
	 */
	spinlock_acquire(&curproc->p_lock);
	if (curproc->p_cwd != NULL) {
		VOP_INCREF(curproc->p_cwd);
		newproc->p_cwd = curproc->p_cwd;
	}
	spinlock_release(&curproc->p_lock);
	
	if(glbl_mngr == NULL){
	  glbl_mngr = proc_mngr_create();
	  KASSERT(glbl_mngr != NULL);
	  proc_mngr_add(glbl_mngr, curproc, curthread);
	  user_init = true;
	} 


	if (kproc != NULL && kproc->open_files->files->first == NULL) {  
	  struct vnode *console_node = kmalloc(sizeof(struct vnode));
	  
	  int err = vfs_open((char*)"con:", O_RDWR, 0664, &console_node); 
	  
	  if (err)
	    {
	      panic("Could not access console....Users are deaf and mute...");
	    }
	  
	  open_file *openfile = open_file_create(console_node, 0, O_RDONLY);
	  file_list_add(kproc->open_files, openfile); //making STD_IN = 0
	  
	  openfile = open_file_create(console_node, 0, O_WRONLY); 
	  file_list_add(kproc->open_files, openfile); //making STD_OUT = 1
	  file_list_add(kproc->open_files, openfile); //making STD_ERR = 2
	}

	Linked_List_Node *runner = kproc->open_files->files->last;
	while(runner != NULL){
	  if(linkedlist_insert(newproc->open_files->files, runner->key, runner->data)){
	    panic("Out of memory without user program running, you've got a leak");
	  }
	  open_file_incref((open_file*)runner->data);
	  runner = runner->prev;
	}
	
	runner = kproc->open_files->available->last;
	while(runner != NULL){
	  if(linkedlist_insert(newproc->open_files->available, runner->key, runner->data))
	    panic("Out of memory without user program running, you've got a leak");
	  runner = runner->prev;
	}
	newproc->parent = kproc;
	if(linkedlist_append(kproc->children, newproc))
	  panic("Out of memory without user program running, you've got a leak");

	return newproc;
}

/*
 * Add a thread to a process. Either the thread or the process might
 * or might not be current.
 *
 * Turn off interrupts on the local cpu while changing t_proc, in
 * case it's current, to protect against the as_activate call in
 * the timer interrupt context switch, and any other implicit uses
 * of "curproc".
 */
int
proc_addthread(struct proc *proc, struct thread *t)
{
	int result;
	int spl;

	KASSERT(t->t_proc == NULL);

	spinlock_acquire(&proc->p_lock);
	result = threadarray_add(&proc->p_threads, t, NULL);
	spinlock_release(&proc->p_lock);
	if (result) {
		return result;
	}
	spl = splhigh();
	t->t_proc = proc;
	splx(spl);
	return 0;
}

/*
 * Remove a thread from its process. Either the thread or the process
 * might or might not be current.
 *
 * Turn off interrupts on the local cpu while changing t_proc, in
 * case it's current, to protect against the as_activate call in
 * the timer interrupt context switch, and any other implicit uses
 * of "curproc".
 */
void
proc_remthread(struct thread *t)
{
	struct proc *proc;
	unsigned i, num;
	int spl;

	proc = t->t_proc;
	KASSERT(proc != NULL);

	spl = splhigh();
	/* ugh: find the thread in the array */
	num = threadarray_num(&proc->p_threads);
	for (i=0; i<num; i++) {
		if (threadarray_get(&proc->p_threads, i) == t) {
			threadarray_remove(&proc->p_threads, i);
		       
			t->t_proc = NULL;
			splx(spl);
			return;
		}
	}
	/* Did not find it. */
	splx(spl);
	panic("Thread (%p) has escaped from its process (%p)\n", t, proc);
}

/*
 * Fetch the address space of (the current) process.
 *
 * Caution: address spaces aren't refcounted. If you implement
 * multithreaded processes, make sure to set up a refcount scheme or
 * some other method to make this safe. Otherwise the returned address
 * space might disappear under you.
 */
struct addrspace *
proc_getas(void)
{
	struct addrspace *as;
	struct proc *proc = curproc;

	if (proc == NULL) {
		return NULL;
	}

	spinlock_acquire(&proc->p_lock);
	as = proc->p_addrspace;
	spinlock_release(&proc->p_lock);
	return as;
}

/*
 * Change the address space of (the current) process. Return the old
 * one for later restoration or disposal.
 */
struct addrspace *
proc_setas(struct addrspace *newas)
{
	struct addrspace *oldas;
	struct proc *proc = curproc;

	KASSERT(proc != NULL);

	spinlock_acquire(&proc->p_lock);
	oldas = proc->p_addrspace;
	proc->p_addrspace = newas;
	spinlock_release(&proc->p_lock);
	return oldas;
}

void set_p_cwd(proc * p, struct vnode * new)
{
  vnode_decref(p->p_cwd);
  p->p_cwd = new;
  vnode_incref(new);
}

void set_state(proc *p, state s)
{
  p->cur_state = s;
}

void copy_context(struct trapframe* tf){
  memcpy(tf, curproc->context, sizeof(struct trapframe));
}

proc* proc_copy(void)
{
  KASSERT(curproc != NULL);
  struct proc *ret;
  
  ret = kmalloc(sizeof(proc));
  if (ret == NULL) {
    return NULL;
  }
  ret->p_name = kstrdup(curproc->p_name);
  if (ret->p_name == NULL) {
    kfree(ret);
    return NULL;
  }
  
  threadarray_init(&ret->p_threads);
  spinlock_init(&ret->p_lock);
  
  /* VM fields */
  ret->p_addrspace = NULL;
  
  /* VFS fields */
  if(curproc->p_cwd){
    VOP_INCREF(curproc->p_cwd);  
  }
  ret->p_cwd = curproc->p_cwd;
  
  ret->rt = curproc->rt;
  
  ret->pid = 0;
  //copy
  
  ret->context = kmalloc(sizeof(struct trapframe));
  if(ret->context == NULL){
    kfree(ret->p_name);
    kfree(ret);
  }
  copy_context(ret->context);
  ret->parent = NULL;
  
  // Complete
  ret->cur_state = curproc->cur_state;
  
  ret->open_files = file_list_create();
  if(ret->open_files == NULL){
    kfree(ret->p_name);
    kfree(ret);
  }
  
  lock_acquire(curproc->open_files->files->lk);
  Linked_List_Node *runner = curproc->open_files->files->last;
  while(runner != NULL){
    if(linkedlist_insert(ret->open_files->files, runner->key, runner->data)){
      lock_release(curproc->open_files->files->lk);
      file_list_destroy(ret->open_files);
      kfree(ret->p_name);
      kfree(ret);
      return NULL;
    }
    open_file_incref((open_file*)runner->data);
    runner = runner->prev;
  }
  lock_release(curproc->open_files->files->lk);

  lock_acquire(curproc->open_files->available->lk);
  runner = curproc->open_files->available->last;
  while(runner != NULL){
    if(linkedlist_insert(ret->open_files->available, runner->key, runner->data)){
      lock_release(curproc->open_files->available->lk);
      file_list_destroy(ret->open_files);
      kfree(ret->p_name);
      kfree(ret);
      return NULL;
    }
    runner = runner->prev;
  }
  lock_release(curproc->open_files->available->lk);
  
  ret->children = linkedlist_create();
  if(ret->children == NULL){
    kfree(ret->open_files);
    kfree(ret->p_name);
    kfree(ret);
  }
  
  ret->exit_lock = lock_create("exit");
  if(ret->exit_lock == NULL){
    kfree(ret->children);
    kfree(ret->open_files);
    kfree(ret->p_name);
    kfree(ret);
  }
  ret->exit_cv = cv_create("exit");
  if(ret->exit_cv == NULL){
    kfree(ret->exit_lock);
    kfree(ret->children);
    kfree(ret->open_files);
    kfree(ret->p_name);
    kfree(ret);
  }  

  ret->exit_sem = sem_create("exit", 1);
  if(ret->exit_lock == NULL){
    kfree(ret->exit_cv);
    kfree(ret->exit_lock);
    kfree(ret->children);
    kfree(ret->open_files);
    kfree(ret->p_name);
    kfree(ret);
  }

  return ret;

}
