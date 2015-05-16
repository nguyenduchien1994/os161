#include <types.h>
#include <lib.h>
#include <usr_file.h>
#include <linkedlist.h>
#include <vnode.h>
#include <synch.h>
#include <limits.h>

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
  linkedlist_append(ret->files,NULL);
  linkedlist_append(ret->files,NULL);
  linkedlist_append(ret->files,NULL);

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
 * insert into linked list at (if stack empty, last, else pop stack)
 * stack available is unsigned integers representing available keys
 * The return value is the most recent file handle created
 */
int file_list_add(file_list *fl, open_file *of)
{
  KASSERT(fl != NULL);

  int ret;
  if (fl -> available -> first == NULL)
  {
    linkedlist_append(fl -> files, of);
    ret = fl -> files -> last -> key;
  }
  else
  {
    ret =  *( (int*) stack_pop(fl -> available) );
    linkedlist_insert(fl -> files, ret, of);
  }
  
  return ret;
}

int file_list_insert(file_list *fl, open_file *of, int fd)
{
  KASSERT(fl != NULL);
  (void)of;

  if (fd < 0 || fd > INT_MAX)
  {
    return -1;
  } 
  else
  {
    return 0;
  }
}

static Linked_List_Node *file_list_get_node(file_list *fl, int fd)
{
  KASSERT(fl != NULL);

  if (fd < 0 || fd > INT_MAX)
  {
    return NULL;
  }
  else
  {
    Linked_List_Node *node = fl -> files -> first;
    while(node != NULL && fd < node -> key)
    {
      node = node -> next;
    }
    if (node -> key != fd)
    {
      return NULL;
    }
    else
    {
      return node;
    }
  }
}

/*
 * Search linkedlist and return
 */
open_file *file_list_get(file_list *fl, int fd)
{
  Linked_List_Node *node = file_list_get_node(fl,fd);
  if (node == NULL)
  {
    return NULL;
  }
  else
  {
    return node -> data;
  }
}

/*
 * add key to available stack, remove from list, and return
 */
open_file *file_list_remove(file_list *fl, int fd)
{
  Linked_List_Node *in_list = file_list_get_node(fl,fd);

  if (in_list == NULL)
  {
    //don't change stack
    return NULL;
  }
  else
  {
    //change stack
    open_file *ret = linkedlist_remove(fl -> files, fd);
    
    int *to_push = kmalloc(sizeof(int));
    *to_push = fd;
    stack_push(fl -> available, to_push);
    return ret;
  }
}
