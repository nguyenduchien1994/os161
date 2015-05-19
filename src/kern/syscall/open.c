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
      return ENOENT;                                                                                                           
    }

   void* namedest = kmalloc(sizeof(filename));
   int err = copyin((const_userptr_t)filename, namedest, sizeof(filename));

   if (err != 0)
   {
      return EIO;
   }

   struct vnode *file = kmalloc(sizeof(struct vnode));
   struct vnode **ret = &file;
  
   
  mode_t mode;

  if(flags == O_RDONLY)
    {
      mode = 0444;
    } 
  else if(flags == O_WRONLY)
    {
      mode =0222;
    }
  else if(flags == O_RDWR)
    {
      mode = 0666;
    }
  else
    {
      mode = 0000;
      //return error?
    }

    err = vfs_open(namedest, flags, mode, ret); 
  
    if (err)
    {
      return err;
    }

    open_file *openfile = open_file_create(file, 0, flags); 
  
    err = file_list_add(curproc->open_files, openfile);

    return err;
  
}
