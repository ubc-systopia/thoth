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
#include <bpf/bpf.h>

#include "spade.c"
#include "record.h"
#include "track.skel.h"

#define MAX_LINE_SIZE 100

static int fd;

static int tracking_inode = 5567;

// Start with adding just one id into the map
static void add_inode(struct track *skel, uint32_t index, uint64_t value) {
  int map_fd;
  uint64_t id;
  map_fd = bpf_object__find_map_fd_by_name(skel->obj, "inode_map");
  id = value;
  bpf_map_update_elem(map_fd, &index, &id, BPF_ANY);
}

static void write_to_file(struct entry_t *entry) 
{
  // should lock here
  spade_write_node_file(fd, entry);
  spade_write_node_proc(fd, entry);
  spade_write_edge(fd, entry);
  // should unlock here
}

static int buf_process_entry(void *ctx, void *data, size_t len)
{
  struct entry_t *read_entry = (struct entry_t *)data; 
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
  
  // add test inode number
  add_inode(skel, 0, (uint64_t)tracking_inode);

  err = track__attach(skel);
  if (err != 0) {
    printf("Error attaching skeleton\r\n");
  }

  fd = open("/tmp/track.json", O_RDWR);
  if (fd < 0) {
    printf("error opening file\r\n");
  }
  
  // Locate ring buffer
  map_fd = bpf_object__find_map_fd_by_name(skel->obj, "ringbuf");
  if (map_fd < 0) {
    printf("Failed to find ring buffer map object");
    goto close_prog;
  }
 
  ringbuf = ring_buffer__new(map_fd, buf_process_entry, NULL, NULL);
  
  while (ring_buffer__poll(ringbuf, -1) >= 0) { 
    // collect prov in callback
  }
close_prog:
  close(fd);
  track__destroy(skel);
}
