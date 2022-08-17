/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"

#define MAX_BUFFER_LEN 100 

int edge_id = 0;

void add_name(char* buffer, char* name) {
  strncat(buffer, "\"", MAX_BUFFER_LEN);
  strncat(buffer, name, MAX_BUFFER_LEN);
  strncat(buffer, "\":", MAX_BUFFER_LEN);
} 

void spade_write_node_proc(int fd, struct entry_t *entry) {
  char buf[MAX_BUFFER_LEN];
  buf[0]='\0';
  
  char pid[32];
  sprintf(pid, "%u", entry->pid);
  char inode[32];
  sprintf(inode, "%u", entry->inode_inum);

  // start json
  strncat(buf, "{", MAX_BUFFER_LEN);
 
  // type 
  strncat(buf, "\"type\":", MAX_BUFFER_LEN);
  strncat(buf, "\"Activity\",", MAX_BUFFER_LEN);

  // id
  strncat(buf, "\"id\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, pid, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
  
  // annotations
  strncat(buf, "\"annotations\":{", MAX_BUFFER_LEN);  

  strncat(buf, "\"pid\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, pid, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
 
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, inode, MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  // end json
  strncat(buf, "}}\n", MAX_BUFFER_LEN);
  write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_node_file(int fd, struct entry_t *entry) {
  char buf[MAX_BUFFER_LEN];
  buf[0] = '\0';
  
  char inode[32];
  sprintf(inode, "%u", entry->inode_inum);
  char uid[32];
  sprintf(uid, "%u", entry->inode_uid);
  // start json
  strncat(buf, "{", MAX_BUFFER_LEN);

  // type
  strncat(buf, "\"type\":", MAX_BUFFER_LEN);
  strncat(buf, "\"Entity\",", MAX_BUFFER_LEN);

  // id
  strncat(buf, "\"id\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, inode, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
  
  // annotations
  strncat(buf, "\"annotations\":{", MAX_BUFFER_LEN);  

  strncat(buf, "\"inode_inum\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, inode, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
 
  strncat(buf, "\"uid\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, uid, MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  
  // end json
  strncat(buf, "}}\n", MAX_BUFFER_LEN);

  write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_edge(int fd, struct entry_t *entry) {
  char buf[MAX_BUFFER_LEN];
  buf[0] = '\0';
  
  char id[32];
  sprintf(id, "%u", edge_id++);
  char pid[32];
  sprintf(pid, "%u", entry->pid);
  char inode[32];
  sprintf(inode, "%u", entry->inode_inum);
  char utime[32];
  sprintf(utime, "%u", entry->utime);

  char operation[32];
  if (entry->op == READ) {
    sprintf(operation, "%s", "read");
  } else if (entry->op == WRITE) {
    sprintf(operation, "%s", "write");
  } else if (entry->op == EXEC) {
    sprintf(operation, "%s", "execute");
  }

  // start json
  strncat(buf, "{", MAX_BUFFER_LEN);

  // type
  strncat(buf, "\"type\":", MAX_BUFFER_LEN);
  strncat(buf, "\"Used\",", MAX_BUFFER_LEN);

  // to
  strncat(buf, "\"to\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, inode, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
 
  // from
  strncat(buf, "\"from\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, pid, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
  
  // annotations
  strncat(buf, "\"annotations\":{", MAX_BUFFER_LEN);  

  strncat(buf, "\"operation\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, operation, MAX_BUFFER_LEN);
  strncat(buf, "\",", MAX_BUFFER_LEN);
 
  strncat(buf, "\"utime\":", MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  strncat(buf, utime, MAX_BUFFER_LEN);
  strncat(buf, "\"", MAX_BUFFER_LEN);
  
  // end json
  strncat(buf, "}}\n", MAX_BUFFER_LEN);

  write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));

}
