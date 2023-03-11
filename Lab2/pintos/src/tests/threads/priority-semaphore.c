/* Tests that the highest-priority thread waiting on a semaphore
   is the first to wake up. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/semaphore.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func priority_sema_thread;
static struct semaphore sema;

void
test_priority_semaphore (void) 
{
  int i;
  
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  semaphore_init (&sema, 0);
  thread_set_priority (PRI_MIN);
  for (i = 0; i < 10; i++) 
    {
      int priority = PRI_DEFAULT - (i + 3) % 10 - 1;
      char name[16];
      snprintf (name, sizeof name, "priority %d", priority);
      thread_create (name, priority, priority_sema_thread, NULL);
    }
  
  for (i = 0; i < 10; i++) 
    {
      /*
      struct list_elem* e;
  
      printf("waiters: ");
      for (e = list_begin (&sema.waiters); e != list_end (&sema.waiters); e = list_next (e)){
        struct thread* f = list_entry (e, struct thread, sharedelem);
        printf("%s ", f->name);
      }
      printf("\n");
      */

      semaphore_up (&sema);
      msg ("Back in main thread."); 
    }
}

static void
priority_sema_thread (void *aux UNUSED) 
{
  
  semaphore_down (&sema);
  //printf("init thread %s\n", thread_current()->name);
  msg ("Thread %s woke up.", thread_name ());
}
