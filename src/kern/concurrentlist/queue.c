#include <types.h>
#include <linkedlist.h>
#include <lib.h>

queue* queue_create(void)
{
  queue * q = linkedlist_create();  
  return q;
}

void queue_destroy(queue *q)
{
  linkedlist_destroy(q);
}

void queue_add(queue *q, void *data)
{
  linkedlist_prepend(q, data);
}

void* queue_remove(queue *q)
{
  void * data = NULL;

  if (q != NULL)       {

    lock_acquire(q -> lk);

    if (q -> last != NULL) {

      Linked_List_Node * node = q -> last;
      data = node -> data;
      // if (key != NULL)
      //*key = node -> key;

      q -> last = node -> next;

      if (q -> last == NULL)
       q -> first = NULL;
      else
	q -> last -> next = NULL;
                                                                                
      kfree(node);

      q -> length --;

    }

    lock_release(q -> lk);
  }

  return data;
}
