#include <types.h>
#include <kern/seek.h>
#include <kern/errno.h>
#include <syscall.h>
#include <proc.h>
#include <current.h>
#include <vnode.h>
#include <vfs.h>
#include <stat.h>
#include <limits.h>

int lseek(int fd, off_t pos, int whence, off_t *ret)
{
  if (fd < 0 || fd >= OPEN_MAX)
  {
    return EBADF;
  }

  open_file *f = file_list_get(curproc->open_files,fd);
  if(f == NULL)
  {
    return EBADF;
  }
  
  if(!f->vfile->vn_ops->vop_isseekable(f->vfile))
  {
    return ESPIPE;
  }

  off_t init = f->offset;
  if(whence == SEEK_SET){
    init = pos;

  }else if(whence == SEEK_CUR){
    init = init + pos;

  }else if(whence == SEEK_END){
    struct stat *statbuf = kmalloc(sizeof(struct stat));
    f->vfile->vn_ops->vop_stat(f->vfile, statbuf);
    init = statbuf->st_size - pos;
    
  }else{
    *ret = -1;
    return EINVAL;
  }

  if(init < 0){
    *ret = -1;
    return EINVAL;
  }

  f->offset = init;
  *ret = init;
  return 0;
}
