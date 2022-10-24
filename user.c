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
#include <dirent.h>

#include "common.h"
#include "record.h"
#include "spade.c"
#include "track.skel.h"

#define MAX_LINE_SIZE 100

static struct track *skel = NULL;
static int fd;
static int inode_track_index = 0;

static void sig_handler(int sig) {
  if (sig == SIGTERM) {
    syslog(LOG_INFO, "thoth: Received termination signal...");
    track__destroy(skel);
    exit(0);
  }
}

// Start with adding just one id into the map
int add_inode(struct track *skel, uint32_t index, uint64_t value) {
  int map_fd;
  uint64_t id;
  if (inode_track_index > INODE_MAX_ENTRY) {
    printf("max tracking directories reached, failed to add directory\r\n");
    return -1;
  }
  map_fd = bpf_object__find_map_fd_by_name(skel->obj, "inode_map");
  id = value;
  bpf_map_update_elem(map_fd, &inode_track_index, &id, BPF_ANY);
  inode_track_index++;
  return 0;
}

// TODO
int remove_inode(struct track *skel, uint32_t index, uint64_t value) {
  return 0;
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

static int cli_process_msg(struct op_msg *msg, struct err_msg *err) {
  DIR* dir;

  if (msg->op == ADD_DIR) {
    dir = opendir(msg->arg[0]);
    if (dir == NULL) {
      err->err = ERR_OK;
      snprintf(err->msg, MSG_LEN, "error! cannot find directory: %s", msg->arg[0]);
      return 0;
    }

    struct dirent *dir_entry = readdir(dir);
    if (dir_entry == NULL) {
      err->err = ERR_OK;
      snprintf(err->msg, MSG_LEN, "error adding directory to tracked directories");
    }
    // TODO: get response from add
    add_inode(skel, 0, dir_entry->d_ino);
  }

  if (msg->op == RM_DIR) {
    dir = opendir(msg->arg[0]);
    if (dir == NULL) {
      err->err = ERR_OK;
      snprintf(err->msg, MSG_LEN, "error! cannot find directory: %s", msg->arg[0]);
      return 0;
    }

    struct dirent *dir_entry = readdir(dir);
    if (dir_entry == NULL) {
      err->err = ERR_OK;
      snprintf(err->msg, MSG_LEN, "error adding directory to tracked directories");
    }
    // TODO: get response from remove
    remove_inode(skel, 0, dir_entry->d_ino);
  }
  return 0;
}

void* cli_server(void* data) {
  int sockfd; 
  socklen_t client_len;
  struct sockaddr_in server_addr, client_addr;

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
    close(sockfd);
    exit(-1);
  }

  listen(sockfd, 5);
  client_len = sizeof(client_addr);
  int newsockfd;

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);

    if (newsockfd < 0) {
      perror("error on socket accept");
    }

    struct op_msg r_msg;
 
    int n = read(newsockfd, &r_msg, sizeof(r_msg));
    if (n < 0) 
      perror("error reading from socket");
    
    printf("%s\r\n", r_msg.arg[0]);
 
    // TODO: remove and replace with real error msg   
    struct err_msg e_msg = {
      .err = ERR_OK,
      .msg = "Sucess!"
    };

    cli_process_msg(&r_msg, &e_msg);
    
    n = write(newsockfd, &e_msg, sizeof(e_msg));
 
    if (n < 0)
      perror("error writing to socket");

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
