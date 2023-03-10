/* Checks that when the alarm clock wakes up threads, the
   higher-priority threads run first. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/semaphore.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func alarm_priority_thread;
static int64_t wake_time;
static struct semaphore wait_sema;

void
test_alarm_priority(void)
{

    int i;

    wake_time = timer_ticks() + 5 * TIMER_FREQ;
    semaphore_init(&wait_sema, 0);

    for (i = 0; i < 10; i++) {
        int priority = PRI_DEFAULT - (i + 5) % 10 - 1;
        char name[16];
        snprintf(name, sizeof name, "priority %d", priority);
        
        thread_create(name, priority, alarm_priority_thread, NULL);
    }

    thread_set_priority(PRI_MIN);

    for (i = 0; i < 10; i++){
        semaphore_down(&wait_sema);
    }

        
}

static void
alarm_priority_thread(void *aux UNUSED)
{
    //printf("thread %s starting\n", thread_current()->name);
    /* Busy-wait until the current time changes. */
    int64_t start_time = timer_ticks();
    while (timer_elapsed(start_time) == 0)
        continue;

    /* Now we know we're at the very beginning of a timer tick, so
       we can call timer_sleep() without worrying about races
       between checking the time and a timer interrupt. */
    timer_sleep(wake_time - timer_ticks());

    /* Print a message on wake-up. */
    msg("Thread %s woke up.", thread_name());

    /*
      struct list_elem* e;
  
      printf("waiters: ");
      for (e = list_begin (&wait_sema.waiters); e != list_end (&wait_sema.waiters); e = list_next (e)){
        struct thread* f = list_entry (e, struct thread, sharedelem);
        printf("%s ", f->name);
      }
      printf("\n");
      
    */
    semaphore_up(&wait_sema);
}
