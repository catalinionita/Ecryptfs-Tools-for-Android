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
#include <efs/init.h>
#include <efs/android_user_encryption.h>

int copy(char *source, char *destination)
{
    char *buffer = NULL;
    int rc = 0;
    int fd1 = -1, fd2 = -1;
    struct stat info;
    int brtw, brtr;
    char *p;

    if (stat(source, &info) < 0)
        return -1;

    if ((fd1 = open(source, O_RDONLY)) < 0)
        goto out_err;

    if ((fd2 = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0660)) < 0)
        goto out_err;

    if (!(buffer = malloc(info.st_size)))
        goto out_err;

    p = buffer;
    brtr = info.st_size;
    while (brtr) {
        rc = read(fd1, p, brtr);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtr -= rc;
    }

    p = buffer;
    brtw = info.st_size;
    while (brtw) {
        rc = write(fd2, p, brtw);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtw -= rc;
    }

    rc = 0;
    goto out;
out_err:
    rc = -1;
out:
    if (buffer)
        free(buffer);
    if (fd1 >= 0)
        close(fd1);
    if (fd2 >= 0)
        close(fd2);
    return rc;
}

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
    sprintf(storage_path, "%s/%d", ANDROID_USER_DATA_PATH, PRIMARY_USER);
    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0)
        return ret;

    ret = access(private_dir_path, F_OK);
    if (ret < 0)
        return ret;

    memset(storage_path, 0, sizeof(storage_path));
    memset(private_dir_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s/%d", ANDROID_VIRTUAL_SDCARD_PATH,
        PRIMARY_USER);

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0)
        return ret;

    ret = access(private_dir_path, F_OK);
    if (ret < 0)
        return ret;

    fd1 = open(ANDROID_CRYPTO_HEADER_1, O_RDONLY);
    if (fd1 < 0)
        return fd1;

    fd2 = open(ANDROID_CRYPTO_HEADER_2, O_RDONLY);
    if (fd2 < 0) {
        close(fd1);
        return fd2;
    }

    close(fd1);
    close(fd2);

    /* Make a copy of the crypto headear on /mnt/secure */
    ret = copy(ANDROID_CRYPTO_HEADER_1, CRYPTO_HEADER_COPY);
    if (ret < 0) {
        return ret;
    }

    property_set("crypto.primary_user", "encrypted");

    return 1;
}
