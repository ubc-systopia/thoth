#ifndef __RECORD_H
#define __RECORD_H

enum operation {
  READ = 1,
  WRITE = 2,
  EXEC = 3,
};

struct entry_t {
  int pid;
  int utime;
  int gtime;
  unsigned int inode_inum;
  int inode_uid;
  int inode_guid;
  enum operation op;
};

#endif
