#include <types.h>
#include <syscall.h>

int open(const char *filename, int flags)
{
  (void)filename;
  (void)flags;
  return 0;
}
