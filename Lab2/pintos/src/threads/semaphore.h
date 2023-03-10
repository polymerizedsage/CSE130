#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <list.h>
#include <stdbool.h>

/* Semaphore */
struct semaphore {
  unsigned value;               // Current value
  struct list waiters;          // List of waiting threads
  struct list_elem condvarelem; // Condvar waiters list elem
};

void semaphore_init(struct semaphore *, unsigned value);
void semaphore_down(struct semaphore *);
void semaphore_up(struct semaphore *);
bool semaphore_priority_less(const struct list_elem*, const struct list_elem*, void *);
#endif /* UCSC CSE130 */
