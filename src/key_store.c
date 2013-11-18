/**
 * @file   key_store.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 17:56:53 2013
 *
 * @brief
 * Key store interface for EFS
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

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <efs/key_chain.h>
#include <efs/file_utils.h>
#include <efs/efs.h>
#include <efs/process.h>
#include <efs/crypto.h>
#include <efs/key_store.h>

/**
 * Construction to get the private storage path
 * The private storage path for /data/my_encrypted_storage will be /data/.my_encrypted_storage
 *
 * @param path Private EFS path
 * @param storage_path EFS path
 *
 * @return 0 on succes, negative value on error
 */
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

/**
 * Get the recovery path
 * Recovery path is a temporary path were data recovery is attempted
 * Recovery path can be specified by setting DATA_RECOVERY_PATH
 *
 * @param recovery_path Recovery path
 * @param storage_path EFS path
 *
 * @return 0 on success, negative value on error
 */
int get_recovery_path(char *recovery_path, char *storage_path)
{
    int i = strlen(storage_path) - 1;
    int len = strlen(DATA_RECOVERY_PATH);

    for (; i > 0; i--)
        if (storage_path[i] == '/')
            break;

    if (len + strlen(storage_path) - i > MAX_PATH_LENGTH)
        return -1;

    strcpy(recovery_path, DATA_RECOVERY_PATH);
    strcpy(recovery_path + len, storage_path + i);

    return 0;
}

/**
 * Clean up a path by remove multiple //
 * No relative paths are allowed
 *
 * @param storage_path Path
 *
 * @return 0 on succes, negative value on error
 */
int sanitize_storage_path(char *storage_path)
{
    int ret = -1, i = 0;

    if (!storage_path) {
        LOGE("Storage path is null");
        return -1;
    }

    i = strlen(storage_path) - 1;
    if (i > MAX_PATH_LENGTH - 1) {
        LOGE("Invalid storage_path size");
        return -1;
    }

    if (storage_path[0] != '/') {
        LOGE("Full path is required starting from /");
        return -1;
    }
    /* Remove end "/" */
    for (; i > 0; i--) {
        if (storage_path[i] != '/')
            break;
        storage_path[i] = 0;
    }

    if (storage_path[0] == 0) {
        LOGE("root encrypted storage is not allowed");
        return -1;
    }

    ret = access(storage_path, F_OK);
    if (ret < 0) {
        LOGE("Can't access %s", storage_path);
        return ret;
    }

    return 0;
}

/**
 * Get the path were crypto material is stored for a particular EFS
 *
 * @param path Key storage path
 * @param storage_path EFS storage path
 *
 * @return 0 on success, negative value on error
 */
int get_key_storage_path(char *path, char *storage_path)
{
    unsigned char hash[SHA512_DIGEST_LENGTH];
    char id[2 * SHA512_DIGEST_LENGTH + 1];
    char input[MAX_PATH_LENGTH];
    unsigned int i = 0;

    memset(id, 0, sizeof(id));
    memset(input, 0, sizeof(input));
    strncpy(input, storage_path, sizeof(input));

    SHA512((unsigned char *)input, sizeof(hash), hash);

    for (; i < sizeof(hash); i++) {
        snprintf(id + 2 * i, 3, "%02x", hash[i]);
    }

    return snprintf(path, MAX_PATH_LENGTH, "%s%s.%s", KEY_STORAGE_PATH,
            KEY_FILE_NAME, id);
}
