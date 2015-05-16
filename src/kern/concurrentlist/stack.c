#include <types.h>
#include <linkedlist.h>
#include <synch.h>
#include <lib.h>

stack * stack_create(void)
{
  stack * s = linkedlist_create();
  return s;
}

void stack_destroy(stack *s)
{
  KASSERT(s != NULL);
  linkedlist_destroy(s);
}

void stack_push(stack *s, void *data)
{
  linkedlist_prepend(s,data);
}

void* stack_pop(stack *s)
{
  int * key = &s->first->key;
  return linkedlist_remove_head(s,key);
}
