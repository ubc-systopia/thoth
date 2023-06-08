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

#ifndef __COMMON_H
#define __COMMON_H

#define SOCK_BUF_MAX 256
#define CLI_PORT 5001

#define HOSTNAME_MAX 64

#define NUM_ARGS 5
#define ARG_LEN 50
#define MSG_LEN 150

#define INODE_MAX_ENTRY 256

enum cli_op {
	ADD_DIR = 0,
	RM_DIR  = 1,
};

enum cli_err {
	ERR_OK = 0,
};

struct op_msg {
	enum cli_op op;
	char arg[NUM_ARGS][ARG_LEN];
};

struct err_msg {
	enum cli_err err;
	char msg[MSG_LEN];
};


#endif
