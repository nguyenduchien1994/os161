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
   if(progdest == NULL)
     return ENOMEM;

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
     if(progdest[runner + 1] == NULL)
     {
       keep_going = false;
     }
     else if(progdest[runner] == '\0')
     {
       nargs++;
       argno = argno + 1;
       curarg = progdest[argno];
     }
     else
     {
       runner = runner + 1;
     }
   }
   kfree(progdest);

   lock_acquire(glbl_mngr->proc_sys_lk);

   struct addrspace *as;
   struct vnode *v;
   vaddr_t entrypoint, stackptr;
   int result;
   
   /* Open the file. */
   result = vfs_open((char*)program, O_RDONLY, 0, &v);
   if (result) {
     lock_release(glbl_mngr->proc_sys_lk);
     return result;
   }
   
   /* We should be a new process. */
   KASSERT(proc_getas() == NULL);
   
   /* Create a new address space. */
   as = as_create();
   if (as == NULL) {
     vfs_close(v);
     lock_release(glbl_mngr->proc_sys_lk);
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
     lock_release(glbl_mngr->proc_sys_lk);
     return result;
   }
   
   /* Done with the file now. */
   vfs_close(v);
   
   /* Define the user stack in the address space */
   result = as_define_stack(as, &stackptr);
   lock_release(glbl_mngr->proc_sys_lk);

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
