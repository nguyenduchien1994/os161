#include <types.h>
#include <syscall.h>

int execv(const char *program, char **args)
{
  (void)program;
  (void)args;
  return 0;
}
