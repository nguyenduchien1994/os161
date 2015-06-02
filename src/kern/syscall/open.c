#include <types.h>
#include <spl.h>
#include <syscall.h>
#include <copyinout.h>
#include <vfs.h>
#include <usr_file.h>
#include <proc.h>
#include <current.h>
#include <kern/fcntl.h>
#include <kern/errno.h>
#include <vnode.h>

int open(const char *filename, int flags)
{
  
   if (filename == NULL)                                                                                    
    {
      return EFAULT;                                                                                                        }

   lock_acquire(glbl_mngr->file_sys_lk);

   void *namedest = kmalloc(sizeof(filename));
   if(namedest == NULL){
     kfree(namedest);
     lock_release(glbl_mngr->file_sys_lk);
     return ENOMEM;
   }

   int err = copyin((const_userptr_t)filename, namedest, sizeof(filename));

   if (err)
   {
     kfree(namedest);
     lock_release(glbl_mngr->file_sys_lk);
     return err;
   }


   // 3 eqauls invalid or if over 64 (1000000)
   if(((flags & 3) == 3) || flags < 1000000)
   {
     kfree(namedest);
     lock_release(glbl_mngr->file_sys_lk);
     return EINVAL;
   }

   mode_t mode = 0000;
   
   if(flags & O_RDONLY)
   {
     mode = 0444;
   } 
   else if(flags & O_WRONLY)
   {
     mode = 0222;
   }
   else if(flags & O_RDWR)
   {
     mode = 0666; 
   } 
   else
   {
     kfree(namedest);
     lock_release(glbl_mngr->file_sys_lk);
     return EINVAL;
   }


   struct vnode *file = kmalloc(sizeof(struct vnode));
   
   err = vfs_open(namedest, flags, mode, &file); 
  
   if (err)
   {
     kfree(namedest);
     kfree(file);
     lock_release(glbl_mngr->file_sys_lk);
     return err;
   }

   open_file *openfile = open_file_create(file, 0, flags); 
   err = file_list_add(curproc->open_files, openfile);   
   kfree(namedest);
   if(err){
     kfree(file);
   }

   lock_release(glbl_mngr->file_sys_lk);
   return err;
}
