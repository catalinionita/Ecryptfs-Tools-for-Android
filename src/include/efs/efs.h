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

#ifndef EFS_H
#define EFS_H

#define TAG "EFS"
#define MIN_PASSWD_LEN 4
#define STORAGE_ENCRYPTION_NOT_STARTED 1
#define STORAGE_ENCRYPTION_IN_PROGRESS 2
#define STORAGE_ENCRYPTION_COMPLETED 3

#ifdef __cplusplus
extern "C" {
#endif
        /* EFS Storage API */
        extern int EFS_create(char *storage_path, int user, char *passwd);
        extern int EFS_unlock(char *storage_path, char *passwd);
        extern int EFS_lock(char *storage_path);
        extern int EFS_change_password(char *path, char *old_passwd,
                                               char *new_passwd);
        extern int EFS_remove(char *storage_path);
        extern int EFS_recover_data_and_remove(char *storage_path,
                                                       char *password);
        extern int EFS_get_status(char *storage_path);
	extern int EFS_get_progress(char *storage_path);

        /* Android encrypt user API */
        extern int android_encrypt_user_data(int userId, char *password);
        extern int android_unlock_user_data(int user, char *password);
        extern int android_lock_user_data(int user);
        extern int android_change_user_data_password(int user, char *old_password,
                                             char *new_password);
        extern int android_decrypt_user_data(int user, char *password);
        extern int android_remove_user_encrypted_data(int user);
        extern int android_get_encrypted_user_status(int user);
        extern int android_check_primary_user_encrypted();
#ifdef __cplusplus
}
#endif
#endif                          /* EFS_H */
