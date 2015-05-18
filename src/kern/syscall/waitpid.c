#include <types.h>
#include <syscall.h>

int waitpid(pid_t pid, int *status, int options, pid_t *ret)
{
  (void)pid;
  (void)status;
  (void)options;
  *ret = pid;
  return 0;
}
