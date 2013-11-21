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

#ifndef EFS_CRYPTO_H
#define EFS_CRYPTO_H

#define PBKDF2_ITERATIONS 10000
#define SHA512_DIGEST_LENGTH 64

void pbkdf2(char *passwd, int passwd_len, unsigned char *salt,
                   unsigned char *key, unsigned int key_len);
int encrypt_key(unsigned char *plain_text_key,
                       unsigned char *encrypted_key,
                       unsigned char *encryption_key, unsigned char *IV);
int decrypt_key(unsigned char *encrypted_key,
                       unsigned char *plain_text_key,
                       unsigned char *decryption_key, unsigned char *IV);
int generate_crypto_header(struct crypto_header *header);
int encrypt_crypto_header(struct crypto_header *header,
                                 unsigned char *encryption_key,
                                 unsigned char *IV);
int decrypt_crypto_header(struct crypto_header *header,
                                 unsigned char *decryption_key,
                                 unsigned char *IV);
int write_crypto_header(struct crypto_header *header, char *path);
int read_crypto_header(struct crypto_header *header, char *path);
int generate_crypt_info(char *storage_path, char *passwd);
int check_passwd(struct crypto_header *header, char *passwd);
int change_passwd(char *storage_path, char *old_passwd, char *new_passwd);

#endif /* EFS_CRYPTO_H */
