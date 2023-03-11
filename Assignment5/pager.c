/************************************************************************
 *
 * CSE130 Assignment 5
 *
 * Copyright (C) 2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 * 
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "simulator.h"
#include "queue.h"

static void* page_queue;
static enum algorithm a_type;
static unsigned int frame;

typedef struct _page{
    unsigned int page_num;
    int last_request;
    int frequency; 
} Page;

static bool inner_equalitor(void *outer, void *inner) {
    return ((Page*)outer)->page_num == *(int*)inner;
}

static int recent_comparator(void *a, void *b) {
    return ((Page*)a)->last_request - ((Page*)b)->last_request;
}

static int freq_comparator(void *a, void *b) {
    int freq = ((Page*)a)->frequency - ((Page*)b)->frequency;
    if(freq == 0 && ((Page*)a)->frequency > 1){
        return ((Page*)b)->last_request - ((Page*)a)->last_request;
    }
    return freq;
}
/**
 * Initialise your ALGORITHM pager with FRAMES frames (physical pages).
 */
void pager_init(enum algorithm algorithm, unsigned int frames) {
    a_type = algorithm;
    frame = frames;
    page_queue = queue_create();
    switch (a_type)
    {
    case FIRST_IN_FIRST_OUT:
        
        break;
    
    default:
        break;
    }
    //printf("init algorithm %d with %d frames", a_type, frames);
}

/**
 * Free any dynamically allocated resources.
 */
void pager_destroy() {
    while(queue_size(page_queue) > 0){
        free(queue_dequeue(page_queue));
    }
    queue_destroy(page_queue);

    
}

/**
 * A request has been made for virtual page PAGE. If your pager does
 * not currently have PAGE in physical memory, request it from the 
 * simulator.
 */
void pager_request(unsigned int page) {
    Page* page_idx = (Page*) queue_find(page_queue, inner_equalitor, &page);
    switch (a_type)
    {
    case FIRST_IN_FIRST_OUT:
        //printf("Requesting page %d\n", page);
        if(!page_idx){
            sim_get_page(page);
            if(queue_size(page_queue) >= frame){
                Page* free_p = queue_dequeue(page_queue);
                //printf("Page released %d", free_p->page_num);
                free(free_p);
            }
            Page* new_page = calloc(1, sizeof(Page));
            new_page->page_num = page;
            queue_enqueue(page_queue, new_page);
        }
        break;
    case LEAST_RECENTLY_USED:
        if(!page_idx){

            sim_get_page(page);
            queue_sort(page_queue, recent_comparator);
            if(queue_size(page_queue) >= frame){
                Page* free_p = queue_dequeue(page_queue);
                //printf("Page released %d", free_p->page_num);
                free(free_p);
            }
            Page* new_page = calloc(1, sizeof(Page));
            new_page->page_num = page;
            new_page->last_request = sim_time();
            queue_enqueue(page_queue, new_page);
        }else{
            page_idx->last_request = sim_time();
        }
        break;
    case LEAST_FREQUENTLY_USED:
        if(!page_idx){
            queue_sort(page_queue, freq_comparator);
            sim_get_page(page);
            
            
            if(queue_size(page_queue) >= frame){
                Page* free_p = queue_dequeue(page_queue);
                printf("Page released %d with frequency of %d, arrival time %d\n", free_p->page_num, free_p->frequency, free_p->last_request);
                free(free_p);
            }
            Page* new_page = calloc(1, sizeof(Page));
            new_page->page_num = page;
            new_page->frequency = 1;
            new_page->last_request = sim_time();
            queue_enqueue(page_queue, new_page);
            queue_sort(page_queue, freq_comparator);
           
        }else{
            page_idx->frequency++;
            page_idx->last_request = sim_time();
        }
        break;
        
    case SECOND_CHANCE:
        //printf("Requesting page %d\n", page);
        if(!page_idx){
            sim_get_page(page);
            if(queue_size(page_queue) >= frame){
                Page* check = queue_dequeue(page_queue);
                while (check->last_request == 1)
                {
                    
                    check->last_request = 0;
                    queue_enqueue(page_queue, check);
                    check = queue_dequeue(page_queue);
                }
                //printf("Page released %d\n", check->page_num);
                free(check);
                
                
                //free(free_p);
            }
            Page* new_page = calloc(1, sizeof(Page));
            new_page->page_num = page;
            new_page->last_request = 0;
            queue_enqueue(page_queue, new_page);
        }
        else{
            page_idx->last_request = 1;
        }
        break;
    case CIRCULAR_QUEUE:
        if(!page_idx){
            sim_get_page(page);
            if(queue_size(page_queue) >= frame){
                Page* check = queue_dequeue(page_queue);
                while (check->last_request == 1)
                {
                    
                    check->last_request = 0;
                    queue_enqueue(page_queue, check);
                    check = queue_dequeue(page_queue);
                }
                //printf("Page released %d\n", check->page_num);
                free(check);
                
                
                //free(free_p);
            }
            Page* new_page = calloc(1, sizeof(Page));
            new_page->page_num = page;
            new_page->last_request = 1;
            queue_enqueue(page_queue, new_page);
        }
        else{
            page_idx->last_request = 1;
        }
        break;
    default:
        break;
    }
}