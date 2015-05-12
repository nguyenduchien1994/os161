#include <types.h>
#include <synch.h>
#include <shared_buffer.h>
#include <lib.h>

/* Creates a shared buffer with a given size > 0 */
Shared_Buffer * sb_create(int size){

  KASSERT(size > 0);
  
  Shared_Buffer * sb = kmalloc(sizeof(Shared_Buffer));
  if (sb == NULL)
    return NULL;
  
  sb -> buf = kmalloc(size * sizeof(char));
  if (sb -> buf == NULL){
    kfree(sb);
    return NULL;
  }
  
  sb -> lk = lock_create("Shared_Buffer lock");
  if (sb -> lk == NULL) {
    kfree(sb -> buf);
    kfree(sb);
    return NULL;
  }
  
  sb -> cv_pro = cv_create("Shared_Buffer producer cv");
  if (sb -> cv_pro == NULL) {
    kfree(sb -> lk);
    kfree(sb -> buf);
    kfree(sb);
    return NULL;
  }
  
  sb -> cv_con = cv_create("Shared_Buffer consumer cv");
  if (sb -> cv_con == NULL) {
    kfree(sb -> cv_con);
    kfree(sb -> lk);
    kfree(sb -> buf);
    kfree(sb);
    return NULL;
  }	
  
  // Being robust makes for clunky code sometimes.  :(
  
  sb -> size = size;
  sb -> count = 0;
  sb -> p = 0;
  sb -> c = 0;
  
  return sb;
  
}


/* Inserts a character val into the next avaliable cell in the buffer.
   If the buffer is full this function blocks until a character is
   available. */
void sb_produce(Shared_Buffer * sb, char val) {

  KASSERT(sb != NULL);
  
  lock_acquire(sb->lk);
  
  // Go to sleep if the buffer is full.
  if (sb -> count == sb -> size) {
    cv_wait(sb->cv_pro, sb->lk);
  }
  
  (sb -> buf)[sb -> p] = val;
  sb -> p = (sb -> p + 1) % sb -> size;	
  
  sb -> count++;
  
  // Wake a consumer that may be sleeping.
  cv_signal(sb->cv_con, sb->lk);
  
  lock_release(sb->lk);	
  
}


/* Removes and returns a character from the buffer if it contains one,
   otherwise it blocks until space is available. */
char sb_consume(Shared_Buffer * sb) {
  
  char ret = 'X';
  
  KASSERT(sb != NULL);
  
  lock_acquire(sb->lk);
  
  // Go to sleep if the buffer is empty.
  if (sb -> count == 0) {
    cv_wait(sb->cv_con, sb->lk);
  }
  
  ret = (sb -> buf)[sb -> c];
  sb -> c = (sb -> c + 1) % sb -> size;	
  
  sb -> count--;
  
  // Wake a producer that may be sleeping.
  cv_signal(sb->cv_pro, sb->lk);
  
  lock_release(sb->lk);	

  return ret;
}

/* Destroys a shared buffer. */
void sb_destroy(Shared_Buffer * sb) {
  
  KASSERT(sb != NULL);		
  cv_destroy(sb -> cv_pro);
  cv_destroy(sb -> cv_con);
  lock_destroy(sb -> lk);
  kfree(sb -> buf);
  kfree(sb);
  
}

void sb_print(Shared_Buffer * sb)
{
  KASSERT(sb != NULL);
  
  lock_acquire(sb -> lk);
  
  kprintf("Size = %d, Count = %d, c = %d, p = %d, ",
	  sb -> size, sb -> count, sb -> c, sb -> p);
  
  int p = sb -> p;
  int c = sb -> c;
  
  kprintf("\tbuf: ");

  for (int i = 0; i < sb -> size; i++) {
    if ((c == p && sb -> count == sb->size) 
	|| (c <= p && c <= i && i < p) 
	|| (c > p && (i >= c || i < p))) {
      kprintf("%c",(sb->buf)[i]);
    } else
      kprintf("-");
  }
  kprintf("\n");
  
  lock_release(sb -> lk);
  
}

