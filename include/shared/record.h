/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Author: Nichole Boufford <ncbouf@cs.ubc.ca>
 *
 * Copyright (C) 2022 University of British Columbia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef __RECORD_H
#define __RECORD_H

#define PATH_DEPTH_MAX 8
#define PATH_NAME_MAX 48
#define TOTAL_PATH_MAX PATH_DEPTH_MAX *PATH_NAME_MAX

#define ARGS_MAX 8
#define ARGS_SIZE_MAX 48

#define DATE_LEN 256

extern char date[DATE_LEN];

#define ENTRY_TYPE_FILE 0
#define ENTRY_TYPE_SOCK 1
#define ENTRY_TYPE_COMMAND 2

enum file_op {
	READ    = 1,
	WRITE   = 2,
	EXEC    = 3,
	MMAP    = 4,
	EXECVE  = 5
};

enum sock_op {
	CONNECT = 1,
	ALLOC   = 2,
	FREE    = 3,
	RCV_SKB = 4
};

struct entry_t {
	uint32_t flag;
	int pid;
	int utime;
	int gtime;
	unsigned int inode_inum;
	int inode_uid;
	int inode_guid;
	int proc_uid;
	int proc_guid;
	int file_path_depth;
	char file_path[PATH_DEPTH_MAX][PATH_NAME_MAX];
	enum file_op op;
};

struct sock_entry_t {
	uint32_t flag;
	int pid;
	struct sockaddr addr;
	int family;
	uint32_t saddr;
	uint32_t daddr;
	uint32_t port;
	enum sock_op op;
};

struct command_entry_t {
	uint32_t flag;
	int pid;
	int utime;
	unsigned int inode_inum;
	int args_count;
	int args_size;
	int proc_uid;
	char args[ARGS_MAX][ARGS_SIZE_MAX];
	enum file_op op;
};

union prov_entry {
	struct entry_t file_entry;
	struct sock_entry_t sock_entry;
	struct command_entry_t command_entry;
};

#endif
