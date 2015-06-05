#include <types.h>
#include <linkedlist.h>
#include <lib.h>
#include <test.h>
#include <array.h>
#include <synch.h>
#include <limits.h>

Linked_List *linkedlist_create(void)
{
    Linked_List * ptr = kmalloc(sizeof(Linked_List));
    if(ptr == NULL){
      return NULL;
    }
    ptr -> length = 0;
    ptr -> first = NULL;
    ptr -> last = NULL;
    ptr -> limit = 0;
    ptr -> lk = lock_create("Linked_List lock");
    if(ptr -> lk == NULL){
      kfree(ptr);
      return NULL;
    }

    ptr -> monitor = true;

    return ptr;
}

//does not destroy contents
static void node_destroy(Linked_List_Node *n){
  KASSERT(n != NULL);
  if(n->next != NULL){
    node_destroy(n->next);
  }
  kfree(n);
}


void linkedlist_destroy(Linked_List *list)
{
  KASSERT(list != NULL);
  
  if(list->first != NULL){
    node_destroy(list->first);
  }
  
  lock_destroy(list -> lk);
  kfree(list);
}

Linked_List_Node *linkedlist_create_node(unsigned key, void *data)
{
    Linked_List_Node *newnode = kmalloc(sizeof(Linked_List_Node));
    newnode -> prev = NULL;
    newnode -> next = NULL;
    newnode -> key = key;
    newnode -> data = data;

    return newnode;
}

bool linkedlist_prepend(Linked_List *list, void *data)
{
  if (list != NULL && (!list->limit || list->length < list->limit)) {
    if(list->monitor){
      lock_acquire(list -> lk);
    }    

    Linked_List_Node * newnode;
    Linked_List_Node * f = list -> first;
    
    if (list -> first == NULL) {
      if(!list->limit)
	newnode = linkedlist_create_node(INT_MAX, data);
      else
	newnode = linkedlist_create_node(list->limit, data);
      if(newnode == NULL){
	if(list->monitor){
	  lock_release(list->lk);
	}
	return true;
      }
      list -> first = newnode;
      list -> last = newnode;
    } else {
      newnode = linkedlist_create_node(f -> key - 1, data);
      if(newnode == NULL){
	if(list->monitor){
	  lock_release(list->lk);
	}
	return true;
      }

      newnode -> next = list -> first;
      f -> prev = newnode;
      list -> first = newnode;
    }
    
    list -> length ++;
    
    if(list->monitor){
      lock_release(list -> lk);
    }
    return false;
  }
  return true;
}


void linkedlist_printlist(Linked_List *list, int which)
{
  if (list != NULL){
    if(list->monitor){
      lock_acquire(list -> lk); 
    }
    
    Linked_List_Node *runner = list -> first;
    
    kprintf("%d: ", which);
    kprintf("(len = %d) ", list -> length);
    
    while (runner != NULL) {
      kprintf("%d[%c] ", runner -> key, *((int *)runner -> data));
      runner = runner -> next;
    }
    
    kprintf("\n");
    
    if(list->monitor){
      lock_release(list -> lk);
    }
  }
}



bool linkedlist_insert(Linked_List *list, unsigned key, void *data) {

  if (list != NULL && (!list->limit || list->length < list->limit)) {
    
    if(list->monitor){
      lock_acquire(list -> lk); 
    }    

    Linked_List_Node *node = linkedlist_create_node(key, data);
    if(node == NULL){
      if(list->monitor){
	lock_release(list->lk);
      }
      return true;
    }

    Linked_List_Node *curr = list -> first;
    
    if (curr == NULL) {
      // Test 3 - Yield so both threads think the list is empty when inserting.
      yield_if_should(3);
      list -> first = node;
      list -> last = node;
    } else if (curr -> key >= key) {
      list -> first = node;
      node -> next = curr;
      curr -> prev = node;
    } else {
      while (curr -> next != NULL && curr -> next -> key < key) {
	curr = curr -> next;
      }
      
      if (curr -> next == NULL) {
	list -> last = node;
      } else {
	curr -> next -> prev = node;
      }
      
      node -> next = curr -> next;
      node -> prev = curr;
      // Test 5 - Yield so first thread's node at curr and curr ->
      // next, but neither ends up pointing back at it.  The second
      // thread's node does get inserted correctly. 
      yield_if_should(5);
      curr -> next = node;
			
    }
    
    unsigned length = list -> length;
    length ++;
    // Test 6 - Yield so list -> length is corrupted -- simulates
    // yielding between asm instructions for list -> length ++;
    yield_if_should(6);
    list -> length = length;

    if(list->monitor){
      lock_release(list -> lk);
    }
    return false;
  }
  return true;
}

void * linkedlist_remove_head(Linked_List *list, unsigned *key) {
  void *data = NULL;
  
  if (list != NULL){
    if(list->monitor){
      lock_acquire(list -> lk);
    }    

    if (list -> first != NULL) {
      
      Linked_List_Node * node = list -> first;
      data = node -> data;
      if (key != NULL)
	*key = node -> key;
      
      // Test 4 - Yield so both threads have the same node to remove.
      yield_if_should(4);
      list -> first = node -> next;
      
      if (list -> first == NULL) 
	list -> last = NULL;
      else 
	list -> first -> prev = NULL;
      
      // Test 4 - Thread 2 - Error here on second deallocation of the same node.
      kfree(node);  
      
      list -> length --;
      
    }
    
    if(list->monitor){
      lock_release(list -> lk);
    }
  }
  return data;
}

void * linkedlist_remove(Linked_List *list, unsigned key){

  void * data = NULL;

  if(list != NULL)
  {
    if(list->monitor){
      lock_acquire(list -> lk);
    }    

    if (list -> first != NULL)
    {
      Linked_List_Node * node = list -> first;
      if (key == node->key)
      {
	data = node -> data;
	list -> first = node -> next;
	
	if (list -> first == NULL) 
	  list -> last = NULL;
	else 
	  list -> first -> prev = NULL;
      }
      else
      {
	while(node != NULL && key > node->key)
	{
	  node = node->next;
	}
	if(node != NULL && node->key == key)
	{
	  if (node -> next == NULL)
	  {
	    list -> last = node -> prev;
	    list -> last -> next = NULL;
	  }
	  else 
	  {
	    node -> prev -> next = node -> next;
	    node -> next -> prev = node -> prev;
	  }
	  data = node->data;
	}
	kfree(node);
	list -> length --;
      }
    }
    if(list->monitor){
      lock_release(list -> lk);
    }
  }
  return data;
}


bool linkedlist_append(Linked_List *list, void *data)
{
  if (list != NULL && (!list->limit || list->length < list->limit)) {
    if(list->monitor){
      lock_acquire(list -> lk);
    }    

    Linked_List_Node * newnode;
    Linked_List_Node * f = list -> last;
    
    if (list -> last == NULL) {
      newnode = linkedlist_create_node(0, data);
      if(newnode == NULL){
	if(list->monitor){
	  lock_release(list->lk);
	}
	return true;
      }

      list -> last = newnode;
      list -> first = newnode;
    } 
    else {
      newnode = linkedlist_create_node(f -> key + 1, data);
      if(newnode == NULL){
	return true;
      }
      
      newnode -> prev = list -> last;
      f -> next = newnode;
      list -> last = newnode;
    }
    
    list -> length ++;
    
    if(list->monitor){
      lock_release(list -> lk);
    }
    return false;
  }
  return true;
}
