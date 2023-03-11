/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "cartman.h"

#define NUM_JUNCTIONS 17

static sem_t junction_lock[NUM_JUNCTIONS];
static sem_t cart_lock;
int max_track;
/*
carts are threads

all carts have the same code

they will:




however this could deadlock ie is a race condition

best solve is alternate the order that junctions are reserved based on if the cart is on an even or odd track

*/

typedef struct thread_args{

  int cart;
  enum track track;
  enum junction junction;
}thread_args;


void* thread_routine(void* args){
  //Detach the thread
  pthread_detach(pthread_self());

  int * arrive_args = (int *) args;

  int cart = arrive_args[0];
  
  
  int junction = arrive_args[1];
  int track = arrive_args[2];

  //printf("Running thread for cart: %d, at track %d, starting at junction %d\n", cart, track, junction);
  enum junction destination = A; 
  //int track_int = track;
  //int junction_int = junction;


  if(track == max_track){
    if(junction == A){
      destination = max_track;
    }
    else if(junction == max_track){
      destination = A;
    }
  }
  else if(junction == track){
    destination = junction + 1; 
  }
  else if(junction > track){
    destination = junction - 1;
  }

  //printf("Starting crossing for cart: %d, at track %d, starting at junction %d and ending at %d\n", cart, track, junction, destination);
  

  //DEADLOCK AVOIDANCE ALT 
  //ALL CARTS MUST RESERVE THE LOWER NUMBER JUNCTION FIRST

  if(junction < destination){
      sem_wait(&junction_lock[junction]);
      


      sem_wait(&junction_lock[destination]);
      reserve(cart, junction);
      reserve(cart, destination);
  }
  else{
      sem_wait(&junction_lock[destination]);
      

      sem_wait(&junction_lock[junction]);
      reserve(cart, destination);
      reserve(cart, junction);
  }


  //if its an even numbered track pick up right then left, ie the lesser junction number

  /*
  if(track % 2 == 0){

    //junction is on the right
    if(junction < destination){

      sem_wait(&junction_lock[junction]);
      reserve(cart, junction);


      sem_wait(&junction_lock[destination]);
      reserve(cart, destination);
    }
    //destination is on the right
    else{

      sem_wait(&junction_lock[destination]);
      reserve(cart, destination);


      sem_wait(&junction_lock[junction]);
      reserve(cart, junction);
    }

  }
  //odd numbered tracks pick up greater junction number first
  else{
    if(junction > destination){

      sem_wait(&junction_lock[junction]);
      reserve(cart, junction);


      sem_wait(&junction_lock[destination]);
      reserve(cart, destination);
    }
    //destination is on the right
    else{

      sem_wait(&junction_lock[destination]);
      reserve(cart, destination);


      sem_wait(&junction_lock[junction]);
      reserve(cart, junction);
    }
  }

  */
  //try and reserve their first junction (aquire lock from semaphore)
  //Aquire semaphore lock for starting junction

  //printf("Cart %d aquired lock for junction %d\n", cart, junction);
  
  //printf("Cart %d aquired lock for junction %d\n", cart, destination);

  
  
  //reserve their second junciton (aquire second lock) 
  //Aquire semaphore lock for destination junction
  
  
  
  //cross track at given junction 
  cross(cart, track, junction); 

  //give up their reservations (release locks)
  release(cart,junction);
  
  sem_post(&junction_lock[junction]);
  //printf("Cart %d released lock for junction %d\n", cart, junction);
  
  release(cart, destination);
  sem_post(&junction_lock[destination]);
  //printf("Cart %d released lock for junction %d\n", cart, destination);

  free(arrive_args);
  //printf("Completed crossing for cart: %d, at track %d, starting at junction %d and ending at %d\n", cart, track, junction, destination);
  return NULL;
}


/*
 * Callback when CART on TRACK arrives at JUNCTION in preparation for
 * crossing a critical section of track.
 */ 
void arrive(unsigned int cart, enum track track, enum junction junction) 
{
  //printf("incoming cart %d, at junction %d, and track %d\n", cart, junction, track);

  pthread_t cart_thread;
  /*
  thread_args args;
  args.cart = cart;
  args.junction = junction;
  args.track = track;
  */

  int * args = (int *) malloc(3 * sizeof(int));

  args[0] = cart;
  args[1] = junction;
  args[2] = track;

  //The following code was copied from:
  //https://www.cs.fsu.edu/~baker/realtime/restricted/notes/pthreads.html
  //From the section "creating detached threads"
  /*
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  */

  //printf("Initializing thread for cart %d, at junction %d, and track %d\n", args[0], args[1], args[2]);
  int r = pthread_create(&cart_thread, NULL, thread_routine, (void*) args);
    if(r){
      printf("Thread for cart %i failed to initialize\n", cart);
      exit(EXIT_FAILURE);
  }

  
  
}

/*
 * Start the CART Manager for TRACKS tracks.
 *
 * Return is optional, i.e. entering an infinite loop is allowed, but not
 * recommended. Some implementations will do nothing, most will initialise
 * necessary concurrency primitives.
 */
void cartman(unsigned int tracks) 
{
  max_track = tracks - 1;
  sem_init(&cart_lock, 0, 0);
  for(int i = 0; i < tracks; i++){
    sem_init(&junction_lock[i], 0, 1);
  }
  // Remove, only here to fail code warning test
  
}
