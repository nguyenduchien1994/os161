#include <types.h>
#include <shared_buffer.h>
#include <lib.h>
#include <thread.h>
#include <spl.h>
#include <test.h>
#include <synch.h>

// Code to synchronize tests.
static struct semaphore * test_sem = NULL;

static bool verbose = false;

static void sync_init() {
  
  KASSERT(test_sem == NULL);
  test_sem = sem_create("Shared_Buffer Test Sync Sem",0);
  
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

static void test_consumer(void * data, unsigned long num){
  
  Shared_Buffer * sb = (Shared_Buffer *)data;
  for (unsigned int i = 0; i < num; i++) {
    char ret = sb_consume(sb);
    if (verbose) 
      sb_print(sb);
    else 
      kprintf("Consumed: %c\n",ret);
  }
  
  sync_signal();
}

static void test_producer(void * data, unsigned long num){
  
  Shared_Buffer * sb = (Shared_Buffer *)data;
  for (unsigned int i = 0; i < num; i++) {
    char val = ('A' + i) % 26 + 'A';
    sb_produce(sb,val);
    if (verbose) 
      sb_print(sb);
    else
      kprintf("Produced: %c\n",val);
  }
  
  sync_signal();	
}

int shared_buffer_run_tests(int nargs, char **args)
{
  int testnum = 0;
  int NTHREADS = 10;
  
  sync_init();
  
  if (nargs == 2) {
    testnum = args[1][0] - '0'; // XXX - Hack - only works for testnum 0 -- 9
  }
  
  kprintf("Shared Buffer Test %d\n", testnum);
  
  if (testnum == 0 || testnum == 1) {
    // One Producer - One Consumer 

    verbose = (testnum == 1);

    Shared_Buffer * sb = sb_create(10);
    
    sb_print(sb);
    
    thread_fork("SBT 0-1 Producer",
		NULL,
		test_producer,
		sb,
		20);
    
    thread_fork("SBT 0-1 Consumer",
		NULL,
		test_consumer,
		sb,
		20);
    
    sync_wait(2);
    
    sb_print(sb);
    
  }
  
  if (testnum == 2 || testnum == 3) {
    // Many Producers - One Consumer 
    
    verbose = (testnum == 3);
    
    Shared_Buffer * sb = sb_create(10);
    
    sb_print(sb);
    
    for (int i = 0; i < NTHREADS; i++){
      thread_fork("SBT 2-3 Producer",
		  NULL,
		  test_producer,
		  sb,
		  10);
    }      

    thread_fork("SBT 2-3 Consumer",
		NULL,
		test_consumer,
		sb,
		10*NTHREADS);
    
    sync_wait(NTHREADS+1);
    
    sb_print(sb);
    
  }

  if (testnum == 4 || testnum == 5) {
    // One Producer - Many Consumers 
    
    verbose = (testnum == 5);
    
    Shared_Buffer * sb = sb_create(10);
    
    sb_print(sb);
    

    thread_fork("SBT 4-5 Producer",
		NULL,
		test_producer,
		sb,
		10*NTHREADS);
    
    
    for (int i = 0; i < NTHREADS; i++){
      thread_fork("SBT 4-5 Consumer",
		  NULL,
		  test_consumer,
		  sb,
		  NTHREADS);
    }    
    
    sync_wait(NTHREADS+1);
    
    sb_print(sb);
    
  }

  if (testnum == 6 || testnum == 7) {
    // Many Producers - Many Consumers 
    
    verbose = (testnum == 7);
    
    Shared_Buffer * sb = sb_create(10);
    
    sb_print(sb);
    

    for (int i = 0; i < NTHREADS; i++){
      thread_fork("SBT 6-7 Producer",
		  NULL,
		  test_producer,
		  sb,
		  10);
      thread_fork("SBT 6-7 Consumer",
		  NULL,
		  test_consumer,
		  sb,
		  10);
    }    
    
    sync_wait(2*NTHREADS);
    
    sb_print(sb);
    
  }

    



  sync_destroy();
  
  return 0;  
}
