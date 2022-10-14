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

#ifndef __RECORD_H
#define __RECORD_H

#define PATH_DEPTH_MAX 5
#define PATH_NAME_MAX 32
#define TOTAL_PATH_MAX PATH_DEPTH_MAX*PATH_NAME_MAX

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
  int file_path_depth;
  char file_path[PATH_DEPTH_MAX][PATH_NAME_MAX];
  char file_name[TOTAL_PATH_MAX];
  enum operation op;
};

#endif
