#include <types.h>
#include <syscall.h>
#include <lib.h>
#include <copyinout.h>

ssize_t write(int fd, const void *buf, size_t nbytes) 
{
  (void)fd;
  
  void *dest = kmalloc(nbytes);
  copyin((const_userptr_t)buf, dest, nbytes);

  for(unsigned i = 0; i < nbytes; i++){
    kprintf(buf+i);
  }

  kfree(dest);
  return 1;
}
