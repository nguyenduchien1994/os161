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

bool queue_add(queue *q, void *data)
{
  return linkedlist_append(q, data);
}

void* queue_remove(queue *q)
{
  unsigned key;
  return linkedlist_remove_head(q, &key);
}
