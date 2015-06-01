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
  // Error Checks                                                                                          

  if (pathname == NULL)                                                                                    
    {
      return EFAULT;                                                                                                           
    }    

  void* namedest = kmalloc(sizeof(pathname));                                                                               
  int err = copyin((const_userptr_t)pathname, namedest, sizeof(pathname));                                                   
                                                                                                                             
  if (err != 0)                                                                                              
  {
    kfree(namedest);
    return err;                                                                                                            
  }                                                                
  //set_p_cwd(userproc, dest);                                                                                            
  
  err = vfs_chdir(namedest);
  kfree(namedest);
  if (err)
  {
      return err;
  }
  return 0;                           
}
