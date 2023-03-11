/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "manpage.h"

#define THREAD_NUM 7
/*
 * Create a thread for each of seven manpage paragraphs and have them synchronize
 * their invocations of showParagraph() so the manual page displays as follows:
 *
 --------- 8< ---------------------------------------------------
 
A semaphore S is an unsigned-integer-valued variable.
Two operations are of primary interest:
 
P(S): If processes have been blocked waiting on this semaphore,
 wake one of them, else S <- S + 1.
 
V(S): If S > 0 then S <- S - 1, else suspend execution of the calling process.
 The calling process is said to be blocked on the semaphore S.
 
A semaphore S has the following properties:

1. P(S) and V(S) are atomic instructions. Specifically, no
 instructions can be interleaved between the test that S > 0 and the
 decrement of S or the suspension of the calling process.
 
2. A semaphore must be given an non-negative initial value.
 
3. The V(S) operation must wake one of the suspended processes. The
 definition does not specify which process will be awakened.

 --------- 8< ---------------------------------------------------
 *
 * As supplied, shows random single messages.
 */

void* thread_routine(void * args){
  int pid = getParagraphId();
  //printf("running thread with pid: %i\n", pid);
  sem_t * lock_list = (sem_t*) args;
  

  if(pid == 0 ){
    showParagraph();
    sem_post(&lock_list[pid + 1]);
  }

  else if(pid == THREAD_NUM - 1 ){
    sem_wait(&lock_list[pid]);
    showParagraph();

    for(int i = 0; i < THREAD_NUM; i++){
      sem_destroy(&lock_list[i]);
    }
  }

  else{
    sem_wait(&lock_list[pid]);
    showParagraph();
    sem_post(&lock_list[pid + 1]);
  }

  pthread_exit(NULL);
  return NULL;
}

void manpage() 
{


  //down(pid)
  //showPara
  //up(pid + 1) 
  //int pid = getParagraphId(); // pid = 'Paragraph Id"
  sem_t locks_[THREAD_NUM]; 
  pthread_t threads[THREAD_NUM];

  //init semaphores list
  for(int i = 0; i < THREAD_NUM; i++){
      sem_init(&locks_[i], 0, 0);
  }
  //init threads
  for(int i = 0; i < THREAD_NUM; i ++ ){
    //printf("initializing thread %i \n", i);
    int r = pthread_create(&threads[i], NULL, thread_routine, (void*) &locks_);
    if(r){
      printf("Thread %i failed to initialize\n", i);
      exit(EXIT_FAILURE);
    }
  }
  //init semaphores
  
  for(int i = 0; i < THREAD_NUM; i++){
    pthread_join(threads[i], NULL);
  }
  
}


/*
With a lock and conditional variable:

aquire lock
while ( turn == pid )
  wait
showPara
turn++
broadcast
unlock

With locks/binary semaphore:



this requires special cases for the first and last thread
*/
