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
#include <syscall-nr.h>
#include <list.h>
#include <string.h>
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "userprog/umem.h"


/****************** System Call Implementations ********************/

/*
 * BUFFER+0 should be a valid user adresses
 */
void sys_exit(int exitcode) 
{
  thread_current()->parent->child_exit = exitcode;
  printf("%s: exit(%d)\n", thread_current()->name, exitcode);
  thread_exit();
}

static void exit_handler(struct intr_frame *f) 
{
  int exitcode;
  umem_read(f->esp + 4, &exitcode, sizeof(exitcode));
  sys_exit(exitcode);
}

/*
 * BUFFER+0 and BUFFER+size should be valid user adresses
 */
static uint32_t sys_write(int fd, const void *buffer, unsigned size)
{
  //umem_check((const uint8_t*) buffer);
  //umem_check((const uint8_t*) buffer + size - 1);

  int ret = -1;
  if(umem_get((const uint8_t*) buffer) == -1) sys_exit(-1);
  //printf("writeing to file with fd %i", fd);
  if (fd == 1) { // write to stdout
    putbuf(buffer, size);
    ret = size;
  }
  else if(!(fd < 3 || fd > 32)){
    struct file* wf = thread_current()->oft[fd - 3];
    ret = file_write(wf, buffer, size);
  }
  else{
    sys_exit(-1);
  }

  return (uint32_t) ret;
}

static void write_handler(struct intr_frame *f)
{
    int fd;
    const void *buffer;
    unsigned size;

    umem_read(f->esp + 4, &fd, sizeof(fd));
    umem_read(f->esp + 8, &buffer, sizeof(buffer));
    umem_read(f->esp + 12, &size, sizeof(size));

    f->eax = sys_write(fd, buffer, size);
}

static uint32_t sys_create(const char *file, unsigned initial_size){
    if(file == NULL){
      sys_exit(-1);
    }

    if(umem_get((const uint8_t*) file) == -1){
      sys_exit(-1);
    };
    uint32_t status = filesys_create (file, initial_size, false);
    return status;
}


static void create_handler(struct intr_frame *f){
    const char* file;
    unsigned init_size;

    umem_read(f->esp + 4, &file, sizeof(file));
    umem_read(f->esp + 8, &init_size, sizeof(init_size));

    
    f->eax = sys_create(file, init_size);
}


static int sys_open(const char* file){
  if(file == NULL || umem_get((const uint8_t*) file) == -1){
    sys_exit(-1);
  }
  struct file* of = filesys_open (file);
  struct file** oft = thread_current()->oft;
  int fd = thread_current()->max_fd++;
  thread_current()->max_fd++;
  //printf("oft of size %i \n", list_size(&oft));
  if(of == NULL){
    return -1;
  }
  oft[fd - 3] = of;
  return fd;

}


static void open_handler(struct intr_frame *f){
  const char* file;

  umem_read(f->esp + 4, &file, sizeof(file));
  f->eax = sys_open(file);
}

static int sys_read(int fd, void *buffer, unsigned size){
    
    if(umem_get((const uint8_t*) buffer) == -1) sys_exit(-1);
    if(fd < 3 || fd > 32){
      return -1;
    }
    struct file* rf = thread_current()->oft[fd - 3];
    if(rf == NULL){
      return -1;
    }


    int count = file_read(rf, buffer, size);

    return count;
}

static void read_handler(struct intr_frame* f){
  int fd;
  void *buffer;
  unsigned size;

  umem_read(f->esp + 4, &fd, sizeof(fd));
  umem_read(f->esp + 8, &buffer, sizeof(buffer));
  umem_read(f->esp + 12, &size, sizeof(size));
  f->eax = sys_read(fd, buffer, size);
}

static int sys_filesize(int fd){

  return file_length(thread_current()->oft[fd - 3]);

}

static void filesize_handler(struct intr_frame* f){
  int fd;

  umem_read(f->esp + 4, &fd, sizeof(fd));
  f->eax = sys_filesize(fd);
}


static void sys_close(int fd){
  if(fd < 3 || fd > 32){
      sys_exit(-1);
  }
  thread_current()->oft[fd - 3] = NULL;
}

static void close_handler(struct intr_frame* f){
  int fd;

  umem_read(f->esp + 4, &fd, sizeof(fd));
  sys_close(fd);
}

static int sys_wait (int pid_t){
  
  return process_wait(pid_t);
}

static void wait_handler(struct intr_frame* f){
  int pid;

  umem_read(f->esp + 4, &pid, sizeof(pid));
  f->eax = sys_wait(pid);
}

static int sys_exec(const char* file){
  if(umem_get((const uint8_t*) file) == -1) sys_exit(-1);
  
  int tid = process_execute(file);
  struct file* f = filesys_open(file);
  if(f == NULL) return -1;
  return tid;
}


static void exec_handler(struct intr_frame* f){
  const char* file;

  umem_read(f->esp + 4, &file, sizeof(file));
  f->eax = sys_exec(file);
}


/****************** System Call Handler ********************/

static void
syscall_handler(struct intr_frame *f)
{
  int syscall;
  ASSERT( sizeof(syscall) == 4 ); // assuming x86

  // The system call number is in the 32-bit word at the caller's stack pointer.
  umem_read(f->esp, &syscall, sizeof(syscall));

  // Store the stack pointer esp, which is needed in the page fault handler.
  // Do NOT remove this line
  thread_current()->current_esp = f->esp;

  switch (syscall) {
  case SYS_HALT: 
    shutdown_power_off();
    break;

  case SYS_EXIT: 
    exit_handler(f);
    break;
      
  case SYS_WRITE: 
    write_handler(f);
    break;

  case SYS_CREATE:
    create_handler(f);
    break;

  case SYS_OPEN:
    open_handler(f);
    break;

  case SYS_FILESIZE:
    filesize_handler(f);
    break;

  case SYS_READ:
    read_handler(f);
    break;

  case SYS_CLOSE:
    close_handler(f);
    break;

  case SYS_EXEC:
    exec_handler(f);
    break;
  
  case SYS_WAIT:
    wait_handler(f);
    break;
  default:
    printf("[ERROR] system call %d is unimplemented!\n", syscall);
    thread_exit();
    break;
  }
}

void
syscall_init (void)
{

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}




