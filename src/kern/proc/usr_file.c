#include <types.h>
#include <lib.h>
#include <usr_file.h>
#include <linkedlist.h>
#include <vnode.h>
#include <synch.h>


open_file* open_file_create(struct vnode *file, off_t init_offset)
{
  open_file *ret = kmalloc(sizeof(open_file));
  if(ret == NULL){
    return NULL;
  }

  ret->file_lk = lock_create("open file");
  if(ret->file_lk == NULL){
    kfree(ret);
    return NULL;
  }
  VOP_INCREF(file);
  ret->vfile = file;
  ret->offset = init_offset;
  return ret;
}

void open_file_destroy(open_file *of)
{
  KASSERT(of != NULL);

  lock_destroy(of->file_lk);
  VOP_DECREF(of->vfile);
  kfree(of);
}


/**********************************************/



file_list* file_list_create(void)
{
  file_list *ret = kmalloc(sizeof(file_list));
  if(ret == NULL){
    return NULL;
  }
  ret->files = linkedlist_create();
  if(ret->files == NULL){
    kfree(ret);
    return NULL;
  }
  ret->available = stack_create();
  if(ret->available == NULL){
    kfree(ret->files);
    return NULL;
  }
  return ret;
}

void file_list_destroy(file_list *fl)
{
  KASSERT(fl != NULL);
  kfree(fl->files);
  kfree(fl->available);
  kfree(fl);
}
/*
 * insert into linked list at (if stack empty, list->first-1, else pop stack)
 * stack available is unsigned integers representing available keys
 */
int file_list_add(file_list *fl, open_file *of)
{
  (void)of;
  (void)fl;
  return 0;
}

/*
 * Search linkedlist and return
 */
open_file *file_list_get(file_list *fl, int fd)
{
  (void)fd;
  (void)fl;
  return NULL;
}
/*
 * add key to available stack, remove from list, and return
 */
open_file *file_list_remove(file_list *fl, int fd)
{
  (void)fd;
  (void)fl;
  return NULL;
}
