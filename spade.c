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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>

#include "user/json.h"
#include "net.c"
#include "shared/record.h"

char date[DATE_LEN];
pthread_rwlock_t date_lock;
int edge_id = 0;

static void update_datetime()
{
	struct timeval curr_time;

	gettimeofday(&curr_time, NULL);
	int millisec = curr_time.tv_usec / 1000;

	char buffer[80];

	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curr_time.tv_sec));

	pthread_rwlock_wrlock(&date_lock);
	sprintf(date, "%s:%03d", buffer, millisec);
	pthread_rwlock_unlock(&date_lock);
}

static void write_datetime(char *buffer, bool delim)
{
	update_datetime();

	strncat(buffer, "\"datetime\":", MAX_BUFFER_LEN);
	strncat(buffer, "\"", MAX_BUFFER_LEN);
	pthread_rwlock_wrlock(&date_lock);
	strncat(buffer, date, MAX_BUFFER_LEN);
	pthread_rwlock_unlock(&date_lock);
	strncat(buffer, "\"", MAX_BUFFER_LEN);
	if (delim)
		strncat(buffer, ",", MAX_BUFFER_LEN);
}

void add_name(char *buffer, char *name)
{
	strncat(buffer, "\"", MAX_BUFFER_LEN);
	strncat(buffer, name, MAX_BUFFER_LEN);
	strncat(buffer, "\":", MAX_BUFFER_LEN);
}

void init_buffer(char *buffer)
{
	buffer[0] = '\0';
}

void write_start_json(char *buffer)
{
	strncat(buffer, "{", MAX_BUFFER_LEN);
}

void write_end_json(char *buffer)
{
	strncat(buffer, "}\n", MAX_BUFFER_LEN);
}

void write_start_annotations(char *buffer)
{
	strncat(buffer, "\"annotations\":{", MAX_BUFFER_LEN);
}

void write_end_annotations(char *buffer)
{
	strncat(buffer, "}", MAX_BUFFER_LEN);
}

void spade_write_node_proc(int fd, struct entry_t *entry)
{
	char buf[MAX_BUFFER_LEN];

	buf[0] = '\0';

	char pid[32];

	sprintf(pid, "%u", entry->pid);
	char inode[32];

	sprintf(inode, "%u", entry->inode_inum);

	// start json
	write_start_json(buf);

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

	write_key_val_str(buf, "pid", pid, false);

	// end json
	strncat(buf, "}}\n", MAX_BUFFER_LEN);
	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_node_file(int fd, struct entry_t *entry, char *buffer)
{
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
	strncat(buf, "\",", MAX_BUFFER_LEN);

	strncat(buf, "\"path\":", MAX_BUFFER_LEN);
	strncat(buf, "\"", MAX_BUFFER_LEN);
	strncat(buf, buffer, MAX_BUFFER_LEN);
	strncat(buf, "\"", MAX_BUFFER_LEN);

	// end json
	strncat(buf, "}}\n", MAX_BUFFER_LEN);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_node_socket(int fd, struct sock_entry_t *entry)
{
	char buf[MAX_BUFFER_LEN];

	init_buffer(buf);

	write_start_json(buf);

	// TODO: make helper fn
	strncat(buf, "\"type\":", MAX_BUFFER_LEN);
	strncat(buf, "\"Entity\",", MAX_BUFFER_LEN);

	strncat(buf, "\"id\":\"", MAX_BUFFER_LEN);
	get_socket_id(entry, buf);
	strncat(buf, "\",", MAX_BUFFER_LEN);

	write_start_annotations(buf);

	write_address_string(entry, buf, true);

	write_family_string(entry, buf, false);

	write_end_annotations(buf);
	write_end_json(buf);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_edge_socket(int fd, struct sock_entry_t *entry)
{
	char buf[MAX_BUFFER_LEN];

	init_buffer(buf);

	write_start_json(buf);

	char pid[32];

	sprintf(pid, "%u", entry->pid);

	char operation[32];

	strncat(buf, "\"type\":", MAX_BUFFER_LEN);
	strncat(buf, "\"Used\",", MAX_BUFFER_LEN);

	// to
	strncat(buf, "\"to\":\"", MAX_BUFFER_LEN);
	get_socket_id(entry, buf);
	strncat(buf, "\",", MAX_BUFFER_LEN);

	// from
	strncat(buf, "\"from\":", MAX_BUFFER_LEN);
	strncat(buf, "\"", MAX_BUFFER_LEN);
	strncat(buf, pid, MAX_BUFFER_LEN);
	strncat(buf, "\",", MAX_BUFFER_LEN);

	if (entry->op == CONNECT)
		sprintf(operation, "%s", "socket_connect");
	if (entry->op == RCV_SKB)
		sprintf(operation, "%s", "socket_rcv_skb");

	write_start_annotations(buf);

	strncat(buf, "\"operation\":", MAX_BUFFER_LEN);
	strncat(buf, "\"", MAX_BUFFER_LEN);
	strncat(buf, operation, MAX_BUFFER_LEN);
	strncat(buf, "\",", MAX_BUFFER_LEN);

	write_datetime(buf, false);

	write_end_annotations(buf);
	write_end_json(buf);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_node_command(int fd, struct command_entry_t *entry, int id)
{
	char buf[MAX_BUFFER_LEN];

	char command_id[32];

	sprintf(command_id, "c%u", id);

	char args_count[32];

	sprintf(args_count, "%u", entry->args_count);

	init_buffer(buf);

	write_start_json(buf);

	// type
	write_key_val_str(buf, "type", "Entity", true);
	// id
	write_key_val_str(buf, "id", command_id, true);

	write_start_annotations(buf);
	write_key_val_str(buf, "command", entry->args[0], true);
	for (int i = 1; i < entry->args_count; i++) {
		char arg[32];
		sprintf(arg, "arg[%u]", i);
		write_key_val_str(buf, arg, entry->args[i], true);
	}
	write_key_val_str(buf, "args_count", args_count, false);

	write_end_annotations(buf);
	write_end_json(buf);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_edge_command(int fd, struct command_entry_t *entry, int id)
{
	char buf[MAX_BUFFER_LEN];

	init_buffer(buf);

	write_start_json(buf);

	char pid[32];

	sprintf(pid, "%u", entry->pid);
	char command_id[32];

	sprintf(command_id, "c%u", id);
	char operation[32];

	sprintf(operation, "%s", "executed_command");

	strncat(buf, "\"type\":", MAX_BUFFER_LEN);
	strncat(buf, "\"Used\",", MAX_BUFFER_LEN);

	// to
	write_key_val_str(buf, "to", command_id, true);

	// from
	write_key_val_str(buf, "from", pid, true);

	write_start_annotations(buf);

	write_key_val_str(buf, "operation", operation, true);
	write_datetime(buf, false);

	write_end_annotations(buf);
	write_end_json(buf);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}

void spade_write_edge(int fd, struct entry_t *entry)
{
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

	if (entry->op == READ)
		sprintf(operation, "%s", "read");
	else if (entry->op == WRITE)
		sprintf(operation, "%s", "write");
	else if (entry->op == EXEC)
		sprintf(operation, "%s", "execute");
	else if (entry->op == MMAP)
		sprintf(operation, "%s", "mmap");

	// start json
	write_start_json(buf);

	// type
	strncat(buf, "\"type\":", MAX_BUFFER_LEN);
	if (entry->op == READ)
		strncat(buf, "\"Used\",", MAX_BUFFER_LEN);
	else if (entry->op == WRITE)
		strncat(buf, "\"WasGeneratedBy\",", MAX_BUFFER_LEN);
	else if (entry->op == EXEC || entry->op == MMAP)
		strncat(buf, "\"Used\",", MAX_BUFFER_LEN);

	if (entry->op == READ || entry->op == EXEC || entry->op == MMAP) {
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
	} else if (entry->op == WRITE) {
		// to
		strncat(buf, "\"to\":", MAX_BUFFER_LEN);
		strncat(buf, "\"", MAX_BUFFER_LEN);
		strncat(buf, pid, MAX_BUFFER_LEN);
		strncat(buf, "\",", MAX_BUFFER_LEN);

		// from
		strncat(buf, "\"from\":", MAX_BUFFER_LEN);
		strncat(buf, "\"", MAX_BUFFER_LEN);
		strncat(buf, inode, MAX_BUFFER_LEN);
		strncat(buf, "\",", MAX_BUFFER_LEN);
	}

	// annotations
	write_start_annotations(buf);

	write_key_val_str(buf, "operation", operation, true);
	write_datetime(buf, false);

	write_end_annotations(buf);
	write_end_json(buf);

	write(fd, buf, strnlen(buf, MAX_BUFFER_LEN));
}
