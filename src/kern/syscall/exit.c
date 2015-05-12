#include <types.h>
#include <syscall.h>
#include <thread.h>

void _exit(int exitcode)
{
  (void)exitcode;

  thread_exit();
}
