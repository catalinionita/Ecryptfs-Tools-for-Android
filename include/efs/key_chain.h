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

#ifndef EFS_KEY_CHAIN_H
#define EFS_KEY_CHAIN_H

#define PASSWD_SALT_LEN 16
#define ECRYPTFS_KEY_LEN 16
#define SHA512_DIGEST_LENGTH 64
#define ECRYPTFS_SIG_LEN SHA512_DIGEST_LENGTH
#define KEY_TYPE "user"
#define PGP_DIGEST_ALGO_SHA512   10

/* generic types for x86 to use with ecryptfs.h */
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed char s8;
#include "include/linux/ecryptfs.h"

struct crypto_header {
        unsigned char fefek[ECRYPTFS_KEY_LEN];
        unsigned char fnek[ECRYPTFS_KEY_LEN];
        unsigned char salt[PASSWD_SALT_LEN];
        unsigned char signature[SHA512_DIGEST_LENGTH];
        int stat;
};

int add_ecryptfs_key(unsigned char *key, char *sig_hex, unsigned char *salt);
int remove_ecryptfs_key(char *sig);
void convert_to_hex_format(unsigned char *src, char *dest, size_t src_len);

#endif /* EFS_KEY_CHAIN_H */
