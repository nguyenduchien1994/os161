
/*
 * Tools for handling open files on a user process
 */

#ifndef _USR_FILE_H_
#define _USR_FILE_H_


#include <types.h>
#include <linkedlist.h>


/*
 * Represents a file opened by process
 * File descriptor represented by index(key) in file_list
 */
typedef 
  struct open_file{
              struct vnode *vfile;
              struct lock *file_lk;
              int flag;
              volatile off_t offset;
              volatile unsigned refcount;
  } open_file;

open_file* open_file_create(struct vnode *file, off_t init_offset);
void open_file_destroy(open_file *of);
void open_file_incref(open_file *of);
void open_file_decref(open_file *of);

typedef 
  struct file_list{
              Linked_List *files;
              stack *available;
  } file_list;

file_list* file_list_create(void);
void file_list_destroy(file_list *fl);//assumes empty
int file_list_add(file_list *fl, open_file *of);//return handle
int file_list_insert(file_list *fl, open_file *of, int fd); 
open_file *file_list_get(file_list *fl, int fd);
open_file *file_list_remove(file_list *fl, int fd);




#endif
