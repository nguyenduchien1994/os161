#include <types.h>
#include <syscall.h>

int read(int fd, void *buf, size_t buflen, ssize_t *ret)
{
  (void)ret;
  (void)fd;
  (void)buf;
  (void)buflen;
  return 0;
}
