#ifndef _SHARED_BUFFER_H_
#define _SHARED_BUFFER_H_

#include <types.h>
#include <synch.h>

typedef struct Shared_Buffer Shared_Buffer;

struct Shared_Buffer {

	char * buf;
	int size;
	int count;
	int p;
	int c;
	struct lock * lk;
	struct cv * cv_con;
	struct cv * cv_pro;
	
};

/* Creates a shared buffer with a given size > 0 */
Shared_Buffer * sb_create(int size);

/* Inserts a character val into the next avaliable cell in the buffer.
	 If the buffer is full this function blocks until a character is
	 available. */
void sb_produce(Shared_Buffer * sb, char val);

/* Removes and returns a character from the buffer if it contains one,
	 otherwise it blocks until space is available. */
char sb_consume(Shared_Buffer * sb);

/* Destroys a shared buffer. */
void sb_destroy(Shared_Buffer * sb);

void sb_print(Shared_Buffer * sb);


#endif
