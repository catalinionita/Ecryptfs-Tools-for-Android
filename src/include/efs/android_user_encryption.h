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

#ifndef ANDROID_USER_ENCRYPTION_H
#define ANDROID_USER_ENCRYPTION_H

#define ANDROID_PRIMARY_USER_DATA_PATH "/data/data"
#define ANDROID_USER_DATA_PATH "/data/user/"
#define ANDROID_VIRTUAL_SDCARD_PATH "/data/media/"
#define PRIMARY_USER 0

#ifdef __cplusplus
extern "C" {
#endif
        /* Android encrypt user API */
        extern int android_encrypt_user_data(int userId, char *password);
        extern int android_unlock_user_data(int from_init, int user, char *password);
        extern int android_lock_user_data(int user);
        extern int android_change_user_data_password(int user, char *old_password,
                                             char *new_password);
        extern int android_decrypt_user_data(int user, char *password);
        extern int android_remove_user_encrypted_data(int user);
        extern int android_get_encrypted_user_status(int user);
        extern int android_check_primary_user_encrypted();
        extern int android_restart_framework(int user);
#ifdef __cplusplus
}
#endif
#endif                          /* ANDROID_USER_ENCRYPTION */
