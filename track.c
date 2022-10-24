/*
 *
 * Author: Nichole Boufford <ncbouf@cs.ubc.ca>
 *
 * Copyright (C) 2022 University of British Columbia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 */

#include "vmlinux.h"
#include "record.h"
#include "common.h"

#define __kernel_size_t
#define __kernel_fsid_t
#define __kernel_fd_set
#define statx_timestamp
#define statx
#include <linux/stat.h>
#undef __kernel_size_t
#undef __kernel_fsid_t
#undef __kernel_fd_set
#undef statx_timestamp
#undef statx

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char _license[] SEC("license") = "GPL";

struct inode_elem {
  uint32_t id;
  uint8_t flag; 
  struct bpf_spin_lock lock;
};

// MAPS

// ring buffer
struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1 << 12);
} ringbuf SEC(".maps");

// list of tracking dir inodes
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, uint32_t);
	__type(value, struct inode_elem);
	__uint(max_entries, INODE_MAX_ENTRY);
} inode_map SEC(".maps");

// cache for inodes in tracking dir
struct {
  __uint(type, BPF_MAP_TYPE_INODE_STORAGE);
  __uint(map_flags, BPF_F_NO_PREALLOC);
  __type(key, uint32_t);
  __type(value, struct inode_elem);
} inode_storage_map SEC(".maps");

// FILE MACROS

#define MAY_EXEC		0x00000001
#define MAY_WRITE		0x00000002
#define MAY_READ		0x00000004
#define MAY_APPEND		0x00000008
#define MAY_ACCESS		0x00000010
#define MAY_OPEN		0x00000020
#define MAY_CHDIR		0x00000040

#define IS_PRIVATE(inode)	((inode)->i_flags & S_PRIVATE)

#define is_inode_dir(inode)             S_ISDIR(inode->i_mode)
#define is_inode_socket(inode)          S_ISSOCK(inode->i_mode)
#define is_inode_file(inode)            S_ISREG(inode->i_mode)

#define FILE__EXECUTE           0x00000001UL
#define FILE__READ              0x00000002UL
#define FILE__APPEND            0x00000004UL
#define FILE__WRITE             0x00000008UL
#define DIR__SEARCH             0x00000010UL
#define DIR__WRITE              0x00000020UL
#define DIR__READ               0x00000040UL

#define inode_check_flag(inode, bit)  ((inode->flag & (1 << bit)) == (1 << bit))
#define inode_set_flag(inode, bit)    (inode->flag |= 1 << bit)
#define inode_clear_flag(inode, bit)  (inode->flag &= ~(1 << bit))

#define INIT_BIT 0
#define inode_set_init(inode)         inode_set_flag(inode, INIT_BIT)
#define inode_is_init(inode)          inode_check_flag(inode, INIT_BIT)

#define TRACKED_BIT 1
#define inode_set_tracked(inode)      inode_set_flag(inode, TRACKED_BIT)
#define inode_is_tracked(inode)       inode_check_flag(inode, TRACKED_BIT)


// FUNCTIONS

// from profbpf project
static inline uint32_t file_mask_to_perms(int mode, unsigned int mask)
{
  uint32_t av = 0;

  if (!S_ISDIR(mode)) {
    if (mask & MAY_EXEC) {
      av |= FILE__EXECUTE;
    }
    if (mask & MAY_READ) {
      av |= FILE__READ;
    }
    if (mask & MAY_APPEND) {
      av |= FILE__APPEND;
    }
    else if (mask & MAY_WRITE) {
      av |= FILE__WRITE;
    } else {
      if (mask & MAY_EXEC) {
	      av |= DIR__SEARCH;
      }
      if (mask & MAY_WRITE) {
	      av |= DIR__WRITE;
      }
      if (mask & MAY_READ) {
        av |= DIR__READ;
      }
    }
  }
  return av;
}

// Check if inode is in the tracking inode map
static int inode_in_map(struct inode *inode) {

  for (uint32_t i = 0; i < INODE_MAX_ENTRY; i++) {
    uint32_t key = i;
    struct inode_elem *val = bpf_map_lookup_elem(&inode_map, &key);
    if (val == NULL) {
      // log something about map not set-up
      return 0;
    }
    if (val->id == 0) { // reached the end of the list
      return 0;
    }
    if (val->id == inode->i_ino) {
      return 1;
    }
  }

  return 0;
}

// check if inode is in a directory that we track
static int in_tracking_dir(struct dentry *entry) {
  struct dentry *d = entry;
  int path_len = 0;
  while (path_len < PATH_DEPTH_MAX && d != NULL) {
    if (inode_in_map(d->d_inode) == 1) {
       return 1;
    }
    if (d == d->d_parent) {
      return 0;
    }
    d = d->d_parent;
    path_len++;
  }
  return 0;
}

static int inode_initialized(struct inode_elem *inode) {
  // get the lock, check the init flag and then set the flag
  bpf_spin_lock(&inode->lock);
  int is_init = inode_is_init(inode);
  if (!is_init) {
    inode_set_init(inode);
  }
  bpf_spin_unlock(&inode->lock);
  return is_init;
}

static int get_inode_tracking(struct inode_elem *inode) {
  bpf_spin_lock(&inode->lock);
  int is_tracking = inode_is_tracked(inode);
  bpf_spin_unlock(&inode->lock);
  return is_tracking;
}

static int set_inode_tracking(struct inode_elem *inode) {
  bpf_spin_lock(&inode->lock);
  inode_set_tracked(inode);
  bpf_spin_unlock(&inode->lock);
  return 0;
}

#define MAX_NAME_LEN 16

int read_path_name(struct entry_t *entry, struct dentry* dentry) {
  struct dentry *d = dentry;
  
  for (int i = 0; i < PATH_DEPTH_MAX; i++) {
    if (d == d->d_parent || d == NULL) {
      break;
    }

    int len = bpf_probe_read_kernel_str(&entry->file_path[i], MAX_NAME_LEN, d->d_iname);
    bpf_printk("read %u bytes", len); 
    bpf_printk("%s", entry->file_path[i]);
    entry->file_path_depth++;
    d = d->d_parent;
  }
    
  return 0;
}

SEC("lsm/file_permission")
int BPF_PROG(file_permission, struct file *file, int mask)
{
  struct task_struct *current_task;
  uint32_t perms;
  struct inode_elem *inode;

  // only check files not directories
  if (is_inode_dir(file->f_inode)) {
    return 0;
  }

  // check inode cache
  inode = bpf_inode_storage_get(&inode_storage_map, file->f_inode, 0, BPF_LOCAL_STORAGE_GET_F_CREATE); 

  if (inode == NULL) {
    bpf_printk("something has gone wrong in inode storage");
    return 0;
  }

  // initialize node if not already, if not init then we walk file path to check tracking
  if (inode_initialized(inode) == 0) {
    // walk directory and check if its in there
    // inode not initialized (not cached)
    if (in_tracking_dir(file->f_path.dentry) == 1) {
      set_inode_tracking(inode);
      // bpf_printk("setting tracking bit to 1");
    }
  }

  if (get_inode_tracking(inode) == 0) {
    return 0; // not tracking
  }

  current_task = (struct task_struct *)bpf_get_current_task_btf();

  perms = file_mask_to_perms((file->f_inode)->i_mode, mask);

  if ((perms & (FILE__READ)) != 0) {
    struct entry_t new_entry = {
      .pid = current_task->pid,
      .utime = current_task->utime,
      .gtime = current_task->gtime,
      .inode_inum = file->f_inode->i_ino,
      .inode_uid = file->f_inode->i_uid.val,
      .inode_guid = file->f_inode->i_gid.val,
      .op = READ,
    };
    read_path_name(&new_entry, file->f_path.dentry);
    //bpf_probe_read_kernel_str(new_entry.file_name, MAX_NAME_LEN, file->f_path.dentry->d_iname);
    bpf_ringbuf_output(&ringbuf, &new_entry, sizeof(struct entry_t), 0);
  }

  if ((perms & (FILE__WRITE)) != 0) {
    struct entry_t new_entry = {
      .pid = current_task->pid,
      .utime = current_task->utime,
      .gtime = current_task->gtime,
      .inode_inum = file->f_inode->i_ino,
      .inode_uid = file->f_inode->i_uid.val,
      .inode_guid = file->f_inode->i_gid.val,
      .op = WRITE,
    };
    read_path_name(&new_entry, file->f_path.dentry);
    //bpf_probe_read_kernel_str(new_entry.file_name, MAX_NAME_LEN, file->f_path.dentry->d_iname);
    bpf_ringbuf_output(&ringbuf, &new_entry, sizeof(struct entry_t), 0);
  }

  if ((perms & (FILE__EXECUTE)) != 0) {
    struct entry_t new_entry = {
      .pid = current_task->pid,
      .utime = current_task->utime,
      .gtime = current_task->gtime,
      .inode_inum = file->f_inode->i_ino,
      .inode_uid = file->f_inode->i_uid.val,
      .inode_guid = file->f_inode->i_gid.val,
      .op = EXEC,
    };
    read_path_name(&new_entry, file->f_path.dentry);
    //bpf_probe_read_kernel_str(new_entry.file_name, MAX_NAME_LEN, file->f_path.dentry->d_iname);
    bpf_ringbuf_output(&ringbuf, &new_entry, sizeof(struct entry_t), 0);
  }

  return 0;
}

SEC("lsm/bprm_creds_for_exec")
int BPF_PROG(bprm_creds_for_exec, struct linux_binprm *bprm) 
{
  struct inode_elem *inode;
  struct task_struct *current_task = (struct task_struct *)bpf_get_current_task_btf();
  
  // check inode cache
  inode = bpf_inode_storage_get(&inode_storage_map, current_task->fs->pwd.dentry->d_inode, 0, BPF_LOCAL_STORAGE_GET_F_CREATE); 

  if (inode == NULL) {
    bpf_printk("something has gone wrong in inode storage");
    return 0;
  }
  
  // initialize node if not already, if not init then we walk file path to check tracking
  if (inode_initialized(inode) == 0) {
    // walk directory and check if its in there
    // inode not initialized (not cached)
    if (in_tracking_dir(current_task->fs->pwd.dentry) == 1) {
      set_inode_tracking(inode);
      // bpf_printk("setting tracking bit to 1");
    }
  }

  if (get_inode_tracking(inode) == 0) {
    return 0; // not tracking
  }

  struct entry_t new_entry = {
    .pid = current_task->pid,
    .utime = current_task->utime,
    .gtime = current_task->gtime,
    .inode_inum = bprm->file->f_inode->i_ino,
    .inode_uid = bprm->file->f_inode->i_uid.val,
    .inode_guid = bprm->file->f_inode->i_gid.val,
    .op = EXEC,
  };
  read_path_name(&new_entry, bprm->file->f_path.dentry);
  //bpf_probe_read_kernel_str(new_entry.file_name, FILE_PATH_MAX, bprm->file->f_path.dentry->d_iname);
  bpf_ringbuf_output(&ringbuf, &new_entry, sizeof(struct entry_t), 0);

  return 0;
}
