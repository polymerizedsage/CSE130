#ifndef LOCK_H
#define LOCK_H

#include "threads/semaphore.h"

/* Lock */
struct lock {
    struct thread *holder; /* Thread holding lock (for debugging) */
    //struct thread *donated; //thread that got priority donated to it
    struct thread *donator; //the thread giving its priority to the holding thread
    struct semaphore semaphore; /* Binary semaphore controlling access */
    struct list_elem lockelem;
};

void lock_init(struct lock *);
void lock_acquire(struct lock *);
void lock_release(struct lock *);
bool lock_held_by_current_thread(const struct lock *);
bool lock_donator_less(const struct list_elem*, const struct list_elem*, void *);
void donate(struct lock*);
#endif /* UCSC CSE130 */
