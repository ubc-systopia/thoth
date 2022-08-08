/*
 */

#include "vmlinux.h"
#include "record.h"

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

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1 << 12);
} ringbuf SEC(".maps");


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

SEC("lsm/file_permission")
int BPF_PROG(file_permission, struct file *file, int mask) 
{
  struct task_struct *current_task;
  uint32_t perms;

  if (is_inode_dir(file->f_inode)) {
    return 0;
  }

  current_task = (struct task_struct *)bpf_get_current_task_btf();
  int pid = current_task->pid;

  perms = file_mask_to_perms((file->f_inode)->i_mode, mask);
  
  if ((perms & (FILE__READ)) != 0) {
    // bpf_printk("read! inode uid: %u", file->f_inode->i_ino);
    struct entry_t new_entry = {
      .pid = pid,
      .utime = current_task->utime,
      .gtime = current_task->gtime,
      .inode_inum = file->f_inode->i_ino,
      .inode_uid = file->f_inode->i_uid.val,
      .inode_guid = file->f_inode->i_gid.val,
      .op = READ,
    };
    bpf_ringbuf_output(&ringbuf, &new_entry, sizeof(struct entry_t), 0);
  } 

  return 0;
}

SEC("lsm/file_open")
int BPF_PROG(file_open, struct file *file) 
{
  return 0;
}
