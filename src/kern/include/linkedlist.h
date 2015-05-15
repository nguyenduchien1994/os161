#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <types.h>
#include <synch.h>

typedef struct Linked_List_Node Linked_List_Node;

struct Linked_List_Node {
    Linked_List_Node *prev;
    Linked_List_Node *next;
    int key;
    void *data;
};

typedef struct Linked_List Linked_List;

struct Linked_List {
  Linked_List_Node *first;
  Linked_List_Node *last;
  int length;
  struct lock * lk;
};

Linked_List *linkedlist_create(void);
void linkedlist_destroy(Linked_List *list);

Linked_List_Node *linkedlist_create_node(int key, void *data);

/*
 * Inserts the item at the front of the list.  If the item is not the
 * only item in the list, set its key to min - 1, where min is the
 * previous minimum key in the list.  If this is the first item, set
 * its key to 0.
 */
void 
linkedlist_prepend(Linked_List *list, void *data);

/*
 * Prints the list.  Adds the int 'which' to the front of the output,
 * to aid in debugging.
 */
void linkedlist_printlist(Linked_List *list, int which);

/*******************************
 * These are to be implemented *
 *******************************/

/*
 * Inserts into the linked-list so that the items are in order by
 * their keys.
 */
void 
    linkedlist_insert(Linked_List *list, int key, void *data);

/*
 * Removes the head node from the list.  key is set to the key of the
 * removed node.  The removed item is returned.
 *
 */
void *
linkedlist_remove_head(Linked_List *list, int *key);

typedef Linked_List stack;

stack* stack_create(void);
void stack_destroy(stack *s);
void stack_push(stack *s, void *data);
void* stack_pop(stack *s);//NULL if empty

typedef Linked_List queue;

queue* queue_create(void);
void queue_destroy(queue *q);
void queue_add(queue *q, void *data);
void* queue_remove(queue *q);//NULL if empty

typedef struct multi_queue{
  queue **queues;
  int *turns;//same size as queue array, should all be positive and distinct
  int which;//which queue to pull from
  int count;//how many times we have pulled from current queue
  int num_queues;
} multi_queue;


/*
 * multi_queue_add
 *      Adds the data to the queue in the multiqueue specified by which
 * 
 * multi_queue_remove
 *      Remove data from the multi_queue. Pulled from queues by frequency. For each queue, if priority % counter == 0, pull from queue, increment counters (if non-zero or return counter) and return. Else increment counters and try again.
 */

multi_queue* multi_queue_create(int * turns, int size);
void multi_queue_destroy(multi_queue *mq);
void multi_queue_add(multi_queue *mq, void *data, int which);
void* multi_queue_remove(multi_queue *mq);//NULL if empty

#endif
