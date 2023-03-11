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

#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "userprog/tss.h"
#include "userprog/elf.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "devices/timer.h"
#include "lib/string.h"

typedef struct list_address{
  void* arg_ptr;
  struct list_elem ptr_elem;
} arg_addr;
/*
 * Push the command and arguments found in CMDLINE onto the stack, word 
 * aligned with the stack pointer ESP. Should only be called after the ELF 
 * format binary has been loaded by elf_load();
 */
static void
push_command(char* argv[], int argc, void **esp)
{
  //struct list args_address; 
  //list_init(&args_address);
  char** argv_addr[argc];
  //printf("calling thread name: %s\n", thread_current()->name);
  //printf("Base Address: 0x%08x\n", (unsigned int)*esp);

  // Word align with the stack pointer.
  *esp = (void *)((unsigned int)(*esp) & 0xfffffffc);

  // Some of your CSE130 Lab 3 code will go here.
  //
  // You'll be doing address arithmetic here and that's one of only a handful
  // of situations in which it is acceptable to have comments inside functions.
  //
  // As you advance the stack pointer by adding fixed and variable offsets
  // to it, add a SINGLE LINE comment to each logical block, a comment that
  // describes what you're doing, and why.
  //
  // If nothing else, it'll remind you what you did when it doesn't work :)


  //insert arg strings into stack
  
  //(*esp) -= sizeof(argv) ;
  //printf("esp at %p\n", *esp);
  for (int i = argc - 1; i >= 0; --i){

    
    //printf("writing string %s\n", argv[i]);
    int s_len = strlen(argv[i]) + 1;
    *esp -= s_len;
    memcpy(*esp, argv[i], s_len);
    argv_addr[i] = *esp;
  }

  //word align   
  *esp = (void *)((unsigned int)(*esp) & 0xfffffffc);
  //printf("esp at %p after alignment\n", *esp);
  //null sentinel
  (*esp) -= sizeof(char*);
  //printf("esp at %p after null sent\n", *esp);

  for(int i = argc - 1; i >= 0; --i){
    *esp -= sizeof(char*);
    memcpy(*esp, &(argv_addr[i]), sizeof(char*));
  }


  
  const char** argv_ptr = *esp;
  
  //printf("esp at %p after argv contents\n", *esp);
  *esp -= sizeof(char**);
  memcpy(*esp, &argv_ptr, sizeof(char**));
  
  //printf("esp at %p after argv\n", *esp);

  //int one_ = 1;

  //memcpy(*esp, &one_, sizeof(int));
  *esp -= sizeof(int);
  *((int*) *esp) = argc;
  
  //printf("esp at %p after argc\n", *esp);




  *esp -= sizeof(void*);
  //printf("esp at %p after return\n", *esp);

}

/* 
 * A thread function to load a user process and start it running. 
 * CMDLINE is assumed to contain an executable file name with no arguments.
 * If arguments are passed in CMDLINE, the thread will exit imediately.
 */
static void
start_process(void *cmdline)
{
  // Initialize interrupt frame and load executable.
  struct intr_frame pif;
  memset(&pif, 0, sizeof pif);
  
  pif.gs = pif.fs = pif.es = pif.ds = pif.ss = SEL_UDSEG;
  pif.cs = SEL_UCSEG;
  pif.eflags = FLAG_IF | FLAG_MBS;

  //char s[strlen(cmdline)];
  char* save_ptr, *token;
  char* argv[strlen(cmdline)]; 
  int argc = 0; 

  //strlcpy(s, cmdline, strlen(cmdline) + 1);
  //tokenize cmd string
  for (token = strtok_r (cmdline, " ", &save_ptr); token != NULL; token = strtok_r (NULL, " ", &save_ptr)){
    argv[argc] = token;
    argc++;
  }
  bool loaded = elf_load(argv[0], &pif.eip, &pif.esp);
  //printf("here i am\n");
  if (loaded)
    push_command(argv, argc, &pif.esp);
 
  palloc_free_page(cmdline);
  
  if (!loaded)
    thread_exit();

  // Start the user process by simulating a return from an
  // interrupt, implemented by intr_exit (in threads/intr-stubs.S).
  // Because intr_exit takes all of its arguments on the stack in
  // the form of a `struct intr_frame',  we just point the stack
  // pointer (%esp) to our stack frame and jump to it.
  asm volatile("movl %0, %%esp; jmp intr_exit" : : "g"(&pif) : "memory");

  printf("reached after finish?\n");
  NOT_REACHED();
}

/*  
 * Starts a new kernel thread running a user program loaded from CMDLINE. 
 * The new thread may be scheduled (and may even exit) before process_execute() 
 * returns.  Returns the new process's thread id, or TID_ERROR if the thread 
 * could not be created. 
 */
tid_t 
process_execute(const char *cmdline)
{
  // Make a copy of CMDLINE to avoid a race condition between the caller 
  // and elf_load()
  char* save_ptr;
  char s[strlen(cmdline)];
  char *cmdline_copy = palloc_get_page(0);
  if (cmdline_copy == NULL)
    return TID_ERROR;

  strlcpy(cmdline_copy, cmdline, PGSIZE);
  strlcpy(s, cmdline, strlen(cmdline) + 1);
  // Create a Kernel Thread for the new process
  tid_t tid = thread_create(strtok_r (s, " ", &save_ptr), PRI_DEFAULT, start_process, cmdline_copy);
  thread_current()->child_tid = tid;
  // CSE130 Lab 3 : The "parent" thread immediately returns after creating
  // the child. To get ANY of the tests passing, you need to synchronise the
  // activity of the parent and child threads.
  //process_wait(tid);
 
  //timer_msleep(1000);
  semaphore_down(thread_current()->child_lock);

  return tid;
}

/* 
 * Waits for thread TID to die and returns its exit status.  If it was 
 * terminated by the kernel (i.e. killed due to an exception), returns -1.
 * If TID is invalid or if it was not a child of the calling process, or 
 * if process_wait() has already been successfully called for the given TID, 
 * returns -1 immediately, without waiting.
 *
 * This function will be implemented in CSE130 Lab 3. For now, it does nothing. 
 */
int 
process_wait(tid_t child_tid)
{
  if(thread_current()->child_tid == child_tid){
    semaphore_down(thread_current()->child_lock);
    int exit = thread_current()->child_exit;
    thread_current()->child_exit = -1;
    return exit;
  }
  else{
    return -1;
  }
  return -1;
}

/* Free the current process's resources. */
void 
process_exit(void)
{
  struct thread *cur = thread_current();
  uint32_t *pd;

  // Destroy the current process's page directory and switch back
  // to the kernel-only page directory. 
  
  pd = cur->pagedir;
  if (pd != NULL)
  {
    // Correct ordering here is crucial.  We must set
    // cur->pagedir to NULL before switching page directories,
    // so that a timer interrupt can't switch back to the
    // process page directory.  We must activate the base page
    // directory before destroying the process's page
    // directory, or our active page directory will be one
    // that's been freed (and cleared). 
    cur->pagedir = NULL;
    pagedir_activate(NULL);
    pagedir_destroy(pd);
  }
  semaphore_up(&cur->parent_lock);
}

/* 
 * Sets up the CPU for running user code in the current thread.
 * This function is called on every context switch. 
 */
void 
process_activate(void)
{
  struct thread *t = thread_current();

  // Activate thread's page tables. 
  pagedir_activate(t->pagedir);

  // Set thread's kernel stack for use in processing interrupts. 
  tss_update();
}
