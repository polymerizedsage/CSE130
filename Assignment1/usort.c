/************************************************************************
 * 
 * CSE130 Assignment 1
 * 
 * UNIX Shared Memory Multi-Process Merge Sort
 * 
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ************************************************************************/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "merge.h"

/* 
 * Merge Sort in the current process a sub-array of ARR[] defined by the 
 * LEFT and RIGHT indexes.
 */
void singleProcessMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleProcessMergeSort(arr, left, middle); 
    singleProcessMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

/* 
 * Merge Sort in the current process and at least one child process a 
 * sub-array of ARR[] defined by the LEFT and RIGHT indexes.
 */
void multiProcessMergeSort(int arr[], int left, int right) 
{
  // Delete this line, it's only here to fail the code quality check

  // Your code goes here 

  //create shared memory
  key_t key = ftok("shmem", 65);

  int shmid = shmget(key, sizeof(int) * right, 0666 | IPC_CREAT);
  //attach to shared memory
  int* shared_array = (int*) shmat(shmid, (void*) 0, 0);
  //copy the array into shared memory
  

  

  int middle = (left + right) / 2;

  for(int i = middle + 1; i < right + 1; i++){
    shared_array[i] = arr[i];
  }
  //fork into two processes
  //switch based on the fork
  switch (fork()) {
  //if error
  case -1:
    fprintf(stderr, "Fork failed \n");
    exit(EXIT_FAILURE);
    //exit

  //if im the child
  case 0:

    //int* shared_right = (int*) shmat(shmid, (void*) 0, 0);
    singleProcessMergeSort(shared_array, middle + 1, right);
    //shmdt(shared_array);
    //attach to the shared memory
    //sort left side of memory
    //detach from shared memory
    //exit
    exit(0);
  default:
  //if im the parent
    
    //sort the right side of shared memory
    singleProcessMergeSort(arr, 0, middle);
    //wait for child to finish
    wait(NULL);
    //merge shared memory
    
    //copy shared memory back into the local array
    for(int i = middle + 1; i < right + 1; i++){
      arr[i] = shared_array[i];
    }

    
    //detach shared memory
    shmdt(shared_array);
    //destroy shared memory
    shmctl(shmid, IPC_RMID, NULL);

    merge(arr, left, middle, right);
  }
  
}
