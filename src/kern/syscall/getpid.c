#include <types.h>
#include <spl.h>
#include <syscall.h>
#include <proc.h>
#include <current.h>

int getpid(pid_t *ret)
{  
  *ret = curproc->pid;
  // spinlock_release(&syscall_lock);
  //spl0();
  return 0;
}
