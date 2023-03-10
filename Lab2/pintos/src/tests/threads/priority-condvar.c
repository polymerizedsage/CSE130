/* Tests that cond_signal() wakes up the highest-priority thread
   waiting in cond_wait(). */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/condvar.h"
#include "threads/lock.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func priority_condvar_thread;
static struct lock lock;
static struct condvar condvar;

void
test_priority_condvar(void)
{
    int i;

    /* This test does not work with the MLFQS. */
    ASSERT(!thread_mlfqs);

    lock_init(&lock);
    condvar_init(&condvar);

    thread_set_priority(PRI_MIN);
    for (i = 0; i < 10; i++) {
        int priority = PRI_DEFAULT - (i + 7) % 10 - 1;
        char name[16];
        snprintf(name, sizeof name, "priority %d", priority);
        thread_create(name, priority, priority_condvar_thread, NULL);
    }

    for (i = 0; i < 10; i++) {
        /*

        struct list_elem* e;
  
        printf("waiters: ");
        for (e = list_begin (&condvar.waiters); e != list_end (&condvar.waiters); e = list_next (e)){
            struct semaphore* f = list_entry (e, struct semaphore, condvarelem);
            
            struct list_elem* waiter = list_front( &f->waiters);
            struct thread* g = list_entry(waiter, struct thread, sharedelem);
            printf("%s ", g->name);
        }
        printf("\n");
        */

        lock_acquire(&lock);
        msg("Signaling...");
        condvar_signal(&condvar, &lock);
        lock_release(&lock);
    }
}

static void
priority_condvar_thread(void *aux UNUSED)
{
    msg("Thread %s starting.", thread_name());
    lock_acquire(&lock);
    condvar_wait(&condvar, &lock);
    msg("Thread %s woke up.", thread_name());
    lock_release(&lock);
}
