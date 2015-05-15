#include <types.h>
#include <syscall.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
  (void)pid;
  (void)status;
  (void)options;
  return 0;
}
