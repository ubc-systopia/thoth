/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Author: Nichole Boufford
 */

// from: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/tools/testing/selftests/bpf/prog_tests/test_lsm.c

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "record.h"
#include "track.skel.h"

#define MAX_LINE_SIZE 100

static int fd;

static void write_to_file(struct entry_t *entry) 
{
  int rc;
  char buf[MAX_LINE_SIZE] = { 0 };

  int len = 0;

  if (entry->op == READ) {
    len += sprintf(buf + len, "READ ");
  } else {
    len += sprintf(buf + len, "WRITE ");  
  }
  len += sprintf(buf + len, "%u, ", entry->pid);
  len += sprintf(buf + len, "%u, ", entry->inode_uid);
  len += sprintf(buf + len, "%u, ", entry->inode_guid);
  len += sprintf(buf + len, "%u, ", entry->inode_inum);
  
  // should lock here
  rc = write(fd, buf, len);
  if (rc < 0) {
    fprintf(stderr, "error writing to log\r\n");
  }

  rc = write(fd, "\n", 1);

  if (rc < 0) {
    fprintf(stderr, "error writing to log\r\n");
  } 
  // should unlock here
}

static int buf_process_entry(void *ctx, void *data, size_t len)
{
  struct entry_t *read_entry = (struct entry_t *)data; 
  printf("pid: %d\r\n", read_entry->pid);
  printf("inode num: %d\r\n", read_entry->inode_inum);

  // save to log file
  write_to_file(read_entry);
  return 0;
}

int main(void)
{
  struct track *skel = NULL;
  struct ring_buffer *ringbuf = NULL;
  int err, map_fd;

  printf("starting...\r\n");

  skel = track__open_and_load();
  if (!skel) {
    printf("Failed to load bpf skeleton");
    goto close_prog;
  }

  err = track__attach(skel);

  FILE* f;
  char s;
  
  fd = open("track.log", O_RDWR);

  if (fd < 0) {
    printf("error opening file\r\n");
  }
  
  /* Locate ring buffer */
  map_fd = bpf_object__find_map_fd_by_name(skel->obj, "ringbuf");
  if (map_fd < 0) {
    printf("Failed to find ring buffer map object");
    goto close_prog;
  }
 
  ringbuf = ring_buffer__new(map_fd, buf_process_entry, NULL, NULL);
  
  f = fopen("test.txt", "r");
  
  while (ring_buffer__poll(ringbuf, -1) >= 0) { 
    while ((s = fgetc(f)) != EOF) {
      printf("%c", s);
    }
    fseek(f, 0, SEEK_SET);    
    sleep(1);  
  }
  printf("closing file\r\n");
  fclose(f);
close_prog:
  close(fd);
  track__destroy(skel);
}
