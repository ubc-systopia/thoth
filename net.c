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

static void write_address_string(struct sock_entry_t *entry, char* buffer, bool delim) 
{
    int err = 0;
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    struct sockaddr *ad = (struct sockaddr*)(&entry->addr);

    if (ad->sa_family == AF_INET) {
        err = getnameinfo(ad, sizeof(struct sockaddr_in), host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        if (err == 0) {
            write_key_val_str(buffer, "host", host, true);
            write_key_val_str(buffer, "serv", serv, true); 
        } else {
            write_key_val_str(buffer, "host", "unknown", true);
            write_key_val_str(buffer, "serv", "unknown", true);
        }
        err = getnameinfo(ad, sizeof(struct sockaddr_in), host, NI_MAXHOST, serv, NI_MAXSERV, 0);
        if (err == 0) {
            write_key_val_str(buffer, "host_name", host, true);
            write_key_val_str(buffer, "serv_name", serv, delim);
        }
    } else if (ad->sa_family == AF_INET6) {
        err = getnameinfo(ad, sizeof(struct sockaddr_in6), host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        if (err == 0) {
            write_key_val_str(buffer, "host", host, true);
            write_key_val_str(buffer, "serv", serv, true); 
        } else {
            write_key_val_str(buffer, "host", "unknown", true);
            write_key_val_str(buffer, "serv", "unknown", true);
        }
        err = getnameinfo(ad, sizeof(struct sockaddr_in6), host, NI_MAXHOST, serv, NI_MAXSERV, 0);
        if (err == 0) {
            write_key_val_str(buffer, "host_name", host, true);
            write_key_val_str(buffer, "serv_name", serv, delim);
        }
    } else if (ad->sa_family == AF_UNIX) {
        err = getnameinfo(ad, sizeof(struct sockaddr), host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        if (err == 0) {
            write_key_val_str(buffer, "host", host, true);
            write_key_val_str(buffer, "serv", serv, delim);
        }
    } else {
        err = getnameinfo(ad, sizeof(struct sockaddr), host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        if (err == 0) {
            write_key_val_str(buffer, "host", host, true);
            write_key_val_str(buffer, "serv", serv, true); 
        } else {
            write_key_val_str(buffer, "host", "unknown", true);
            write_key_val_str(buffer, "serv", "unknown", true);
        }
        err = getnameinfo(ad, sizeof(struct sockaddr), host, NI_MAXHOST, serv, NI_MAXSERV, 0);
        if (err == 0) {
            write_key_val_str(buffer, "host_name", host, true);
            write_key_val_str(buffer, "serv_name", serv, delim);
        }
    }
}

static void write_family_string(struct sock_entry_t *entry, char* buffer, bool delim)
{
    char family[32];
	sprintf(family, "%u", entry->family);

    if (entry->family == AF_UNSPEC) {
        write_key_val_str(buffer, "family", "AF_UNSPEC", delim);
    } else if (entry->family == AF_UNIX) {
        write_key_val_str(buffer, "family", "AF_UNIX", delim);
    } else if (entry->family == AF_LOCAL) {
        write_key_val_str(buffer, "family", "AF_LOCAL", delim);
    } else if (entry->family == AF_INET) {
        write_key_val_str(buffer, "family", "AF_INET", delim);
    } else if (entry->family == AF_INET6) {
        write_key_val_str(buffer, "family", "AF_INET6", delim);
    } else if (entry->family == AF_PACKET) {
        write_key_val_str(buffer, "family", "AF_PACKET", delim);
    } else {
        write_key_val_str(buffer, "family", family, delim);
    }
}

static void get_socket_id(struct sock_entry_t *entry, char* buffer)
{
    char protocol[32];
    sprintf(protocol, "%u", entry->family);
    
    char address[NI_MAXHOST];
    char port[NI_MAXSERV];

    if (entry->family == AF_UNSPEC) {
        sprintf(address, "%u", 0);
        sprintf(port, "%u", 0);
    } else if (entry->op == RCV_SKB) {
        sprintf(address, "%u", entry->daddr);
        sprintf(port, "%u", entry->port);
    } else {
        getnameinfo(&entry->addr, sizeof(struct sockaddr_in), address, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    }

	strncat(buffer, protocol, MAX_BUFFER_LEN);
    strncat(buffer, "-", MAX_BUFFER_LEN);
    strncat(buffer, address, MAX_BUFFER_LEN);
    strncat(buffer, "-", MAX_BUFFER_LEN);
    strncat(buffer, port, MAX_BUFFER_LEN);
}