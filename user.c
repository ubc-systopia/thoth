// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2020 Google LLC.
 */

// from: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/tools/testing/selftests/bpf/prog_tests/test_lsm.c

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

#include "track.skel.h"

int main(void)
{
  struct track *skel = NULL;
  int err;

  printf("starting...\r\n");

  skel = track__open_and_load();
  if (!skel) {
    printf("Failed to load bpf skeleton");
    goto close_prog;
  }

  printf("open and load completed\r\n");

  err = track__attach(skel);

close_prog:
  track__destroy(skel);
}
