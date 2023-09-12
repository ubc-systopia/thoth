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

#ifndef __KERN_NET_H
#define __KERN_NET_H

#define ADDR_MAX PATH_MAX

// Address family macros from Linux kernel /include/linux/socket.h
#define AF_UNSPEC 0
#define AF_UNIX 1
#define AF_INET 2
#define AF_INET6 10

struct address_struct {
	uint8_t addr[PATH_MAX*2];
	size_t length;
};

#endif