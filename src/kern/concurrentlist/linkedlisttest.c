#include <linkedlist.h>
#include <lib.h>
#include <thread.h>
#include <spl.h>
#include <test.h>

static void linkedlist_test_adder(void *list, unsigned long which)
{
    splhigh();

    int i;
    int *c;

    for (i = 0; i < 10; i++) {
	c = kmalloc(sizeof(int));
	*c = 'A' + i;
	linkedlist_prepend(list, c);
	linkedlist_printlist(list, which);
    }
}

int linkedlist_test_runtests(int nargs, char **args)
{
    int testnum = 0;
    
    if (nargs == 2) {
	testnum = args[1][0] - '0'; // XXX - Hack - only works for testnum 0 -- 9
    }

    kprintf("testnum: %d\n", testnum);

    Linked_List * list = linkedlist_create();
  
    thread_fork("adder 1",
		linkedlist_test_adder,
		list,
		1,
		NULL);

    thread_fork("adder 2",
		linkedlist_test_adder,
		list,
		2,
		NULL);

    // XXX - Bug - We're returning from this function without waiting
    // for these two threads to finish.  The execution of these
    // threads may interleave with the kernel's main menu thread and
    // cause interleaving of console output.  We going to accept this
    // problem for the moment until we learn how to fix in Project 2.
    // An enterprising student might investigate why this is not a
    // problem with other tests suites the kernel uses.

    return 0;  
}
