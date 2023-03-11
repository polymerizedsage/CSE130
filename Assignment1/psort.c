/************************************************************************
 * 
 * CSE130 Assignment 1
 *  
 * POSIX Shared Memory Multi-Process Merge Sort
 * 
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
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
  int shared_fd = shm_open("shmem_p", O_CREAT | O_RDWR, 0666);

  ftruncate(shared_fd, sizeof(int) * right);

  //int shmid = shmget(key, sizeof(int) * right, 0666 | IPC_CREAT);
  //attach to shared memory
  int* shared_array = (int*) mmap(0, sizeof(int) * right, PROT_WRITE | PROT_READ, MAP_SHARED, shared_fd, 0);
  //copy the array into shared memory
  

  for(int i = left; i < right + 1; i++){
    shared_array[i] = arr[i];
  }

  int middle = (left + right) / 2;
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
    singleProcessMergeSort(shared_array, 0, middle);
    //attach to the shared memory
    //sort left side of memory
    //detach from shared memory
    //exit
    exit(0);
  default:
  //if im the parent
    
    //sort the right side of shared memory
    singleProcessMergeSort(shared_array, middle + 1, right);
    //wait for child to finish
    wait(NULL);
    //merge shared memory
    merge(shared_array, left, middle, right);
    //copy shared memory back into the local array
    for(int i = left; i < right + 1; i++){
      arr[i] = shared_array[i];
    }
    //detach shared memory
    shm_unlink("shmem_p");
    shmdt(shared_array);
    //destroy shared memory
    close(shared_fd);
  }
  
}
