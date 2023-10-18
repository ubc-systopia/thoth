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

#ifndef __USER_JSON_H
#define __USER_JSON_H

#define MAX_BUFFER_LEN 1024

void write_key_val_str(char* buffer, char* key, char* val, bool delim) 
{
	strncat(buffer, "\"", MAX_BUFFER_LEN);
	strncat(buffer, key, MAX_BUFFER_LEN);
	strncat(buffer, "\":", MAX_BUFFER_LEN);
    strncat(buffer, "\"", MAX_BUFFER_LEN);
    strncat(buffer, val, MAX_BUFFER_LEN);
    strncat(buffer, "\"", MAX_BUFFER_LEN);
    if (delim) {
		strncat(buffer, ",", MAX_BUFFER_LEN);
    }
}

void write_key_val_int(char* buffer, char* key, int val, bool delim) 
{
    char num[32];
	sprintf(num, "%u", val);

	strncat(buffer, "\"", MAX_BUFFER_LEN);
	strncat(buffer, key, MAX_BUFFER_LEN);
	strncat(buffer, "\":", MAX_BUFFER_LEN);
	strncat(buffer, "\"", MAX_BUFFER_LEN);
    strncat(buffer, num, MAX_BUFFER_LEN);
    strncat(buffer, "\"", MAX_BUFFER_LEN);

    if (delim) {
		strncat(buffer, ",", MAX_BUFFER_LEN);
    }
}

#endif