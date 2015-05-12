#include <types.h>
#include <linkedlist.h>
#include <lib.h>
#include <thread.h>
#include <spl.h>
#include <test.h>
#include <synch.h>

static int test_flags[] = {0, 0, 0, 0, 0, 0, 0};

static void reset_flags(void) {

  bzero(test_flags,7*sizeof(int));

}

// Code to synchronize tests.
struct semaphore * test_sem = NULL;

static void sync_init() {
	
	KASSERT(test_sem == NULL);
	test_sem = sem_create("Linked_List Test Sync Sem",0);
	
}

static void sync_destroy() {
	
	KASSERT(test_sem != NULL);
	sem_destroy(test_sem);
	test_sem = NULL;
	
}

static void sync_wait(int num) {
	
	for(int i = 0; i < num; i++)	{
		P(test_sem);
	}
	
}

static void sync_signal()
{
	V(test_sem);
}





void yield_if_should(int testnum) {

  if (test_flags[testnum]) {
    //    kprintf("Yielding\n");
    thread_yield();
  }

}




static void linkedlist_test_adder(void *list, unsigned long which) {

  int i;
  int *c;

  for (i = 0; i < 4; i++) {
    c = kmalloc(sizeof(int));
    *c = 'A' + i;
    yield_if_should(2);
    linkedlist_prepend(list, c);
    //   linkedlist_printlist(list, which);
  }

  which = 1;

	sync_signal(); // signal test done
}

static void linkedlist_test_insert(void * list, unsigned long which){

  int *data = kmalloc(sizeof(int));
  *data = 'A' + which;
  linkedlist_insert(list,which,data);

	sync_signal();
}

static void linkedlist_test_remove(void * list, unsigned long which){

  int key;
  int *data = linkedlist_remove_head(list,&key);

  if (which == 2) {
    kprintf("Expected\n2\nActual%d\n",*data);
  }

	sync_signal();
}


/* A sequential test of added list functions */
static int sequential_test(void) {

  const int NUM_ELTS = 10;
  Linked_List * list = linkedlist_create();
  int failed = 0;
  
  int key = -1;
  int * data = linkedlist_remove_head(list, &key);
  
  if (data != NULL) {
    kprintf("TEST FAILURE: expected NULL data\n");
    failed = -1;
  }
  
  data = linkedlist_remove_head(list, NULL);
  
  if (data != NULL) {
    kprintf("TEST FAILURE: expected NULL data\n");
    failed = -1;
  }
  
  data = linkedlist_remove_head(NULL, &key);
  
  if (data != NULL) {
    kprintf("TEST FAILURE: expected NULL data\n");
    failed = -1;
  }
  
  for (int i = 0; i < NUM_ELTS; i++) {
    data = (int *)kmalloc(sizeof(int));
    *data = 'A'+i;
    key = i * (1 - 2 * (i % 2));
    linkedlist_insert(list, key, data);
  }      
  
  linkedlist_printlist(list, 0);
  
  for (int i = 0; i < NUM_ELTS; i++){
    data = linkedlist_remove_head(list, &key);
    int target_key = -9 + i * 2 - (i >= 5 ? 1 : 0);
    int target_data = 'A' + (target_key >= 0 ? target_key : -target_key);
    if (key != target_key || *data != target_data) {
      kprintf("TEST FAILURE: %d[%c] != %d[%c]\n",
	      target_key,target_data,key,*data);
      failed = -1;
    }
    kfree(data);
  }     
  
  if (!failed)
    kprintf("TEST 0 SUCCESSFUL!\n");
  
  kfree(list);
  
  return failed;
  
}



int linkedlist_test_runtests(int nargs, char **args)
{
	int testnum = 0;

	sync_init();
  
	if (nargs == 2) {
		testnum = args[1][0] - '0'; // XXX - Hack - only works for testnum 0 -- 9
	}
	
	kprintf("LLT Test %d\n", testnum);
	
	reset_flags();
	
	if (testnum == 0) {
		// Sequential test on linkedlist_insert

		sequential_test();
		
	} else if (testnum == 1) {
		// Original thread test creates two threads that add to the same
		// list and print.
		
		Linked_List * list = linkedlist_create();
		
		thread_fork("Test 1 Thread 1",
								NULL,
								linkedlist_test_adder,
								list,
								1);
		
		thread_yield();  
		
		thread_fork("Test 1 Thread 2",
								NULL,
								linkedlist_test_adder,
								list, 
								2);
		
		sync_wait(2);
	 
		kprintf("Expected\n1: (len = 8) -7[D] -6[C] -5[B] -4[A] -3[D] -2[C] -1[B] 0[A]\nActual\n");
		linkedlist_printlist(list,1);
		
	} else if (testnum == 2) {
		// Test 1 
		
		// Version of test 1 which interleaves threads between inserts.
		// Doesn't break anything.
		
    
		test_flags[2] = 1;
		
		Linked_List * list = linkedlist_create();
    
		thread_fork("Test 2 Thread 1",
								NULL,
								linkedlist_test_adder,
								list,
								1);
		
		// Let new thread progress and before adding.
		thread_yield();  
		
		thread_fork("Test 2 Thread 2",
								NULL,
								linkedlist_test_adder,
								list, 
								2);
		
		sync_wait(2);		

		kprintf("Expected\n2: (len = 8) -7[D] -6[D] -5[C] -4[C] -3[B] -2[B] -1[A] 0[A] \n(Note ordering depends on scheduling and may vary.)\nActual\n");
		linkedlist_printlist(list,2);
		
	} else if (testnum == 3) {
		// Test 3
		// Two threads try to simulatenously add to the beginning of an
		// empty list.  Interleaves after check for list being empty
		// causing first insert to have no effect.  Causes memory leak.
		
		test_flags[3] = 1;
    
		Linked_List * list = linkedlist_create();
    
		thread_fork("Test 3 Thread 1",
								NULL,
								linkedlist_test_insert,
								list,
								0);
		
		// Let new thread proceed to after empty check.
		thread_yield();  
    
		// Let thread 2 proceed to after empty check, and then swap
		// back to the thread 1.  
		thread_fork("Test 3 Thread 2",
							 NULL,
							 linkedlist_test_insert,
							 list, 
							 1); 
		
		sync_wait(2);				

		kprintf("Expected\n3: (len = 2) 0[A] 1[B] \nActual\n");
		linkedlist_printlist(list,3);
		
	} else if (testnum == 4) {
		// Test 4
		// Two threads try to simulatenously remove the head of a list of length 2.
		// Only the first item is removed twice.  The list structure is ok, but 
		// it break in kfree when trying to deallocate a location in memory.
		
		test_flags[4] = 1;
		
		// Make a list of two items.
		Linked_List * list = linkedlist_create();
    
		int d1 = 'A';
		int d2 = 'B';
		
		linkedlist_insert(list, 1, &d1);
		linkedlist_insert(list, 2, &d2);
    
		linkedlist_printlist(list,4);
		
		thread_fork("Test 4 Thread 1",
								NULL,
								linkedlist_test_remove,
								list,
								1);
		
		// Switch to new thread.
		thread_yield();  
		
		thread_fork("Test 4 Thread 2",
								NULL,
								linkedlist_test_remove,
								list, 
								2); 
		
		sync_wait(2);		

		kprintf("Expected\n4: (len = 0) \nActual\n");
		linkedlist_printlist(list,4);

		// XXX - never gets here because of error caused in kfree.
		
	} else if (testnum == 5) {
		// Test 5 
		
		// Simulatenously insert leaves the new thread's node points at
		// the list, but is not actually connected in the list because
		// the main thread overwrites it.  Causes memory leak.
		
		test_flags[5] = 1;
		
		// Make a list of two items.
		Linked_List * list = linkedlist_create();
    
		int d1 = 'A';
		int d2 = 'D';
		
		linkedlist_insert(list, 0, &d1);
		linkedlist_insert(list, 3, &d2);
    
		linkedlist_printlist(list,5);
		
		thread_fork("Test 5 Thread 1",
								NULL,
								linkedlist_test_insert,
								list,
								1);
		
		// Switch to new thread.
		thread_yield();  
		
		thread_fork("Test 5 Thread 2",
								NULL,
								linkedlist_test_insert,
								list,
								2);
		
		sync_wait(2);		

		kprintf("Expected\n5: (len = 4) 0[A] 1[B] 2[C] 3[D]\nActual\n");
		linkedlist_printlist(list,5);
		
	} else if (testnum == 6) {
		// Test 6 
		
		// Simultaneous insert corrupts list -> length, very similar to
		// example in class.
		
		test_flags[6] = 1;
		
		// Make a list of two items.
		Linked_List * list = linkedlist_create();
    
		int d1 = 'A';
		int d2 = 'D';
		
		linkedlist_insert(list, 0, &d1);
		linkedlist_insert(list, 3, &d2);
    
		linkedlist_printlist(list,5);
		
		thread_fork("Test 6 Thread 1",
								NULL,
								linkedlist_test_insert,
								list,
								1);
		
		// Switch to new thread.
		thread_yield();  
 	
		// Add with main thread.
		thread_fork("Test 6 Thread 2",
								NULL,
								linkedlist_test_insert,
								list,
								2);
		
		sync_wait(2);		

		kprintf("Expected\n6: (len = 4) 0[A] 1[B] 2[C] 3[D]\nActual\n");
		linkedlist_printlist(list,6);
		
	}

	sync_destroy();
	
	return 0;  
}
