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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ARG_TRACK_DIR     "--track-dir"
#define ARG_REMOVE_DIR    "--remove-dir"

#define MATCH_ARGS(str1, str2) if (strcmp(str1, str2) == 0)
#define CHECK_ATTR(argc, min) if (argc < min) { exit(-1); }

int add_dir(char* arg) {
  return 0;
}

int remove_dir(char* arg) {
  return 0;
}

int main(int argc, char *argv[])
{
  CHECK_ATTR(argc, 2);

  MATCH_ARGS(argv[1], ARG_TRACK_DIR) {
    printf("hello\r\n");
    CHECK_ATTR(argc, 3);
    add_dir(argv[2]);
    return 0;
  }

  MATCH_ARGS(argv[1], ARG_REMOVE_DIR) {
    CHECK_ATTR(argc, 3);
    remove_dir(argv[2]); 
    return 0;
  }

  return 0; 
}
