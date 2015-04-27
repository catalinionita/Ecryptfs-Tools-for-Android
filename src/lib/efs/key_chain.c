/**
 * @file   key_chain.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 17:41:44 2013
 *
 * @brief
 * Functions for managing the kernel key ring.
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

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <efs/key_chain.h>
#include <efs/file_utils.h>
#include <efs/efs.h>
#include <asm/unistd.h>
#include <linux/keyctl.h>

typedef int32_t key_serial_t;

/**
 * Generates an ecryptfs token
 *
 * @param auth_tok Structure holding the resulting token
 * @param passphrase_sig Key signature
 * @param salt Salt
 * @param session_key_encryption_key Encryption key
 */
static void generate_auth_tok(struct ecryptfs_auth_tok *auth_tok,
                  char *passphrase_sig, char *salt,
                  char *session_key_encryption_key)
{
    memset(auth_tok, 0, sizeof(struct ecryptfs_auth_tok));
    auth_tok->version = (((uint16_t) (ECRYPTFS_VERSION_MAJOR << 8) & 0xFF00)
                 | ((uint16_t) ECRYPTFS_VERSION_MINOR & 0x00FF));
    auth_tok->token_type = ECRYPTFS_PASSWORD;
    memcpy(auth_tok->token.password.salt, salt, ECRYPTFS_SALT_SIZE);
    memcpy(auth_tok->token.password.session_key_encryption_key,
           session_key_encryption_key, ECRYPTFS_KEY_LEN);
    strncpy((char *)auth_tok->token.password.signature, passphrase_sig,
        ECRYPTFS_PASSWORD_SIG_SIZE);
    auth_tok->token.password.session_key_encryption_key_bytes =
        ECRYPTFS_KEY_LEN;
    auth_tok->token.password.flags |=
        ECRYPTFS_SESSION_KEY_ENCRYPTION_KEY_SET;
    auth_tok->session_key.encrypted_key[0] = 0;
    auth_tok->session_key.encrypted_key_size = 0;
    auth_tok->token.password.hash_algo = PGP_DIGEST_ALGO_SHA512;
    auth_tok->token.password.flags &= ~(ECRYPTFS_PERSISTENT_PASSWORD);
}

/**
 * Add a key to kernel keyring
 *
 * @param key Key to be added
 * @param sig Key signature (hash of the key)
 * @param salt Salt
 *
 * @return 0 on success, negative value on error
 */
int add_ecryptfs_key(unsigned char *key, char *sig, unsigned char *salt)
{
    int ret;
    struct ecryptfs_auth_tok tok;

    generate_auth_tok(&tok, sig, (char *)salt, (char *)key);

    ret = syscall(__NR_add_key, KEY_TYPE, sig, (void *)&tok, sizeof(struct ecryptfs_auth_tok), KEY_SPEC_USER_KEYRING);
    if (ret == -1) {
        ret = -errno;
        LOGE("Error adding key with sig %s; ret = [%d]\n : %s", sig,
             errno, strerror(errno));
        if (ret == -EDQUOT)
            LOGE("Error adding key to keyring - keyring is full\n");
        return ret;
    }

    return 0;
}

/**
 * Remove encryption key from kernel keyring
 *
 * @param sig Key signature (hash of the key)
 *
 * @return 0 for success, negative value in case of an error
 */
int remove_ecryptfs_key(char *sig)
{
    int ret;
    key_serial_t id;

    id = syscall(__NR_keyctl, KEYCTL_SEARCH, KEY_SPEC_USER_KEYRING, KEY_TYPE, sig, 0);
    if (id < 0) {
        LOGE("Failed to find key with sig %s\n", sig);
        return id;
    }

    ret = syscall(__NR_keyctl, KEYCTL_UNLINK, id, KEY_SPEC_USER_KEYRING);
    if (ret < 0) {
        LOGE("Failed to unlink key with sig %s: %s\n",
             sig, strerror(ret));
        return ret;
    }
    return 0;
}

/**
 * Convert buffer to hex format.
 * Note: This function assume that dest buffer is at least 2*src_len
 *
 * @param src Source buffer
 * @param dest Destination buffer
 * @param src_len Source len
 */
void convert_to_hex_format(unsigned char *src, char *dest, size_t src_len)
{
    size_t i;
    char *ptr;

    ptr = dest;
    for (i = 0; i < src_len; i++) {
        ptr += sprintf(ptr, "%02x", src[i]);
    }
    *ptr = '\0';
}
