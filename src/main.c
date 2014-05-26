/**
 * @file   test.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 20:40:50 2013
 *
 * @brief
 * Testing tool for EFS library
 *
 */
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <efs/efs.h>
#include <efs/file_utils.h>

void show_help()
{
	printf("Usage: efs-tools storage <command> <params>\n");
	
	printf("Posible commands\n \
		create\n \
		\t->efs-tools storage create <path> <password>\n \
		unlock\n \
		\t->efs-tools storage unlock <path> <password>\n \
		lock\n \
		\t->efs-tools storage lock <path>\n \
		remove\n \
		\t->efs-tools storage remove <path>\n \
		change password\n \
		\t->efs-tools storage change_passwd <path> <old_password> <new_password>\n \
		restore\n \
		\t->efs-tools storage restore <path> <password>\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc == 1) {
        printf("Tool to manage encrypted storages\n");
        show_help();
        return 0;
    }

    if (argc < 3) {
        printf("Invalid usage\n");
        show_help();
        return -1;
    }

    if (strcmp(argv[1], "storage") == 0) {

        if (strcmp(argv[2], "create") == 0) {
            if (argc != 5) {
                printf("Incorect usage of create storage\n");
                return -1;
            }
            return EFS_create(argv[3], argv[4]);
        }

        if (strcmp(argv[2], "unlock") == 0) {
            if (argc != 5) {
                printf("Incorect usage of unlock storage\n");
                return -1;
            }
            return EFS_unlock(argv[3], argv[4]);
        }

        if (strcmp(argv[2], "lock") == 0) {
            if (argc != 4) {
                printf("Incorect usage of lock storage\n");
                return -1;
            }
            return EFS_lock(argv[3]);
        }

        if (strcmp(argv[2], "change_passwd") == 0) {
            if (argc != 6) {
                printf
                    ("Incorect usage of change storage password\n");
                return -1;
            }
            return EFS_change_password(argv[3], argv[4], argv[5]);
        }

        if (strcmp(argv[2], "remove") == 0) {
            if (argc != 4) {
                printf("Incorect usage of storage destroy\n");
                return -1;
            }
            return EFS_remove(argv[3]);
        }

        if (strcmp(argv[2], "stat") == 0) {
            if (argc != 4) {
                printf("Incorect usage of stat\n");
                return -1;
            }
            ret = EFS_get_status(argv[3]);
            printf("Storage status = %d", ret);
            return 0;
        }

        if (strcmp(argv[2], "restore") == 0) {
            if (argc != 5) {
                printf("Incorect usage of storage restore\n");
                return -1;
            }
            return EFS_recover_data_and_remove(argv[3], argv[4]);
        }
    }

    if (strcmp(argv[1], "utils") == 0) {
        if (strcmp(argv[2], "size") == 0) {
            if (argc != 4) {
                printf("Incorect usage of size\n");
                return -1;
            }
            off_t size = 0;
            get_dir_size(argv[3], &size);
            printf("Dir size = %d bytes", size);
            return 0;
        }
        if (strcmp(argv[2], "cp") == 0) {
            if (argc != 6) {
                printf("Incorect usage of copy dir content\n");
                return -1;
            }
            copy_dir_content(argv[3], argv[4]);
            return 0;
        }
        if (strcmp(argv[2], "rm") == 0) {
            if (argc != 5) {
                printf("Incorect usage of rm dir content\n");
                return -1;
            }
            remove_dir_content(argv[3]);
            return 0;
        }
    }

    printf("Invalid usage\n");
    show_help();

    return -1;
}
