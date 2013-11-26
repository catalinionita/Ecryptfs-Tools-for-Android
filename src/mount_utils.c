/**
 * @file   mount_utils.c
 * @author Catalin Ionita <catalin.ionita>
 * @date   Mon Oct 14 18:13:30 2013
 *
 * @brief
 * API for managing ecryptfs mounts
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
#include <errno.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
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
#include <efs/mount_utils.h>

/**
 * Mounts a file system
 *
 * @param source Source
 * @param target Mount point
 * @param type FS type
 * @param mountflags Flags
 * @param opts Options
 *
 * @return 0 for success, negative value in case of an error
 */
int mount_fs(const char *source, const char *target, const char *type,
         unsigned long mountflags, char *opts)
{
    int ret;

    ret = mount(source, target, type, mountflags, opts);
    if (ret < 0) {
        ret = -errno;
        LOGE("Could not mount ecryptfs - error %d: %s", errno,
             strerror(errno));
        return ret;
    }

    return ret;
}

/**
 * Check for an ecryptfs mount point and extract mount options
 *
 * @param path Ecryptfs device path or mount path
 * @param mount_options Mount options are getting populated in case of a match
 *
 * @return 1 for found, 0 for not found and negative value in case of an error
 */
int get_mount_options(const char *path, char *mount_options)
{
    char device[MAX_OPTION_LENGTH];
    char mount_path[MAX_OPTION_LENGTH];
    char type[MAX_OPTION_LENGTH];
    char line[MAX_LINE_LENGTH];
    int found = 0;
    FILE *fp;

    fp = fopen("/proc/mounts", "r");
    if (!fp) {
        LOGE("Error opening /proc/mounts (%s)", strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%255s %255s %255s %255s", device, mount_path,
               type, mount_options);
        if (!strcmp(type, "ecryptfs")
            && (!strcmp(device, path) || !strcmp(mount_path, path))) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

/**
 * Check if ecryptfs mount point is already there
 *
 * @param path Ecryptfs device path or mount path
 *
 * @return 1 for found, 0 for not found and negative value in case of an error
 */
int check_fs_mounted(const char *path)
{
    char mount_options[MAX_OPTION_LENGTH];
    return get_mount_options(path, mount_options);
}

/**
 * Get fefek and fnek signatures from mount point options
 *
 * @param mount_options Mount options
 * @param fefek_hash_hex Stores fefek signature
 * @param fnek_hash_hex Stores fnek signature
 *
 * @return 0 on success, negative value in case of an error
 */
int get_key_hash_from_mount_options(char *mount_options,
                    char *fefek_hash_hex, char *fnek_hash_hex)
{
    char *option = NULL, *tmp = NULL;
    int ret = -1, count = 0;

    /* Search for ecryptfs hash key through comma-separated mount options */
    option = strtok_r(mount_options, ",", &tmp);
    while (option != NULL && count != 2) {
        if (!strncmp(option, "ecryptfs_sig", strlen("ecryptfs_sig"))) {
            ret =
                sscanf(option, "ecryptfs_sig=%16s", fefek_hash_hex);
            if (ret != 1) {
                LOGE("ecryptfs signature has incorrect format in ecryptfs mount options");
                return -1;
            }
            count++;
        }
        if (!strncmp
            (option, "ecryptfs_fnek_sig",
             strlen("ecryptfs_fnek_sig"))) {
            ret =
                sscanf(option, "ecryptfs_fnek_sig=%16s",
                   fnek_hash_hex);
            if (ret != 1) {
                LOGE("ecryptfs signature has incorrect format in ecryptfs mount options");
                return -1;
            }
            count++;
        }
        option = strtok_r(NULL, ",", &tmp);
    }

    if (count != 2) {
        LOGE("ecryptfs mount params not found");
        return -1;
    }

    return 0;
}

/**
 * Try to unmount ecryptfs mount
 *
 * @param mount_point Mount point
 *
 * @return 0 on success, negative value in case of an error
 */
int ecryptfs_wait_and_unmount(const char *mount_point)
{
    int i, ret;

    for (i = 0; i < WAIT_UNMOUNT_COUNT; i++) {
        ret = umount(mount_point);
        if (ret == 0)
            break;
        /* EINVAL is returned if the directory is not a mountpoint,
         * i.e. there is no filesystem mounted there.  So just get out.
         */
        if (errno == EINVAL)
            break;
        sleep(1);
    }

    if (i == WAIT_UNMOUNT_COUNT) {
        ret = -errno;
        LOGE("Could not unmount ecryptfs - error %d: %s", errno,
             strerror(errno));
        return ret;
    }

    return 0;
}

/**
 * Mount ecryptfs
 *
 * @param path Path
 * @param mount_point Mount Point
 * @param passwd Password
 * @param key_storage_path Key storage path
 *
 * @return 0 on success, negative value in case of an error
 */
int mount_ecryptfs(char *path, char *mount_point, char *passwd,
           char *key_storage_path)
{
    int ret = -1;
    unsigned char fefek_hash[ECRYPTFS_SIG_LEN];
    char fefek_hash_hex[ECRYPTFS_SIG_SIZE_HEX + 1];
    unsigned char fnek_hash[ECRYPTFS_SIG_LEN];
    char fnek_hash_hex[ECRYPTFS_SIG_SIZE_HEX + 1];
    char mount_options[MAX_OPTION_LENGTH];
    struct crypto_header header;
    struct stat st;

    /* Nothing to do if ecryptfs is already mounted on <path> */
    if (check_fs_mounted(path) == 1) {
        LOGE("ecryptfs is already mounted on %s", path);
        return 0;
    }

    ret = stat(mount_point, &st);
    if (ret < 0) {
        LOGE("lstat failed on %s", mount_point);
        return ret;
    }

    ret = read_crypto_header(&header, key_storage_path);
    if (ret < 0) {
        LOGE("Unable to read crypto header");
        return ret;
    }

    ret = check_passwd(&header, passwd);
    if (ret < 0) {
        LOGE("Wrong Password");
        return ret;
    }

    /* Compute hash of fefek */
    SHA512(header.fefek, ECRYPTFS_KEY_LEN, fefek_hash);
    convert_to_hex_format(fefek_hash, fefek_hash_hex, ECRYPTFS_SIG_SIZE);

    /* Compute hash of fnek */
    SHA512(header.fnek, ECRYPTFS_KEY_LEN, fnek_hash);
    convert_to_hex_format(fnek_hash, fnek_hash_hex, ECRYPTFS_SIG_SIZE);

    /* ToDo add gotos for fail */
    /* Add fefek to kernel keyring */
    ret = add_ecryptfs_key(header.fefek, fefek_hash_hex, header.salt);
    if (ret < 0)
        return ret;

    /* Add fnek to kernel keyring */
    ret = add_ecryptfs_key(header.fnek, fnek_hash_hex, header.salt);
    if (ret < 0)
        return ret;

    /* mount ecryptfs */
    snprintf(mount_options, sizeof(mount_options),
         "ecryptfs_sig=%s,ecryptfs_fnek_sig=%s,ecryptfs_cipher=aes,ecryptfs_key_bytes=%d",
         fefek_hash_hex, fnek_hash_hex, ECRYPTFS_KEY_LEN);

    ret = mount_fs(path, mount_point, "ecryptfs", 0, mount_options);
    if (ret < 0) {
        LOGE("Error mounting ecryptfs");
        return ret;
    }

    ret = chown(mount_point, st.st_uid, st.st_gid);
    if (ret < 0) {
        LOGE("chown %s fail", mount_point);
        return ret;
    }

    ret = chmod(mount_point, st.st_mode);
    if (ret < 0) {
        LOGE("chmod %s fail", mount_point);
        return ret;
    }

    return 0;
}

/**
 * Unmount encryptfs
 *
 * @param path Mount point
 *
 * @return 0 on success, negative value in case of an error
 */
int umount_ecryptfs(char *path)
{
    int ret;
    char fefek_hash_hex[ECRYPTFS_SIG_SIZE_HEX + 1];
    char fnek_hash_hex[ECRYPTFS_SIG_SIZE_HEX + 1];
    char mount_options[MAX_OPTION_LENGTH];

    /* get mouth options */
    ret = get_mount_options(path, mount_options);
    if (ret < 0) {
        LOGE("Error getting mount options for %s", path);
        return ret;
    }

    /* already unmounted */
    if (ret != 1) {
        LOGE("%s not mounted", path);
        return 0;
    }

    /* get hash of the ecryptfs key */
    ret =
        get_key_hash_from_mount_options(mount_options, fefek_hash_hex,
                        fnek_hash_hex);
    if (ret < 0) {
        LOGE("Error getting hash of ecryptfs key %s", path);
        return ret;
    }

    /* umount ecryptfs */
    ret = ecryptfs_wait_and_unmount(path);
    if (ret < 0) {
        LOGE("Error unmounting ecryptfs");
        return ret;
    }

    /* delete fefek sig key from kernel keychain */
    ret = remove_ecryptfs_key(fefek_hash_hex);
    if (ret < 0) {
        LOGE("Error deleting ecryptfs key");
        return ret;
    }

    /* delete fnek  sig key from kernel keychain */
    ret = remove_ecryptfs_key(fnek_hash_hex);
    if (ret < 0) {
        LOGE("Error deleting ecryptfs key");
        return ret;
    }

    return 0;
}
