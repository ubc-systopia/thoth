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

#define PATH_DEPTH_MAX 12
#define PATH_NAME_MAX 32
#define TOTAL_PATH_MAX PATH_DEPTH_MAX *PATH_NAME_MAX

#define DATE_LEN 256

extern char date[DATE_LEN];

enum operation {
	READ    = 1,
	WRITE   = 2,
	EXEC    = 3,
};

struct entry_t {
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
	enum operation op;
};

#endif
