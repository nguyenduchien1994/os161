#include <types.h>
#include <linkedlist.h>



multi_queue* multi_queue_create(queue** queues, int * priorities, int size)
{
  (void)queues;
  (void)priorities;
  (void)size;

  return NULL;
}

void multi_queue_destroy(multi_queue *mq)
{
  (void)mq;
}

void multi_queue_add(multi_queue *mq, void *data, int which)
{
  (void)mq;
  (void)data;
  (void)which;
}

void* multi_queue_remove(multi_queue *mq)
{
  (void)mq;
  return NULL;
}
