#include <types.h>
#include <syscall.h>

int close(int fd)
{
  (void)fd;
  return 0;
}
