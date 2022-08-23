#ifndef __RECORD_H
#define __RECORD_H

#define FILE_PATH_MAX 64

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
  char file_name[FILE_PATH_MAX];
  char parent_name[FILE_PATH_MAX];
  enum operation op;
};

#endif
