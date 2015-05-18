#include <types.h>
#include <spl.h>
#include <syscall.h>
#include <copyinout.h>
#include <vfs.h>
#include <usr_file.h>
#include <proc.h>
#include <current.h>
#include <kern/fcntl.h>

int open(const char *filename, int flags)
{

  //spinlock_acquire(&syscall_lock);
  splhigh();
  void* namedest = kmalloc(sizeof(filename));
  int err = copyin((const_userptr_t)filename, namedest, sizeof(filename));

  if (err != 0)
    {
      spl0();
      return -1;
      // spinlock_release(&syscall_lock);
      //return EIO;
    }

  struct vnode *file;
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

  int toReturn = vfs_open(namedest, flags, mode, ret); 
  
  if (toReturn != 0)
    {
      spl0();
      return -1;
    }

  off_t off = 0;
  open_file *openfile = open_file_create(file, off); 
  
  err = file_list_add(curproc->open_files, openfile);

  if (err != 0)
    {
      spl0();
      return -1;
    }

  //spinlock_release(&syscall_lock);
  spl0();
  return toReturn;
}
