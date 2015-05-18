#include <types.h>
#include <spl.h>
#include <syscall.h>
#include <proc.h>
#include <copyinout.h>
#include <vfs.h>
#include <kern/errno.h>
#include <current.h>
#include <vnode.h>

int chdir(const char *pathname)
{
  /*
  spinlock_acquire(&syscall_lock);

  // Error Checks
  if (pathname == NULL)
    {
      spinlock_release(&syscall_lock);
      // return -1;
       return ENOENT;
    }
  else 
    {
      KASSERT(curthread != NULL);

      //get proc from global manager 
      proc * userproc;
      userproc = proc_mngr_get_proc(glbl_mngr, curthread);   
 
      KASSERT(userproc != NULL);
      KASSERT(userproc->cur_state == running);
      
      // need to use look up and make pathname to a vnode 
      // copy in pathname 
      // error code  = proc-> cur_dir-> vops->vop_loopup(cur_dir, pathname, dest)
      // dest = empty pointer Kmalloc it 
      //if (no error)
      // proc -> cur_dir = dest
      // return 0
      // else
      // return err code
     
      
      //if not 0 return EIO
      void* namedest = kmalloc(sizeof(pathname));
      int err = copyin((const_userptr_t)pathname, namedest, sizeof(pathname));

      if (err != 0) 
	{
	  spinlock_release(&syscall_lock);
	  erno = err;
	  return -1;
	}
	     
      void* dest = kmalloc(sizeof(struct vnode));
      
      err = userproc->p_cwd->vn_ops->vop_lookup(userproc->p_cwd, namedest, dest);

      if (err !=0)
	{
	  spinlock_release(&syscall_lock);
	  return err;
	}
      else
	{
	  set_p_cwd(userproc, dest);
	  vfs_chdir(namedest);
	  spinlock_release(&syscall_lock);
	  return 0;
	}
    }
*/
  splhigh();

  void* namedest = kmalloc(sizeof(pathname));                                                                               
  int err = copyin((const_userptr_t)pathname, namedest, sizeof(pathname));                                                  
                                                                                                                                
  if (err != 0)                                                                                              
    {
      // errno = err;
      return -1;                                                                                                            
    }                                                                
  //set_p_cwd(userproc, dest);                                                                                            
  vfs_chdir(namedest);                                                                                                  
  spl0();
  return 0;                           
}
