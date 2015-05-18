#include <types.h>
#include <syscall.h>

int dup2(int oldfd, int newfd, int *ret)
{
  (void)ret;
  (void)oldfd;
  (void)newfd;
  return 0;
}
