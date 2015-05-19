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
      return ENOENT;                                                                                                           
    }    

  void* namedest = kmalloc(sizeof(pathname));                                                                               
  int err = copyin((const_userptr_t)pathname, namedest, sizeof(pathname));                                                   
                                                                                                                             
  if (err != 0)                                                                                              
    {
      // errno = err;
      return err;                                                                                                            
    }                                                                
  //set_p_cwd(userproc, dest);                                                                                            
  vfs_chdir(namedest);                                                                                                  
  return 0;                           
}
