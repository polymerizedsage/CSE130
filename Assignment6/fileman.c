/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define BUF_SIZE 4096
#define PATH_LEN 60
/*
 * Extended ASCII box drawing characters:
 * 
 * The following code:
 * 
 * printf("CSE130\n");
 * printf("%s%s Assignments\n", TEE, HOR);
 * printf("%s  %s%s Assignment 1\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 2\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 3\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 4\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 5\n", VER, TEE, HOR);
 * printf("%s%s Labs\n", ELB, HOR);
 * printf("   %s%s Lab 1\n", TEE, HOR);
 * printf("   %s%s Lab 2\n", TEE, HOR);
 * printf("   %s%s Lab 3\n", ELB, HOR);
 * printf();
 * 
 * Shows this tree:
 * 
 * CSE130
 * ├─ Assignments
 * │  ├─ Assignment 1
 * │  ├─ Assignment 2
 * │  ├─ Assignment 3
 * │  ├─ Assignment 4
 * │  ├─ Assignment 5
 * └─ Labs
 *    ├─ Lab 1
 *    ├─ Lab 2
 *    └─ Lab 3
 */
#define TEE "\u251C"  // ├ 
#define HOR "\u2500"  // ─ 
#define VER "\u2502"  // │
#define ELB "\u2514"  // └

/*
 * Read at most SIZE bytes from FNAME starting at FOFFSET into BUF starting 
 * at BOFFSET.
 *
 * RETURN number of bytes read from FNAME into BUF, -1 on error.
 */
int fileman_read(
    const char *fname, 
    const size_t foffset, 
    const char *buf, 
    const size_t boffset, 
    const size_t size) 
{

  int fid = open(fname, O_RDONLY);
  if(fid == -1){
    return -1;
  }

  int seek = lseek(fid, foffset, SEEK_SET);
  if (seek == -1) return -1;
  int count = read(fid, (void*) (&buf[boffset]), size);

  if(count == -1) return -1;

  int err = close(fid);
  if(err == -1) return -1;
  return count;
}

/*
 * Create FNAME and write SIZE bytes from BUF starting at BOFFSET into FNAME
 * starting at FOFFSET.
 * 
 * RETURN number of bytes from BUF written to FNAME, -1 on error or if FILE 
 * already exists
 */
int fileman_write(
    const char *fname, 
    const size_t foffset, 
    const char *buf, 
    const size_t boffset, 
    const size_t size) 
{
  
  int f_exists = open(fname, O_WRONLY);
  if(f_exists != -1){
    close(f_exists);
    return -1;
  }
  
  int fid = creat(fname, S_IRWXU);
  if(fid == -1){
    return -1;
  }

  if(lseek(fid, foffset, SEEK_SET) == -1) return -1;
  int count = write(fid, &(buf[boffset]), size);

  if(count == -1) return -1;
  
  if(fsync(fid) == -1) return -1;
  int err = close(fid);
  if(err == -1) return -1;

  //printf("Wrote %d characters of %d intended\n", count, size);
  return count;
}

/*
 * Append SIZE bytes from BUF to existing FNAME.
 * 
 * RETURN number of bytes from BUF appended to FNAME, -1 on error or if FNAME
 * does not exist
 */
int fileman_append(const char *fname, const char *buf, const size_t size) 
{
  int fid = open(fname, O_WRONLY);
  
  if(fid == -1){
    return -1;
  }
  lseek(fid, 0, SEEK_END);
  int count = write(fid, buf, size) ;
  if(count == -1){
    close(fid);
    return -1;
  }

  if(fsync(fid) == -1){
    close(fid);
    return -1;
    
  } 
  
  if(close(fid) == -1) return -1;
  return count;
}

/*
 * Copy existing file FSRC to new file FDEST.
 *
 * Do not assume anything about the size of FSRC. 
 * 
 * RETURN number of bytes from FSRC written to FDEST, -1 on error, or if FSRC 
 * does not exists, or if SDEST already exists.
 */
int fileman_copy(const char *fsrc, const char *fdest) 
{
  int f_exists = open(fdest, O_WRONLY);
  if(f_exists != -1){
    close(f_exists);
    return -1;
  }

  int srcid = open(fsrc, O_RDONLY);
  if(srcid == -1){
    return -1;
  }


  int destid = creat(fdest, S_IRWXU);
  if(destid == -1){
    close(srcid);
    return -1;
  }


  int* buf = calloc(1, BUF_SIZE);
  int count_total = 0;
  int count = 0;
  do
  {
    count = read(srcid, buf, BUF_SIZE);
    if(write(destid, buf, count) == -1){
      close(destid);
      close(srcid);
      return -1;
    }
    count_total += count;
  } while (count == BUF_SIZE);
  

  close(destid);
  close(srcid);
  free(buf);
  return count_total;

}

//Found on GNU directory access documentation 
//https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister-Mark-II.html
static int
one (const struct dirent *unused)
{
  if(unused->d_name[0] == '.'){
    return 0;
  }
  return 1;
}
//scandir implementation copied from:
//https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister-Mark-II.html

void getdir(FILE* out,const char* dirname, int level){
  //write directory name and new line to fd
  
  //fprintf(out, "%s\n", dirname);
  //get a list of files/dirs in current dir
  struct dirent **dirlist;
  int n;

  n = scandir (dirname, &dirlist, one, alphasort);
  //struct dirent **dirlist = *dirlist_ptr;
  //printf("dir %s, level: %d with n of %d\n",dirname, level, n);
  if(n > 0){
    //for each entry in list
    for(int i = 0; i < n; i++){
      //printf("At level %d, with n of %d, iteration %d\n", level, n, i);
      //write level * 4 spaces to fd
      fprintf(out, "%*s", level * 4, "");
      //write entry name and new line to fd
      fprintf(out, "%s\n", dirlist[i]->d_name);
      if(dirlist[i]->d_type == DT_DIR){

        char path[PATH_LEN];
        snprintf(path, PATH_LEN, "%s/%s", dirname, dirlist[i]->d_name);
        getdir(out, path, level + 1);
      }

      free(dirlist[i]);
      //if entry is dir
        //getdir(fd, entry, level + 1)
    }
  }
    free(dirlist);
}

/*
 * Print a hierachival directory view starting at DNAME to file descriptor FD 
 * as shown in an example below where DNAME == 'data.dir'
 *
 *   data.dir
 *       blbcbuvjjko
 *           lgvoz
 *               jfwbv
 *                   jqlbbb
 *                   yfgwpvax
 *           tcx
 *               jbjfwbv
 *                   demvlgq
 *                   us
 *               zss
 *                   jfwbv
 *                       ahfamnz
 *       vkhqmgwsgd
 *           agmugje
 *               surxeb
 *                   dyjxfseur
 *                   wy
 *           tcx
 */
void fileman_dir(const int fd, const char *dname) 
{
  FILE* out = fdopen(fd, "w");
  fprintf(out, "%s\n", dname);
  getdir(out, dname, 1);
  fclose(out);
}



void gettree(FILE* out,const char* dirname, int level, char* spacer){
  //write directory name and new line to fd
  
  //fprintf(out, "%s\n", dirname);
  //get a list of files/dirs in current dir
  struct dirent **dirlist;
  int n;

  n = scandir (dirname, &dirlist, one, alphasort);
  //struct dirent **dirlist = *dirlist_ptr;
  //printf("dir %s, level: %d with n of %d\n",dirname, level, n);
  if(n > 0){
    //for each entry in list
    for(int i = 0; i < n; i++){
      //printf("At level %d, with n of %d, iteration %d\n", level, n, i);
      //write level * 4 spaces to fd
      char spacer_new[PATH_LEN];
      if(i == n - 1){
        fprintf(out, "%s%s%s%s ", spacer, ELB, HOR, HOR);
        snprintf(spacer_new, PATH_LEN, "%s    ", spacer);
      }
      else{
        fprintf(out, "%s%s%s%s ", spacer, TEE, HOR, HOR);
        snprintf(spacer_new, PATH_LEN, "%s%s   ", spacer, VER);
      }
      
      //write entry name and new line to fd
      fprintf(out, "%s\n", dirlist[i]->d_name);
      if(dirlist[i]->d_type == DT_DIR){

        char path[PATH_LEN];
        snprintf(path, PATH_LEN, "%s/%s", dirname, dirlist[i]->d_name);
        gettree(out, path, level + 1, spacer_new);
      }

      free(dirlist[i]);
      //if entry is dir
        //getdir(fd, entry, level + 1)
    }
  }
    free(dirlist);
}

/*
 * Print a hierachival directory tree view starting at DNAME to file descriptor 
 * FD as shown in an example below where DNAME == 'data.dir'.
 * 
 * Use the extended ASCII box drawing characters TEE, HOR, VER, and ELB.
 *
 *   data.dir
 *   ├── blbcbuvjjko
 *   │   ├── lgvoz
 *   │   │   └── jfwbv
 *   │   │       ├── jqlbbb
 *   │   │       └── yfgwpvax
 *   │   └── tcx
 *   │       ├── jbjfwbv
 *   │       │   ├── demvlgq
 *   │       │   └── us
 *   │       └── zss
 *   │           └── jfwbv
 *   │               └── ahfamnz
 *   └── vkhqmgwsgd
 *       ├── agmugje
 *       │   └── surxeb
 *       │       ├── dyjxfseur
 *       │       └── wy
 *       └── tcx
 */
void fileman_tree(const int fd, const char *dname) 
{
  FILE* out = fdopen(fd, "w");
  fprintf(out, "%s\n", dname);
  char spacer[PATH_LEN] = "";
  gettree(out, dname, 1, spacer);
  fclose(out);
}
