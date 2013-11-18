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

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include "libefs_init.h"

int get_private_storage_path(char *path, char *storage_path)
{
        int i = strlen(storage_path);

        if (i >= MAX_PATH_LENGTH)
                return -1;

        /* Look for the next '/' */
        for (; i > 0; i--)
                if (storage_path[i] == '/')
                        break;

        /* Insert a "." in the new path */
        if (i != 0) {
                strncpy(path, storage_path, i + 1);
                path[i + 1] = '.';
                strcpy(path + i + 2, storage_path + i + 1);
        } else {
                path[0] = '.';
                strcpy(path + 1, storage_path);
        }

        return 0;
}

int android_check_primary_user_encrypted()
{
        char storage_path[MAX_PATH_LENGTH], private_dir_path[MAX_PATH_LENGTH];
        int ret = -1, fd1, fd2;

        memset(storage_path, 0, sizeof(storage_path));
        memset(private_dir_path, 0, sizeof(storage_path));
        sprintf(storage_path, "%s", ANDROID_PRIMARY_USER_DATA_PATH);
        ret = get_private_storage_path(private_dir_path, storage_path);
        if (ret < 0)
                return ret;

        ret = access(private_dir_path, F_OK);
        if (ret < 0)
            return ret;


        memset(storage_path, 0, sizeof(storage_path));
        memset(private_dir_path, 0, sizeof(storage_path));
        sprintf(storage_path, "%s", ANDROID_VIRTUAL_SDCARD_PATH);

        ret = get_private_storage_path(private_dir_path, storage_path);
        if (ret < 0)
                return ret;

        ret = access(private_dir_path, F_OK);
        if (ret < 0)
                return ret;

        fd1 = open(ANDROID_CRYPTO_HEADER_1, O_RDONLY);
        if (fd1 < 0)
            return ret;

        fd2 = open(ANDROID_CRYPTO_HEADER_2, O_RDONLY);
        if (ret < 0) {
            close(fd1);
            return ret;
        }

        close(fd1);
        close(fd2);


        property_set("crypto.primary_user", "encrypted");
		sleep(5);

        return 1;
}
