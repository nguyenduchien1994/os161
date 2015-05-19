#include <types.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include <copyinout.h>
#include <limits.h>

int execv(const char *program, char **args)
{

   if (program == NULL)                                                                                    
    {
      return ENOENT;                                                                                                           
    }

   char** progdest = kmalloc(sizeof(ARG_MAX));
   int err = copyin((const_userptr_t)program, progdest, sizeof(program));

   if (err)
   {
      return EIO;
   }

   bool keep_going = true;
   int nargs = 0;
   int runner = 0;
   int argno = 0;
   char *curarg = progdest[argno];
   
   while(keep_going)
   {
     if(progdest[runner] == '\0')
     {
       nargs = nargs + 1;
       if(progdest[runner + 1] == '\0')
       {
	 keep_going = false;
       }
       argno = argno + 1;
       curarg = progdest[argno];
     }
     else
     {
       runner = runner + 1;
     }
   }

        struct addrspace *as;
        struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open((char*)program, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	/* Warp to user mode. */
	enter_new_process(nargs /*argc*/, (userptr_t)&args /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;

}
