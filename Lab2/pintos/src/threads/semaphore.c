/* 
 * This file is derived from source code for the Pintos
 * instructional operating system which is itself derived
 * from the Nachos instructional operating system. The 
 * Nachos copyright notice is reproduced in full below. 
 *
 * Copyright (C) 1992-1996 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose, without fee, and
 * without written agreement is hereby granted, provided that the
 * above copyright notice and the following two paragraphs appear
 * in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
 * AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 *
 * Modifications Copyright (C) 2017-2021 David C. Harrison. 
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>


#include "threads/semaphore.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* 
 * Initializes semaphore SEMA to VALUE.  A semaphore is a
 * nonnegative integer along with two atomic operators for
 * manipulating it:
 *
 * - down or Dijkstra's "P": wait for the value to become positive, 
 * 	then decrement it.
 * - up or Dijkstra's "V": increment the value(and wake up one waiting 
 * 	thread, if any). 
 */
void semaphore_init(struct semaphore *sema, unsigned value)
{
  ASSERT(sema != NULL);

  sema->value = value;
  list_init(&sema->waiters);
}

/* 
 * Down or Dijkstra's "P" operation on a semaphore.  Waits for SEMA's 
 * value to become positive and then atomically decrements it.
 *
 * This function may sleep, so it must not be called within an
 * interrupt handler.  This function may be called with
 * interrupts disabled, but if it sleeps then the next scheduled
 * thread will probably turn interrupts back on. 
 */
void semaphore_down(struct semaphore *sema)
{
  ASSERT(sema != NULL);
  ASSERT(!intr_context());

  enum intr_level old_level = intr_disable();
  
  while (sema->value == 0){
  //if(sema->value == 0){
    list_push_front(&sema->waiters, &thread_current()->sharedelem);
    thread_block();
  }
  

  sema->value--;
  intr_set_level(old_level);
}

/* 
 * Up or Dijkstra's "V" operation on a semaphore.  Increments SEMA's value
 * and wakes up one thread of those waiting for SEMA, if any.
 *
 * This function may be called from an interrupt handler. 
 */
void semaphore_up(struct semaphore *semaphore)
{
  enum intr_level old_level;

  ASSERT(semaphore != NULL);

  old_level = intr_disable();
  semaphore->value++;
  if (!list_empty(&semaphore->waiters))
  {
    //sort the waiting threads list
    list_sort(&semaphore->waiters, thread_priority_less, NULL);

    struct thread* to_run = list_entry(list_pop_back(&semaphore->waiters), struct thread, sharedelem);
    thread_unblock(to_run);
    if(to_run->priority > thread_current()->priority){
      thread_yield();
    }
  //yeild if the thread on the CPU has lower priority than the popped thread
  
  }
  
  intr_set_level(old_level);
}
//Compares two semaphores highest priority waiting threads, returns true if A is lower priority than B
bool semaphore_priority_less(const struct list_elem *a, const struct list_elem *b, UNUSED void *aux){
  struct semaphore* sem_a = list_entry(a, struct semaphore, condvarelem);
  struct semaphore* sem_b = list_entry(b, struct semaphore, condvarelem);

  struct thread* thread_a = list_entry(list_front(&sem_a->waiters), struct thread, sharedelem);
  struct thread* thread_b = list_entry(list_front(&sem_b->waiters), struct thread, sharedelem);

  return thread_a->priority < thread_b->priority;
}
