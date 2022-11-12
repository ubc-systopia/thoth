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
