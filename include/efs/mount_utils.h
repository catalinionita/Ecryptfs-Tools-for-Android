/**
 * Copyright (C) 2013 Intel Corporation, All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Catalin Ionita <catalin.ionita@intel.com>
 */

#ifndef EFS_MOUNT_UTILS_H
#define EFS_MOUNT_UTILS_H

#define WAIT_UNMOUNT_COUNT 5
#define MAX_LINE_LENGTH 1024
#define MAX_OPTION_LENGTH 256

int get_mount_options(const char *path, char *mount_options);
int check_fs_mounted(const char *path);
int get_key_hash_from_mount_options(char *mount_options,
                                           char *fefek_hash_hex,
                                    char *fnek_hash_hex);
int mount_ecryptfs(char *path, char *mount_point, char *passwd,
                   char *key_storage_path);
int umount_ecryptfs(char *path);

#endif /* EFS_MOUNT_UTILS_H */
