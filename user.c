/*
 * 
 * Author: Nichole Boufford <ncbouf@cs.ubc.ca>
 *
 * Copyright (C) 2022 University of British Columbia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <bpf/bpf.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>

#include "common.h"
#include "record.h"
#include "spade.c"
#include "track.skel.h"

#define MAX_LINE_SIZE 100

static struct track *skel = NULL;
static int fd;
static int tracking_inode = 6148;

static void sig_handler(int sig) {
  if (sig == SIGTERM) {
    syslog(LOG_INFO, "thoth: Received termination signal...");
    track__destroy(skel);
    exit(0);
  }
}

// Start with adding just one id into the map
static void add_inode(struct track *skel, uint32_t index, uint64_t value) {
  int map_fd;
  uint64_t id;
  map_fd = bpf_object__find_map_fd_by_name(skel->obj, "inode_map");
  id = value;
  bpf_map_update_elem(map_fd, &index, &id, BPF_ANY);
}

void write_to_file(struct entry_t *entry, char* buffer) 
{
  // should lock here
  spade_write_node_file(fd, entry, buffer);
  spade_write_node_proc(fd, entry);
  spade_write_edge(fd, entry);
  // should unlock here
}

// this is a temporary fix for resolving the file path
void process_file_path(struct entry_t *entry, char* buffer) {
  int path_len = 0;
  
  // sanity check
  if (entry->file_path_depth <= 0 || entry->file_path_depth > PATH_DEPTH_MAX) 
    return; 
  
  for (int i = entry->file_path_depth - 1; i >= 0; i--) {
    int len = 0;
    if (i == entry->file_path_depth - 1) { 
      len = sprintf(buffer, "/");
      if (len > 0)
        path_len += len;
    }
    if (i == 0) {
      len = sprintf(buffer+path_len, "%s", entry->file_path[i]);
    } else {  
      len = sprintf(buffer+path_len, "%s/", entry->file_path[i]);
    }

    if (len > 0)
      path_len += len;
  }
}


int buf_process_entry(void *ctx, void *data, size_t len)
{
  struct entry_t *read_entry = (struct entry_t *)data; 
  char path_buffer[TOTAL_PATH_MAX];
  process_file_path(read_entry, (char *)&path_buffer);
  write_to_file(read_entry, (char *)&path_buffer);
  return 0;
}

static int cli_process_buf(char *buffer) {
  printf("processing buffer: %s", buffer);
  return 0;
}

void* cli_server(void* data) {
  int sockfd; 
  socklen_t client_len;
  struct sockaddr_in server_addr, client_addr;

  char buffer[SOCK_BUF_MAX];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Error opening socket for cli");
    exit(-1);
  }

  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(CLI_PORT);

  if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("Error binding socket for cli");
  }

  listen(sockfd, 5);
  client_len = sizeof(client_addr);
  int newsockfd;

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);

    if (newsockfd < 0) {
      perror("error on socket accept");
    }

    bzero(buffer, SOCK_BUF_MAX);
    int n = read(newsockfd, buffer, SOCK_BUF_MAX);
    if (n < 0) {
      perror("error reading from socket");
    }
    printf("%s\r\n", buffer);
    cli_process_buf((char *)&buffer);

    close(newsockfd);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  struct ring_buffer *ringbuf = NULL;
  int err, map_fd;
  pthread_t cli_thread_id;

  syslog(LOG_INFO, "thothd: Registering signal handler...");
  signal(SIGTERM, sig_handler);

  if (argc == 2) {
    tracking_inode = (uint32_t)atoi(argv[1]);
  } 

  syslog(LOG_INFO, "thothd: Starting CLI server...");
  int rc = pthread_create(&cli_thread_id, NULL, cli_server, NULL);
  if (rc != 0) {
    syslog(LOG_ERR, "thothd: error starting cli thread");
  }

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
