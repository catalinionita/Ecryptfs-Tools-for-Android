 /**
 * @file   efs.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 16:05:51 2013
 *
 * @brief
 * Encrypted File Storage (EFS) API implementation
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

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <cutils/properties.h>
#include <efs/efs.h>
#include <efs/key_chain.h>
#include <efs/file_utils.h>
#include <efs/process.h>
#include <efs/crypto.h>
#include <efs/key_store.h>
#include <efs/mount_utils.h>

/**
 * Internal function to set EFS state
 *
 * @param storage_path EFS path
 * @param state State of encryption: encrypted, not encrypted, encryption in progress
 *
 * @return 0 on success, negative value in case of an error
 *
 */
static int EFS_set_status(char *storage_path, int state)
{
    char key_path[MAX_PATH_LENGTH];
    struct crypto_header header;
    int ret = -1;

    ret = get_key_storage_path(key_path, storage_path);
    if (ret < 0) {
        LOGE("Unable to get key storage path for %s", storage_path);
        return ret;
    }

    ret = read_crypto_header(&header, key_path);
    if (ret < 0) {
        LOGE("Unable to read crypto header from %s", key_path);
        return ret;
    }

    header.stat = state;
    ret = write_crypto_header(&header, key_path);
    if (ret < 0) {
        LOGE("Unable to write crypto header to %s", key_path);
        return ret;
    }

    return 0;
}

/**
 * Internal function to encrypt an EFS
 *
 * @param storage_path EFS path
 * @param passwd Passwd to protect the master key
 *
 * @return 0 on success, negative value on error
 */
static int encrypt_storage(char *storage_path, char *passwd)
{
    char private_dir_path[MAX_PATH_LENGTH];
    char key_storage_path[MAX_PATH_LENGTH];
    int ret = -1;

    ret = generate_crypt_info(storage_path, passwd);
    if (ret < 0) {
        LOGE("Error generating crypto material for efs storage %s",
             storage_path);
        return ret;
    }

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }
    // ToDo split key_storage path into get_key storage and get key name
    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }

    ret =
        mount_ecryptfs(private_dir_path, private_dir_path, passwd,
               key_storage_path);
    if (ret < 0) {
        LOGE("Error mounting %s", private_dir_path);
        return ret;
    }

    ret = copy_dir_content(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error copy data to  private storage %s",
             private_dir_path);
        /* Try to fail gracefully */
        remove_dir_content(private_dir_path);
        umount_ecryptfs(private_dir_path);
        remove(private_dir_path);
        remove(key_storage_path);
        return ret;
    }

    ret = umount_ecryptfs(private_dir_path);
    if (ret < 0) {
        LOGE("Error unmounting private storage %s", private_dir_path);
        return ret;
    }

    ret = remove_dir_content(storage_path);
    if (ret < 0) {
        LOGE("Error removing data from %s directory", storage_path);
        return ret;
    }

    ret = EFS_set_status(storage_path, STORAGE_ENCRYPTION_COMPLETED);
    if (ret < 0) {
        LOGE("Failed to finish storage encryption");
        return ret;
    }

    return 0;
}

/**
 * Get encryption state for an EFS
 *
 * @param storage_path EFS path
 *
 * @return encryption state or negative value in case of an error
 */
int EFS_get_status(char *storage_path)
{
    char key_path[MAX_PATH_LENGTH];
    struct crypto_header header;
    int ret = -1;

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Unable to clean %s", storage_path);
    }

    ret = get_key_storage_path(key_path, storage_path);
    if (ret < 0) {
        LOGE("Unable to get key storage path for %s", key_path);
        return ret;
    }

    ret = read_crypto_header(&header, key_path);
    if (ret < 0) {
        LOGE("Unable to read crypto header from %s", key_path);
        return ret;
    }

    return header.stat;
}

/**
 * Create an EFS
 *
 * @param storage_path EFS path
 * @param passwd User passwd to secure EFS encryption key
 *
 * @return 0 on success, negative value on error
 */
int EFS_create(char *storage_path, char *passwd)
{
    char private_dir_path[MAX_PATH_LENGTH];
    int ret = -1;

    if (!passwd) {
        LOGE("Null passwd provided");
        return ret;
    }

    if (strlen(passwd) < MIN_PASSWD_LEN) {
        LOGE("Passwd too short");
        return ret;
    }

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret == 0) {
        LOGE("Secure storage already exist for %s", storage_path);
        return -1;
    }

    ret = encrypt_storage(storage_path, passwd);
    if (ret < 0) {
        LOGE("Error encrypting efs storage %s", storage_path);
        return ret;
    }

    LOGI("Secure storage created for %s", storage_path);
    return 0;
}

/**
 * Unlock EFS
 *
 * @param storage_path EFS path
 * @param passwd Passwd to access encryption key
 *
 * @return 0 on success, negative value on error
 */
int EFS_unlock(char *storage_path, char *passwd)
{
    char private_dir_path[MAX_PATH_LENGTH];
    char key_storage_path[MAX_PATH_LENGTH];
    int ret = -1;

    if (!passwd) {
        LOGE("Null passwd provided");
        return ret;
    }

    if (strlen(passwd) < MIN_PASSWD_LEN) {
        LOGE("Passwd too short");
        return ret;
    }

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = EFS_get_status(storage_path);
    if (ret != STORAGE_ENCRYPTION_COMPLETED) {
        LOGE("Unable to unlock storage. Storage encryption failed.");
        return ret;
    }

    ret =
        mount_ecryptfs(private_dir_path, storage_path, passwd,
               key_storage_path);
    if (ret < 0) {
        LOGE("Error mounting private storage");
        return ret;
    }

    LOGI("Secure storage %s unlocked", storage_path);
    return 0;
}

/**
 * Lock an EFS
 *
 * @param storage_path EFS path
 *
 * @return 0 on success, negative value on error
 */
int EFS_lock(char *storage_path)
{
    int ret = -1;

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = umount_ecryptfs(storage_path);
    if (ret < 0) {
        LOGE("Error unmounting efs storage");
        return ret;
    }

    LOGI("Secure storage %s locked", storage_path);
    return 0;
}

/**
 * Change passwd of an EFS
 *
 * @param storage_path EFS path
 * @param old_passwd Old EFS passwd
 * @param new_passwd New EFS passwd
 *
 * @return 0 on success, negative value on error
 */
int EFS_change_password(char *storage_path, char *old_passwd, char *new_passwd)
{
    int ret = -1;

    if (!old_passwd || !new_passwd) {
        LOGE("Null passwd provided");
        return ret;
    }

    if (strlen(old_passwd) < MIN_PASSWD_LEN) {
        LOGE("Old passwd too short");
        return ret;
    }

    if (strlen(new_passwd) < MIN_PASSWD_LEN) {
        LOGE("New passwd too short");
        return ret;
    }

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = change_passwd(storage_path, old_passwd, new_passwd);
    if (ret < 0) {
        LOGE("Error changing EFS passwd");
        return ret;
    }

    LOGI("Change passwd successful for %s storage", storage_path);
    return 0;
}

/**
 * Remove an EFS
 *
 * @param storage_path EFS path
 *
 * @return 0 on success, negative value on error
 */
int EFS_remove(char *storage_path)
{
    char private_dir_path[MAX_PATH_LENGTH];
    char key_storage_path[MAX_PATH_LENGTH];
    int ret = -1;

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGE("Could not access  %s", private_dir_path);
        return ret;
    }

    ret = check_fs_mounted(private_dir_path);
    if (ret < 0) {
        LOGE("Error checking mount options for %s", storage_path);
        return ret;
    }
    if (ret == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return 0;
    }

    ret = remove_dir(private_dir_path);
    if (ret < 0) {
        LOGE("Error removing private storage");
        return ret;
    }

    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = unlink(key_storage_path);
    if (ret < 0) {
        LOGE("Error removing key");
        return ret;
    }

    LOGI("Secure storage %s removed", storage_path);
    return 0;
}

/**
 * Recover encrypted data from an EFS. The EFS will be destroyed.
 *
 * @param storage_path EFS path
 * @param passwd EFS passwd
 *
 * @return
 */
int EFS_recover_data_and_remove(char *storage_path, char *passwd)
{
    char private_dir_path[MAX_PATH_LENGTH];
    char key_storage_path[MAX_PATH_LENGTH];
    char recovery_path[MAX_PATH_LENGTH];
    int ret = -1;

    if (!passwd) {
        LOGE("Null passwd provided");
        return ret;
    }

    if (strlen(passwd) < MIN_PASSWD_LEN) {
        LOGE("Passwd too short");
        return ret;
    }

    ret = sanitize_storage_path(storage_path);
    if (ret < 0) {
        LOGE("Invalid storage path");
        return ret;
    }

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGE("Could not access  %s", private_dir_path);
        return ret;
    }

    ret = check_fs_mounted(private_dir_path);
    if (ret < 0) {
        LOGE("Error getting mount options for %s", storage_path);
        return ret;
    }

    if (ret == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return -1;
    }

    ret = get_recovery_path(recovery_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage");
        return ret;
    }

    ret = mkdir(recovery_path, S_IRWXU);
    if (ret < 0) {
        LOGE("mkdir %s fail", recovery_path);
        return ret;
    }

    ret =
        mount_ecryptfs(private_dir_path, recovery_path, passwd,
               key_storage_path);

    if (ret < 0) {
        LOGE("Error mounting private storage %s to %s",
             private_dir_path, recovery_path);
        return ret;
    }

    ret = copy_dir_content(storage_path, recovery_path);
    if (ret < 0) {
        LOGE("Error copy data to storage");
        return ret;
    }

    ret = umount_ecryptfs(recovery_path);
    if (ret < 0) {
        LOGE("Error unmounting");
        return ret;
    }

    ret = remove(recovery_path);
    if (ret < 0) {
        LOGE("unable to remove %s", recovery_path);
        return ret;
    }

    ret = EFS_remove(storage_path);
    if (ret < 0) {
        LOGE("Could not remove efs storage");
        return ret;
    }

    LOGI("Secure storage %s restored", storage_path);
    return 0;
}
