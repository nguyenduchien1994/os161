#include <types.h>
#include <kern/seek.h>
#include <syscall.h>
#include <proc.h>
#include <current.h>
#include <vnode.h>
#include <vfs.h>
#include <stat.h>

int lseek(int fd, off_t pos, int whence, off_t *ret)
{
  (void)ret;
  //splhigh();
  open_file *f = file_list_get(curproc->open_files,fd);
  if(f == NULL){
    //errno = EBADF;
    return -1;
  }
  if(!f->vfile->vn_ops->vop_isseekable(f->vfile)){
    //errno = ESPIPE;
    return -1;
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
    //errno = EINVAL;
    return -1;
  }

  if(init < 0){
    //errno = EINVAL;
    return -1;
  }

  f->offset = init;
  //spl0();
  return f->offset;
}
