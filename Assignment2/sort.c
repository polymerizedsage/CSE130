#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "merge.h"




/* LEFT index and RIGHT index of the sub-array of ARR[] to be sorted */
void singleThreadedMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleThreadedMergeSort(arr, left, middle); 
    singleThreadedMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

struct merge_thing {
    int * arr;
    int left;
    int right;
};

struct merge_args{
  int* arr;
  int left;
  int middle;
  int right;
};

void * merge_thread(void * args){

  struct merge_args * in = (struct merge_args *) args;
  int* arr = in->arr;
  int left = in->left;
  int right = in->right;
  int middle = in->middle;
  //printf("left: %i, middle: %i, right %i \n", left, middle, right);
  merge(arr, left, middle, right);

  pthread_exit(NULL);
  return NULL;
}


void *sort_thread( void * args){
  
  struct merge_thing * in = (struct merge_thing *) args;
  int * arry = in->arr;
  int left = in->left;
  int right = in->right;

  singleThreadedMergeSort(arry, left, right);
  pthread_exit(NULL);
  return NULL;
}


/* 
 * This function stub needs to be completed
 */
void multiThreadedMergeSort(int arr[], int left, int right) 
{
  // Delete this line, it's only here to fail the code quality check
  //int i = 0;

  
  struct merge_thing args_1, args_2, args_3, args_4;
  struct merge_args merge_left, merge_right;
  pthread_t thread1, thread2, thread3, thread4, thread5, thread6;
  int r1, r2, r3, r4, r5, r6;
  //calculate middle
  int middle = (left + right) / 2;

  int fourth = (right - left) / 4;
  int quarter = left + fourth;

  int quarter_end = right - fourth;


  args_1.arr = arr;
  args_1.left = left;
  args_1.right = quarter;

  args_2.arr = arr;
  args_2.left = quarter + 1;
  args_2.right = middle;

  args_3.arr = arr;
  args_3.left = middle + 1;
  args_3.right = quarter_end;
  
  args_4.arr = arr;
  args_4.left = quarter_end + 1;
  args_4.right = right;

  merge_left.arr = arr;
  merge_left.left = left;
  merge_left.middle = quarter;
  merge_left.right = middle;

  merge_right.arr = arr;
  merge_right.left = middle + 1;
  merge_right.middle = quarter_end;
  merge_right.right = right;
  
  //create thread running mergesort on left side
  r1 = pthread_create(&thread1, NULL, sort_thread, (void*) &args_1);
  r2 = pthread_create(&thread2, NULL, sort_thread, (void*) &args_2);
  r3 = pthread_create(&thread3, NULL, sort_thread, (void*) &args_3);
  r4 = pthread_create(&thread4, NULL, sort_thread, (void*) &args_4);
  if(r1 || r2 || r3 || r4){
    printf("threads failed to initialize\n");
    exit(EXIT_FAILURE);
  }

  

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  r5 = pthread_create(&thread5, NULL, merge_thread, (void*) &merge_left);

  if(r5){
    printf("merge left side thread failed to initialize\n");
    exit(EXIT_FAILURE);
  }

  pthread_join(thread3, NULL);
  pthread_join(thread4, NULL);

  r6 = pthread_create(&thread6, NULL, merge_thread, (void*) &merge_right);

  if(r6){
    printf("merge right side thread failed to initialize\n");
    exit(EXIT_FAILURE);
  }
  /*
  printf("before merges\n");
  
  for(int i = left; i < right + 1; i++){
    printf("%i ", arr[i]);
  }
  printf("\n");
  
  printf("left: %i, quarter: %i, middle: %i, 3/4: %i, right %i \n", left, quarter, middle, quarter_end, right);
  */
  pthread_join(thread5, NULL);
  pthread_join(thread6, NULL);
  merge(arr, left, middle, right);
  //exit(EXIT_SUCCESS);
}

