#include <types.h>
#include <syscall.h>
#include <synch.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{
  if(pid != NULL)
  {
    lockacquire(exit_lock);
    wait_count ++;

  } 
  
  return 0;
}
