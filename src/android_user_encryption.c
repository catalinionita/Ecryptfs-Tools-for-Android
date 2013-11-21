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
#include <efs/android_user_encryption.h>
#include <efs/init.h>
#include "cutils/android_reboot.h"

#define MNT_FORCE       0x00000001

/**
 * Stop Android services
 *
 */
static void android_stop_services()
{
    property_set("ctl.stop", "zygote");
    property_set("ctl.stop", "surfaceflinger");
    sleep(1);
}

/**
 * Start Android services
 *
 */
static void android_start_services()
{
    property_set("ctl.start", "surfaceflinger");
    property_set("ctl.start", "zygote");
}

/**
 * Restart Android services
 *
 */
static void android_restart_services()
{
    android_stop_services();
    android_start_services();
}

/**
 * Encrypt Android user data
 *
 * @param user Android user id
 * @param password Android user password
 *
 * @return 0 on success, negative value on error
 */
int android_encrypt_user_data(int user, char *password)
{
    char data_path[MAX_PATH_LENGTH], media_path[MAX_PATH_LENGTH], buf[10];
    off_t size = 0, total_size = 0;
    int ret = -1;

    LOGI("Encrypt user data for %d", user);

    android_stop_services();
    sleep(5);

    memset(data_path, 0, sizeof(data_path));
    sprintf(data_path, "%s%d", ANDROID_USER_DATA_PATH, user);
    memset(media_path, 0, sizeof(media_path));
    sprintf(media_path, "%s%d", ANDROID_VIRTUAL_SDCARD_PATH, user);

    ret = get_dir_size(data_path, &size);
    if (ret < 0) {
        LOGE("Unable to get dir size for %s", data_path);
        return ret;
    }
    total_size += size;
    ret = get_dir_size(media_path, &size);
    if (ret < 0) {
        LOGE("Unable to get dir size for %s", media_path);
        return ret;
    }
    total_size += size;
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%llu", total_size);
    property_set("efs.encrypt.size", buf);

    if (user == PRIMARY_USER) {
        property_set("crypto.primary_user", "encrypting");
    }

    ret = EFS_create(data_path, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", data_path);
        return ret;
    }

    ret = EFS_create(media_path, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", media_path);
        return ret;
    }

    if (user == PRIMARY_USER) {
        android_reboot(ANDROID_RB_RESTART, 0, 0);
    }

    ret = android_unlock_user_data(user, password);
    if (ret < 0) {
        LOGE("Unable to unlock user data %d", user);
        return ret;
    }

    android_start_services();

    return 0;
}

static int android_unlock_primary_user(char *password)
{
    char storage_path[MAX_PATH_LENGTH];
    char private_dir_path[MAX_PATH_LENGTH];
    struct crypto_header header;
    int ret;

    ret = read_crypto_header(&header, CRYPTO_HEADER_COPY);
    if (ret < 0)
        return ret;

    ret = check_passwd(&header, password);
    if (ret < 0)
        return ret;

    property_set("vold.decrypt", "trigger_reset_main");
    sleep(2);
    umount("/data", MNT_FORCE);

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d", ANDROID_USER_DATA_PATH, PRIMARY_USER);

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGE("Private storage %s does not exist", private_dir_path);
        return ret;
    }

    if (check_fs_mounted(private_dir_path) == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return 0;
    }

    ret = remove_dir_content(storage_path);
    if (ret < 0) {
        LOGE("Error removing data from %s directory", storage_path);
        return ret;
    }

    ret = EFS_unlock(storage_path, password);
    if (ret < 0) {
        LOGE("Error unlocking efs storage %s", storage_path);
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d", ANDROID_VIRTUAL_SDCARD_PATH,
        PRIMARY_USER);

    if (check_fs_mounted(storage_path) == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return 0;
    }

    ret = remove_dir_content(storage_path);
    if (ret < 0) {
        LOGE("Error removing data from %s directory", storage_path);
        return ret;
    }

    ret = EFS_unlock(storage_path, password);
    if (ret < 0) {
        LOGE("Error unlocking efs storage %s", storage_path);
        return ret;
    }

    property_set("crypto.primary_user", "decrypted");
    sleep(2);
    property_set("vold.decrypt", "trigger_restart_framework");

    return 0;
}

/**
 * Mount encrypted data for an Android user
 *
 * @param user Android user id
 * @param password Android user password
 *
 * @return 0 on success, negative value on error
 */
int android_unlock_user_data(int user, char *password)
{
    char storage_path[MAX_PATH_LENGTH];
    char private_dir_path[MAX_PATH_LENGTH];
    int ret = -1;

    LOGI("Unlock user %d", user);

    if (user == PRIMARY_USER) {
        ret = android_unlock_primary_user(password);
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d", ANDROID_USER_DATA_PATH, user);

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGE("Private storage %s does not exist", private_dir_path);
        return ret;
    }

    if (check_fs_mounted(private_dir_path) == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return 0;
    }

    ret = remove_dir_content(storage_path);
    if (ret < 0) {
        LOGE("Error removing data from %s directory", storage_path);
        return ret;
    }

    ret = EFS_unlock(storage_path, password);
    if (ret < 0) {
        LOGE("Error unlocking efs storage %s", storage_path);
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d", ANDROID_VIRTUAL_SDCARD_PATH, user);

    if (check_fs_mounted(storage_path) == 1) {
        LOGE("ecryptfs is already mounted on %s", storage_path);
        return 0;
    }

    ret = remove_dir_content(storage_path);
    if (ret < 0) {
        LOGE("Error removing data from %s directory", storage_path);
        return ret;
    }

    ret = EFS_unlock(storage_path, password);
    if (ret < 0) {
        LOGE("Error unlocking efs storage %s", storage_path);
        return ret;
    }

    return 0;
}

/**
 * Unmount Android user data
 *
 * @param user Android user id
 *
 * @return 0 on success, negative value on error
 */
int android_lock_user_data(int user)
{
    char storage_path[MAX_PATH_LENGTH];
    int ret;

    LOGI("Lock user %d", user);
    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
    ret = EFS_lock(storage_path);
    if (ret < 0) {
        LOGE("Error locking efs storage");
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH, user);
    EFS_lock(storage_path);
    if (ret < 0) {
        LOGE("Error locking efs storage");
        return ret;
    }

    return 0;
}

/**
 * Change password for Android user encrypted data
 *
 * @param user Android user id
 * @param old_password Old user passwd
 * @param new_password New user passwd
 *
 * @return 0 on success, negative value on error
 */
int android_change_user_data_password(int user, char *old_password,
                      char *new_password)
{
    char storage_path[MAX_PATH_LENGTH];
    int ret;

    LOGI("Change user %d passwd", user);
    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
    ret = EFS_change_password(storage_path, old_password, new_password);
    if (ret < 0) {
        LOGE("Error changing efs storage pass");
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH, user);
    ret = EFS_change_password(storage_path, old_password, new_password);
    if (ret < 0) {
        LOGE("Error changing efs storage pass");
        return ret;
    }

    return 0;
}

/**
 * Decrypt Android user data.
 *
 * @param user Android user id
 * @param password Android user passwd
 *
 * @return 0 on success, negative value on error
 */
int android_decrypt_user_data(int user, char *password)
{
    char storage_path[MAX_PATH_LENGTH];
    int ret;

    LOGI("Decrypt user %d data", user);

    android_stop_services();

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
    if (user == PRIMARY_USER) {
        ret = umount_ecryptfs(ANDROID_PRIMARY_USER_DATA_PATH);
        if (ret < 0) {
            LOGE("Error unmounting %s",
                 ANDROID_PRIMARY_USER_DATA_PATH);
            return ret;
        }
    } else {
        ret = EFS_lock(storage_path);
        if (ret < 0) {
            LOGE("Can't unlock efs storage");
            return ret;
        }
    }

    ret = EFS_recover_data_and_remove(storage_path, password);
    if (ret < 0) {
        LOGE("Error decrypting efs storage");
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH, user);

    ret = EFS_lock(storage_path);
    if (ret < 0) {
        LOGE("Can't unlock efs storage");
        return ret;
    }

    ret = EFS_recover_data_and_remove(storage_path, password);
    if (ret < 0) {
        LOGE("Error decrypting efs storage");
        return ret;
    }

    if (user == PRIMARY_USER) {
        android_reboot(ANDROID_RB_RESTART, 0, 0);
    }

    android_start_services();
    return 0;
}

/**
 * Remove all data for an Android user
 *
 * @param user Android user id
 *
 * @return 0 on success, negative value on error
 */
int android_remove_user_encrypted_data(int user)
{
    char storage_path[MAX_PATH_LENGTH];
    int ret;

    LOGI("Remove user %d data", user);
    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
    ret = EFS_remove(storage_path);
    if (ret < 0) {
        LOGE("Error removing efs storage");
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH, user);
    ret = EFS_remove(storage_path);
    if (ret < 0) {
        LOGE("Error removing efs storage");
        return ret;
    }

    return 0;
}

/**
 * Get encryption status for an Android user
 *
 * @param user Android user id
 *
 * @return Encryption status(encrypted/unecrypted/in progress) or negative value in case of an error
 */
int android_get_encrypted_user_status(int user)
{
    int ret, data_storage_status, media_storage_status;
    char storage_path[MAX_PATH_LENGTH], private_dir_path[MAX_PATH_LENGTH];

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGI("User %d not encrypted", user);
        return ret;
    }

    data_storage_status = EFS_get_status(storage_path);
    if (data_storage_status < 0) {
        LOGE("Unable to get status for %s.%d", ANDROID_USER_DATA_PATH,
             user);
        return ret;
    }

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH, user);

    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }

    ret = access(private_dir_path, F_OK);
    if (ret < 0) {
        LOGE("Private storage %s does not exist for media",
             private_dir_path);
        return ret;
    }

    media_storage_status = EFS_get_status(storage_path);
    if (media_storage_status < 0) {
        LOGE("Unable to get status for %s.%d", ANDROID_USER_DATA_PATH,
             user);
        return ret;
    }

    if (media_storage_status != data_storage_status) {
        LOGE("Inconsistent encryption status for user %d", user);
        return -1;
    }

    LOGI("Encryption status for user %d is %d", user, data_storage_status);
    return data_storage_status;
}

/**
 * Check if primary user is encrypted with EFS and set a system property
 * This function is called early during vold startup
 */
int android_check_primary_user_encrypted()
{
    return android_get_encrypted_user_status(PRIMARY_USER);
}
