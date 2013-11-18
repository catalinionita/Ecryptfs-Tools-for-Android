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

#ifndef EFS_KEY_STORE_H
#define EFS_KEY_STORE_H

#define KEY_FILE_NAME ".keys"
#define KEY_STORAGE_PATH "/data/misc/keystore/"
#define DATA_RECOVERY_PATH "/data/lost+found"

int get_private_storage_path(char *path, char *storage_path);
int get_recovery_path(char *recovery_path, char *storage_path);
int sanitize_storage_path(char *storage_path);
int get_key_storage_path(char *path, char *storage_path);

#endif /* EFS_KEY_STORE_H */
