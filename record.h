#ifndef __RECORD_H
#define __RECORD_H

#define PATH_MAX 64

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
  char file_name[PATH_MAX];
  char parent_name[PATH_MAX];
  enum operation op;
};

#endif
