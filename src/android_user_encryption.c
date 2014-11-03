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

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "hardware_legacy/power.h"

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
 * Restart Android framework
 *
 */
int android_restart_framework(int user)
{
    struct stat st;
    int ret;

    char prop_value[PROP_VALUE_MAX];
    memset(prop_value, 0, sizeof(prop_value));
    snprintf(prop_value, sizeof(prop_value), "%d", user);
    property_set("efs.selected_user", prop_value);

    property_set("vold.decrypt", "trigger_reset_main");
    sleep(2);

    killProcessesWithOpenFiles("/data", 2);
    ret = umount2("/data", MNT_FORCE);
    if (ret < 0) {
        LOGE("Failed to unmount /data");
        return ret;
    }
    sleep(2);

    memset(prop_value, 0, sizeof(prop_value));
    property_get("efs.encrypted_list", prop_value, "");
    // primary user encrypted
    if (strstr(prop_value, ",0 ")) {
        ret = remove_dir_content("/data/data");
        if (ret < 0) {
            LOGE("Failed to remove dir content from /data/data");
            return ret;
        }
    }

    property_set("crypto.primary_user", "decrypted");
    sleep(2);
    property_set("vold.decrypt", "trigger_restart_framework");

    return 1;
}

/**
 * Encrypt Android primary user data
 *
 * @param password Android user password
 *
 * @return 0 on success, negative value on error
 */
static int android_encrypt_primary_data(char *password)
{
    char data_path[MAX_PATH_LENGTH], media_path[MAX_PATH_LENGTH], buf[10];
    char lockid[32] = { 0 };
    off_t size = 0;
    int ret = -1;

    property_set("crypto.primary_user", "encrypting");

    /*
     * Kill apps that hold open files targeted paths
     * Sending SIGKILL is brutal, but efficient
     */
    memset(data_path, 0, sizeof(data_path));
    sprintf(data_path, "%s%d", ANDROID_USER_DATA_PATH, PRIMARY_USER);
    killProcessesWithOpenFiles(ANDROID_USER_DATA_PATH, 2);
    memset(media_path, 0, sizeof(media_path));
    sprintf(media_path, "%s%d", ANDROID_VIRTUAL_SDCARD_PATH, PRIMARY_USER);
    killProcessesWithOpenFiles(ANDROID_VIRTUAL_SDCARD_PATH, 2);

    /* Acquire a power lock so device won't enter sleep */
    snprintf(lockid, sizeof(lockid), "enablecrypto%d", (int)getpid());
    acquire_wake_lock(PARTIAL_WAKE_LOCK, lockid);

    /*
     * Get total size of encrypted data  and export it to GUI via
     * efs.encrypt.size property
     */
    ret = get_dir_size(data_path, &size);
    if (ret < 0) {
        LOGE("Unable to get dir size for %s", data_path);
        release_wake_lock(lockid);
        return ret;
    }
    ret = get_dir_size(media_path, &size);
    if (ret < 0) {
        LOGE("Unable to get dir size for %s", media_path);
        release_wake_lock(lockid);
        return ret;
    }
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d", size);
    property_set("efs.encrypt.size", buf);

    /* Create secure storages for /data/data and /data/media/0 */
    ret = EFS_create(data_path, PRIMARY_USER, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", data_path);
        release_wake_lock(lockid);
        return ret;
    }

    ret = EFS_create(media_path, PRIMARY_USER, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", media_path);
        release_wake_lock(lockid);
        return ret;
    }

    /* Restart device */
    android_reboot(ANDROID_RB_RESTART, 0, 0);

    return 0;
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
    char path[MAX_PATH_LENGTH];
    int ret = -1;

    LOGI("Encrypt user data for %d", user);

    /* Primary user is device owner so it gets special attention */
    if (user == PRIMARY_USER) {
        return android_encrypt_primary_data(password);
    }

    android_stop_services();
    sleep(5);

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%d", ANDROID_USER_DATA_PATH, user);
    ret = EFS_create(path, user, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", path);
        return ret;
    }

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%d", ANDROID_VIRTUAL_SDCARD_PATH, user);
    ret = EFS_create(path, user, password);
    if (ret < 0) {
        LOGE("Unable to create efs storage %s", path);
        return ret;
    }

    ret = android_unlock_user_data(0, user, password);
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

    /*
     * Hopefully, the init library has saved a copy of the crypto header
     */
    ret = read_crypto_header(&header, CRYPTO_HEADER_COPY[0]);
    if (ret < 0)
        return ret;

    /* Test the user provided password */
    ret = check_passwd(&header, password);
    if (ret < 0)
        return ret;

    /* Setting vold.decrypt will stop class main */
    property_set("vold.decrypt", "trigger_reset_main");
    sleep(2);
    /* Force unmount of the tmpfs
    Despite using MNT_FORCE with umount, partition
    could not be unmounted on some systems.
    Therefore, we kill any processess still working there */
    killProcessesWithOpenFiles("/data", 2);
    umount2("/data", MNT_FORCE);
    sleep(2);

    /* Unlock /data/data and /data/media/0 */
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
int android_unlock_user_data(int from_init, int user, char *password)
{
    char storage_path[MAX_PATH_LENGTH];
    char private_dir_path[MAX_PATH_LENGTH];
    int ret = -1;

    LOGI("Unlock user %d", user);

    if (user == PRIMARY_USER) {
        if (from_init)
            property_set("efs.selected_user", "0");

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

    if (from_init) {
        char prop_value[PROP_VALUE_MAX];
        memset(prop_value, 0, sizeof(prop_value));
        snprintf(prop_value, sizeof(prop_value), "%d", user);
        property_set("efs.selected_user", prop_value);

        property_set("vold.decrypt", "trigger_reset_main");
        sleep(2);

        killProcessesWithOpenFiles("/data", 2);
        if (umount2("/data", MNT_FORCE) < 0)
            LOGE("umount failed: %s", strerror(errno));
        sleep(2);
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

    if (from_init) {
        property_set("crypto.primary_user", "decrypted");
        sleep(2);
        property_set("vold.decrypt", "trigger_restart_framework");
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
 * Decrypt Android primary user data.
 *
 * @param password Android user passwd
 *
 * @return 0 on success, negative value on error
 */
static int android_decrypt_primary_user_data(char *password)
{
    char data_path[MAX_PATH_LENGTH], sdcard_path[MAX_PATH_LENGTH];
    char lockid[32] = { 0 };
    int ret = -1;

    property_set("crypto.primary_user", "decrypting");

    /*
     * Send SIGKILL to all processes that have open files on /data/data
     * and /data/media/0 than "gently" unmount the ecryptfs mountpoints
     */
    killProcessesWithOpenFiles(ANDROID_PRIMARY_USER_DATA_PATH, 2);
    ret = umount_ecryptfs(ANDROID_PRIMARY_USER_DATA_PATH);
    if (ret < 0) {
        LOGE("Error unmounting %s", ANDROID_PRIMARY_USER_DATA_PATH);
        return ret;
    }
    memset(sdcard_path, 0, sizeof(sdcard_path));
    sprintf(sdcard_path, "%s%d/", ANDROID_VIRTUAL_SDCARD_PATH,
        PRIMARY_USER);
    killProcessesWithOpenFiles(sdcard_path, 2);
    ret = EFS_lock(sdcard_path);
    if (ret < 0) {
        LOGE("Can't unlock efs storage %s", sdcard_path);
        return ret;
    }

    /* Get a power wakelock so we don't enter sleep */
    snprintf(lockid, sizeof(lockid), "enablecrypto%d", (int)getpid());
    acquire_wake_lock(PARTIAL_WAKE_LOCK, lockid);

    /*
     * Restore the encrypted data to plain text for /data/data/
     * and /data/media/0/
     */
    memset(data_path, 0, sizeof(data_path));
    sprintf(data_path, "%s%d/", ANDROID_USER_DATA_PATH, PRIMARY_USER);
    ret = EFS_recover_data_and_remove(data_path, password);
    if (ret < 0) {
        LOGE("Error decrypting efs storage %s", data_path);
        release_wake_lock(lockid);
        return ret;
    }
    ret = EFS_recover_data_and_remove(sdcard_path, password);
    if (ret < 0) {
        LOGE("Error decrypting efs storage %s", sdcard_path);
        release_wake_lock(lockid);
        return ret;
    }

    /* Reboot device */
    android_reboot(ANDROID_RB_RESTART, 0, 0);

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
    int ret = -1;

    LOGI("Decrypt user %d data", user);

    if (user == PRIMARY_USER) {
        return android_decrypt_primary_user_data(password);
    }

    android_stop_services();

    memset(storage_path, 0, sizeof(storage_path));
    sprintf(storage_path, "%s%d/", ANDROID_USER_DATA_PATH, user);
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
    int ret = -1;

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
