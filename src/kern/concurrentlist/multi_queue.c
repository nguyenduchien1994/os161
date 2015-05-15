#include <types.h>
#include <linkedlist.h>
#include <lib.h>


multi_queue* multi_queue_create(int * turns, int size)
{
  multi_queue *ret = kmalloc(sizeof(multi_queue));
  
  ret->num_queues = size;
  ret->which = 0;
  ret->count = 0;

  ret->queues = kmalloc(sizeof(queue*)*size);
  if(ret->queues == NULL){
    kfree(ret);
    return NULL;
  }
  ret->turns = kmalloc(sizeof(int)*size);
  if(ret->turns == NULL){
    kfree(ret->queues);
    kfree(ret);
    return NULL;
  }

  for(int i = 0; i < size; i++){
    *(ret->queues + i*sizeof(queue*)) = queue_create();
    if(ret->queues + i*sizeof(queue*) == NULL){
      for(int j = 0; j < i; j++){
	kfree(ret->queues + j*sizeof(queue*));
      }
      kfree(ret->turns);
      kfree(ret->queues);
      kfree(ret);
      return NULL;
    }
    *(ret->turns + i*sizeof(int)) = *(turns + i*sizeof(int));
  }

  return ret;
}

void multi_queue_destroy(multi_queue *mq)
{
  KASSERT(mq != NULL);

  for(int i = 0; i < mq->num_queues; i++){
    queue_destroy(*(mq->queues + i*sizeof(queue*)));
  }
  kfree(mq->turns);
  kfree(mq->queues);
  kfree(mq);
}

void multi_queue_add(multi_queue *mq, void *data, int which)
{

  KASSERT(mq != 0);
  KASSERT(which >= 0);
  
  queue *add_to = *(mq->queues + which*sizeof(queue*));
  queue_add(add_to, data);
}

void* multi_queue_remove(multi_queue *mq)
{
  void *ret = NULL;
  int start = mq->which;
  bool done = false;
  queue *next;
  int turns;
  
  while(ret == NULL && !done){
    
    next = *(mq->queues + mq->which*sizeof(queue*));
    turns = *(mq->turns + mq->which*sizeof(int));
    
    //if out of turns or nothing to dequeue, move to next
    if(mq->count == turns || next->first == NULL){
      //look at next queue
      mq->which = (mq->which + 1) % mq->num_queues;
      mq->count = 0;
    }
    else{
      //return the next item in the queue
      ret = queue_remove(next);
      mq->count++;
    }
    
    //if we looked at all queues
    if(mq->which == start){
      done = true;
    }
  }
  return ret;
}
