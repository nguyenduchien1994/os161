#include <types.h>
#include <lib.h>
#include <usr_file.h>
#include <linkedlist.h>
#include <vnode.h>
#include <synch.h>
#include <limits.h>

open_file* open_file_create(struct vnode *file, off_t init_offset, int flags)
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
  ret->refcount = 0;
  ret->flags = flags;
  return ret;
}

void open_file_destroy(open_file *of)
{
  KASSERT(of != NULL);

  lock_destroy(of->file_lk);
  VOP_DECREF(of->vfile);
  kfree(of);
}

void open_file_incref(open_file *of){
  KASSERT(of != NULL);
  of->refcount++;
}

void open_file_decref(open_file *of){
  KASSERT(of != NULL);
  of->refcount--;
  
  if(of->refcount <= 0){
    open_file_destroy(of);
  }
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
  ret->files->limit = OPEN_MAX;
  ret->available = stack_create();
  if(ret->available == NULL){
    kfree(ret->files);
    return NULL;
  }
  ret->available->limit = OPEN_MAX;
  return ret;
}

void file_list_destroy(file_list *fl)
{
  KASSERT(fl != NULL);
  linkedlist_destroy(fl->files);
  linkedlist_destroy(fl->available);
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
    int *id = stack_pop(fl -> available);
    ret = *id;
    kfree(id);
    linkedlist_insert(fl -> files, ret, of);
  }
  open_file_incref(of);
  return ret;
}

static Linked_List_Node *file_list_get_node(file_list *fl, int fd)
{
  KASSERT(fl != NULL);

  if (fd < 0 || fd > OPEN_MAX)
  {
    return NULL;
  }
  else
  {
    Linked_List_Node *node = fl -> files -> first;
    while(node != NULL && fd > node -> key)
    {
      node = node -> next;
    }
    if (node != NULL && node -> key != fd)
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
 * 0,1,2,6
 * Insert into linked list using given filehandle (overwriting if need be).
 * Get copy of pointer to open file then:
 * Case 1: fd > list->last->key (max fd). file_list_add a copy of open_file pointer, change list->last->key to fd, add x to stack for oldkey < x < fd
 * Case 2: fd < list->last->key and fd not used. Search through stack and remove, insert to list as desired
 * Case 3: '' and fd used. Close file at fd (so now fd on top of stack) and then file_list_add the copy of the pointer
 */
int file_list_insert(file_list *fl, open_file *of, int fd)
{
  KASSERT(fl != NULL);

  if (fd < 0 || fd > OPEN_MAX)
  {
    return -1;
  } 
  else
  {
    open_file_incref(of);
    int curfd = fl -> files -> last -> key;
    if (fd > curfd)
    {
      linkedlist_insert(fl -> files, fd, of);
      for (int i=curfd; i<fd; i++)
      {
	int *to_push = kmalloc(sizeof(int));
	*to_push = i;
	stack_push(fl->available, to_push);
      }
    }
    else
    {
      Linked_List_Node *node = file_list_get_node(fl,fd);
      if (node == NULL)
      {
	linkedlist_insert(fl -> files, fd, of);
	Linked_List_Node *runner = fl->available->first;
	while(runner != NULL){
	  if(*(int*)runner->data == fd){
	    if(runner->prev){
	      runner->prev->next = runner->next;
	    }
	    if(runner->next){
	      runner->next->prev = runner->prev;
	    }
	    fl->available->length--;
	    kfree(runner->data);
	  }
	  else{
	    runner = runner->next;
	  }
	}
      }
      else
      {
	open_file_decref((open_file*)node -> data);
	node -> data = of;
      }
    }
    return 0;
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
  if (fd < 0 || fd > OPEN_MAX)
  {
    return NULL;
  }
  else
  {      
    open_file *ret = linkedlist_remove(fl -> files, fd);
    
    int *to_push = kmalloc(sizeof(int));
    *to_push = fd;
    stack_push(fl -> available, to_push);
    return ret;
  }
}
