/**
 * See scheduler.h for function details. All are callbacks; i.e. the simulator 
 * calls you when something interesting happens.
 */
#include <stdlib.h>
#include <stdio.h>
#include "simulator.h"
#include "scheduler.h"
#include "queue.h"


typedef struct _twrapper_t {
  thread_t* t;
  int wait; 
  int arrive; 
  int start;
  int end;
  int tick;
} twrapper_t;

static void* ready;
static void* all;
static twrapper_t* running = NULL;
static enum algorithm a_type;
static unsigned int gquantum;

static bool thread_equalitor(void *wrapper, void *thread) {
  return ((twrapper_t*)wrapper)->t == (thread_t*)thread;
}

static int thread_comparator(void *a, void *b) {

  int out = ((twrapper_t*)a)->t->priority - ((twrapper_t*)b)->t->priority;
  return out;
}

void run_thread(){

    //get thread to run
    
    switch (a_type)
    {
      case NON_PREEMPTIVE_PRIORITY:
          if(running == NULL){
            queue_sort(ready, thread_comparator);
            running = queue_dequeue(ready);
          }
          else return;
          break;
      case PREEMPTIVE_PRIORITY:
        if (running == NULL){
          queue_sort(ready, thread_comparator);
          running = queue_dequeue(ready);
        }
        else if(queue_size(ready)){
          queue_sort(ready, thread_comparator);
          if(((twrapper_t*)queue_head(ready))->t->priority < running->t->priority){
            twrapper_t* running_old = running;
            running = queue_dequeue(ready);
            queue_enqueue(ready, running_old);

            running_old->wait -= (sim_time());
          }
          else return;
        }
        break;
      case FIRST_COME_FIRST_SERVED:
        if(running == NULL)
          running = queue_dequeue(ready);
        else return;
        break;
      case ROUND_ROBIN:
        if(running == NULL){
          running = queue_dequeue(ready);
          running->tick = gquantum;
        }else if(running->tick <= 0){
          twrapper_t* running_old = running;
          running = queue_dequeue(ready);
          queue_enqueue(ready, running_old);
          running->tick = gquantum;
          running_old->wait -= (sim_time() );
        }
        else return;
        break;
      default:
        return;
        break;
    }
    
    
    running->wait += sim_time();
    
    printf("Scheduling thread %d at tick: %d, waiting %d, ticks: %d\n", running->t->tid, sim_time(), running->wait, running->tick);
    
    //printf("Thread %d executing\n", running->tid);
    sim_dispatch(running->t);
}

//Wait time is time spent not in IO or on CPU

/**
 * Initalise a scheduler implemeting the requested ALGORITHM. QUANTUM is only
 * meaningful for ROUND_ROBIN.
 */
void scheduler(enum algorithm algorithm, unsigned int quantum) {
  ready = queue_create();
  all = queue_create();
  a_type = algorithm;
  gquantum = quantum;
}

/**
 * Thread T is ready to be scheduled for the first time.
 */
void sys_exec(thread_t *t) {
  //printf("Thread %d incoming at tick %d\n", t->tid, sim_time());
  //add to ready queue

  twrapper_t* T  = calloc(1, sizeof(twrapper_t));
  T->t = t;
  T->wait -= sim_time();
  T->arrive = sim_time();
  T->start = 0;
  T->end = 0;
  T->tick = gquantum;
  queue_enqueue(all, T);
  queue_enqueue(ready, T);
  
  //run_thread();
}

/**
 * Programmable clock interrupt handler. Call sim_time() to find out
 * what tick this is. Called after all calls to sys_exec() for this
 * tick have been made.
 */
void tick() { 
  //printf("Tick\n");
  //dispatch from the ready queue if nothing is on the cpu
  
  //starting new thread
  if(queue_size(ready) > 0){

    //get thread to run
    run_thread();
   
  }
  else{
    if(running != NULL && running->tick <= 0){
      running->tick = gquantum;
    }
  }

  if(a_type == ROUND_ROBIN && running != NULL){
    running->tick--;
  }
}

/**
 * Thread T has completed execution and should never again be scheduled.
 */
void sys_exit(thread_t *t) {

  running->end = sim_time() + 1;
  //twrapper_t* exiting = queue_find(all, thread_equalitor, t);

  printf("Thread %d exiting\n", t->tid);
  running = NULL;

}

/**
 * Thread T has requested a read operation and is now in an I/O wait queue.
 * When the read operation starts, io_starting(T) will be called, when the
 * read operation completes, io_complete(T) will be called.
 */
void sys_read(thread_t *t) {
  //twrapper_t* T = queue_find(all, thread_equalitor, t);
  running->wait -= (sim_time() + 1);
  running = NULL;
}

/**
 * Thread T has requested a write operation and is now in an I/O wait queue.
 * When the write operation starts, io_starting(T) will be called, when the
 * write operation completes, io_complete(T) will be called.
 */
void sys_write(thread_t *t) {
  sys_read(t);
}

/**
 * An I/O operation requested by T has completed; T is now ready to be 
 * scheduled again.
 */
void io_complete(thread_t *t) {
  twrapper_t* T = queue_find(all, thread_equalitor, t);
  T->wait -= (sim_time() + 1);
  queue_enqueue(ready, T);
  
  //IO is done, return the thread to the ready queue
}

/**
 * An I/O operation requested by T is starting; T will not be ready for
 * scheduling until the operation completes.
 */
void io_starting(thread_t *t) {
  twrapper_t* T = queue_find(all, thread_equalitor, t);
  T->wait += sim_time();
}

/**
 * Return dynamically allocated stats for the scheduler simulation, see 
 * scheduler.h for details. Memory allocated by your code will be free'd
 * by the similulator. Do NOT return a pointer to a stack variable.
 */
stats_t *stats() {
  //int i = 0; // Remove - only here to fail code warnings check
  int total_wait = 0;
  int total_turnaround = 0;
  stats_t *stats = malloc(sizeof(stats_t));
  stats->thread_count = queue_size(all);
  stats->tstats = malloc(sizeof(stats_t)*stats->thread_count);
  
  
  
  
  for(int i = 0; i < stats->thread_count; i++){
    
    twrapper_t *T = queue_dequeue(all);
    
    stats_t* tstat = &stats->tstats[i];
    
    tstat->tid = T->t->tid;
    //printf("compiling stats\n");
    printf("thread %d with waiting of %d\n", T->t->tid, T->wait);
    tstat->waiting_time = T->wait;
    total_wait += tstat->waiting_time;
    tstat->turnaround_time = T->end - T->arrive;
    total_turnaround += tstat->turnaround_time;

    //printf("Thread %d waited %d total, turnaround %d\n", tstat->tid, tstat->waiting_time, tstat->turnaround_time);
    free(T);
  }
  printf("total waiting time %d\n", total_wait);
  stats->waiting_time = total_wait / stats->thread_count;
  stats->turnaround_time = total_turnaround / stats->thread_count;
  
  
  return stats; 
}
