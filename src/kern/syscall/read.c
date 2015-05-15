#include <types.h>
#include <syscall.h>

int read(int fd, void *buf, size_t buflen)
{
  (void)fd;
  (void)buf;
  (void)buflen;
  return 0;
}
