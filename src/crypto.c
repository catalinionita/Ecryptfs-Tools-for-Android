/**
 * @file   crypto.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 17:18:56 2013
 *
 * @brief
 * Crypto support for EFS
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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <efs/efs.h>
#include <efs/file_utils.h>
#include <efs/key_chain.h>
#include <efs/key_store.h>
#include <efs/crypto.h>

/**
 * Password-Based Key Derivation Function 2
 *
 * @param passwd Password
 * @param passwd_len Password length
 * @param salt Salt
 * @param key Key resulted after derivation
 * @param key_len Key length
 */
void pbkdf2(char *passwd, int passwd_len, unsigned char *salt,
        unsigned char *key, unsigned int key_len)
{
    PKCS5_PBKDF2_HMAC_SHA1(passwd, passwd_len, salt, PASSWD_SALT_LEN,
                   PBKDF2_ITERATIONS, key_len, key);
}

/**
 * Encrypt 256 bit keys using AES-CBC
 * Note that this function is used to encrypt 256 bit text (no padding)
 * @param plain_text_text Plain text
 * @param length Plain text length
 * @param encrypted_text Encryption result
 * @param encryption_key The key used for encryption
 * @param IV IV generated from pbkdf2
 *
 * @return 0 on success, negative value on error
 */
int encrypt_buffer(unsigned char *plain_text, int length,
           unsigned char *encrypted_text,
           unsigned char *encryption_key, unsigned char *IV)
{
    EVP_CIPHER_CTX ctx;
    int len;

    /* Initialize the encryption engine */
    if (!EVP_EncryptInit(&ctx, EVP_aes_256_cbc(), encryption_key, IV)) {
        LOGE("EVP_EncryptInit failed");
        return -1;
    }

	/* Do an extra check before turning off padding */
	if ((length % 32) != 0) {
		LOGE("%d byte keys are not supported", length);
		return -1;
	}

    /* Turn off padding */
    EVP_CIPHER_CTX_set_padding(&ctx, 0);

    /* Encrypt buffer */
    if (!EVP_EncryptUpdate(&ctx, encrypted_text, &len, plain_text, length)) {
        LOGE("EVP_EncryptUpdate failed");
        return -1;
    }

    /* Check the size of the encrypted text */
    if (len != length) {
        LOGE("EVP failed to encrypt text");
        return -1;
    }

    return 0;
}

/**
 * Decrypt text using 256 AES-CBC
 * Note that this function is used to decrypt %256 text (no padding)
 * @param encrypted_text Encrypted buffer
 * @param plain_text Result after decryption
 * @param decryption_key Decryption key
 * @param IV Encryption IV
 *
 * @return 0 on success, negative value on error
 */
int decrypt_buffer(unsigned char *encrypted_text, int length,
           unsigned char *plain_text,
           unsigned char *decryption_key, unsigned char *IV)
{
    EVP_CIPHER_CTX ctx;
    int len;

    /* Initialize the decryption engine */
    if (!EVP_DecryptInit(&ctx, EVP_aes_256_cbc(), decryption_key, IV)) {
        LOGE("EVP_DecryptInit failed");
        return -1;
    }

	/* Do an extra check before turning off padding */
	if ((length % 32) != 0) {
		LOGE("%d byte keys are not supported", length);
		return -1;
	}

    /* Turn off padding */
    EVP_CIPHER_CTX_set_padding(&ctx, 0);

    /* Decrypt buffer */
    if (!EVP_DecryptUpdate(&ctx, plain_text, &len, encrypted_text, length)) {
        LOGE("EVP_DecryptUpdate failed");
        return -1;
    }

    /* Check the size of the buffers */
    if (len != length) {
        LOGE("EVP failed to decrypt buffer");
        return -1;
    }

    return 0;
}

/**
 * Generate crypto material needed for EFS
 *
 * @param header Structure for holding crypto material
 *
 * @return 0 on success, negative value on error
 */
int generate_crypto_header(struct crypto_header *header)
{
    int fd, ret;

    /* TODO: Check if we get suficient entropy from /dev/urandom */
    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open /dev/urandom");
        return fd;
    }

    /* Generate fefek, fnek and salt */
    ret = read(fd, header->fefek, ECRYPTFS_KEY_LEN);
    if (ret != ECRYPTFS_KEY_LEN) {
        LOGE("Can't read from /dev/urandom");
        close(fd);
        return -1;
    }
    ret = read(fd, header->fnek, ECRYPTFS_KEY_LEN);
    if (ret != ECRYPTFS_KEY_LEN) {
        LOGE("Can't read from /dev/urandom");
        close(fd);
        return -1;
    }
    ret = read(fd, header->salt, PASSWD_SALT_LEN);
    if (ret != PASSWD_SALT_LEN) {
        LOGE("Can't read from /dev/urandom");
        close(fd);
        return -1;
    }

    /* Hash fefek, fnek and salt into a signature */
    SHA512((unsigned char *)header,
           ECRYPTFS_KEY_LEN + ECRYPTFS_KEY_LEN + PASSWD_SALT_LEN,
           header->signature);

    close(fd);
    return 0;
}

/**
 * Encrypt crypto header
 *
 * @param header Structure holding crypto material for EFS
 * @param encryption_key Encryption key
 * @param IV Encryption IV
 *
 * @return 0 on success, negative value on error
 */
int encrypt_crypto_header(struct crypto_header *header,
              unsigned char *encryption_key, unsigned char *IV)
{
    unsigned char buffer[SHA512_DIGEST_LENGTH];
    int ret;

    /* encrypt fefek */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->fefek, ECRYPTFS_KEY_LEN);
    ret =
        encrypt_buffer(buffer, ECRYPTFS_KEY_LEN, header->fefek,
               encryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't encrypt fefek");
        return ret;
    }

    /* encrypt fnek */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->fnek, ECRYPTFS_KEY_LEN);
    ret =
        encrypt_buffer(buffer, ECRYPTFS_KEY_LEN, header->fnek,
               encryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't encrypt fnek");
        return ret;
    }

    /* encrypt signature */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->signature, SHA512_DIGEST_LENGTH);
    ret =
        encrypt_buffer(buffer, SHA512_DIGEST_LENGTH, header->signature,
               encryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't encrypt fnek");
        return ret;
    }

    return 0;
}

/**
 * Decrypt crypto header
 *
 * @param header Structure for holding crypto material for EFS
 * @param decryption_key Key for decryption
 * @param IV IV
 *
 * @return 0 on success, negative value on error
 */
int decrypt_crypto_header(struct crypto_header *header,
              unsigned char *decryption_key, unsigned char *IV)
{
    unsigned char buffer[SHA512_DIGEST_LENGTH];
    int ret;

    /* decrypt fefek */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->fefek, ECRYPTFS_KEY_LEN);
    ret =
        decrypt_buffer(buffer, ECRYPTFS_KEY_LEN, header->fefek,
               decryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't decrypt fnek");
        return ret;
    }

    /* decrypt fnek */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->fnek, ECRYPTFS_KEY_LEN);
    ret =
        decrypt_buffer(buffer, ECRYPTFS_KEY_LEN, header->fnek,
               decryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't decrypt fnek");
        return ret;
    }

    /* decrypt signature */
	memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, header->signature, SHA512_DIGEST_LENGTH);
    ret =
        decrypt_buffer(buffer, SHA512_DIGEST_LENGTH, header->signature,
               decryption_key, IV);
    if (ret < 0) {
        LOGE("Couldn't decrypt fnek");
        return ret;
    }

    return 0;
}

/**
 * Write crypto material into a file
 *
 * @param header Structure to hold crypto material
 * @param path File path
 *
 * @return 0 on success, negative value on error
 */
int write_crypto_header(struct crypto_header *header, char *path)
{
    int fd, n;

    fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        LOGE("Can't write crypto header");
        return fd;
    }

    n = write(fd, header, sizeof(struct crypto_header));
    if (n != sizeof(struct crypto_header)) {
        LOGE("Can't write crypto header");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * Read crypto material from a file
 *
 * @param header Structure to hold crypto material
 * @param path File path
 *
 * @return 0 on success, negative value on error
 */
int read_crypto_header(struct crypto_header *header, char *path)
{
    int fd, n;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open key storage file");
        return fd;
    }

    memset(header, 0, sizeof(struct crypto_header));
    n = read(fd, header, sizeof(struct crypto_header));
    if (n != sizeof(struct crypto_header)) {
        LOGE("Malformed crypto header");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * Builds all crypto primitives required by an EFS
 *
 * @param storage_path EFS path
 * @param passwd EFS password
 *
 * @return 0 on success, negative value on error
 */
int generate_crypt_info(char *storage_path, char *passwd)
{
    struct crypto_header header;
    char key_storage_path[MAX_PATH_LENGTH];
    char private_dir_path[MAX_PATH_LENGTH];
    unsigned char buffer[2 * ECRYPTFS_KEY_LEN];
    unsigned char *encryption_key = buffer;
    unsigned char *IV = buffer + ECRYPTFS_KEY_LEN;
    unsigned int passwd_len = strlen(passwd);
    int ret = -1;

    /* Create private directory */
    ret = get_private_storage_path(private_dir_path, storage_path);
    if (ret < 0) {
        LOGE("Error getting private storage for %s", storage_path);
        return ret;
    }
    ret = mkdir(private_dir_path, S_IRWXU);
    if (ret < 0) {
        LOGE("mkdir %s fail", private_dir_path);
        return ret;
    }

    /* Generate fefek, fnek and salt */
    ret = generate_crypto_header(&header);
    if (ret < 0) {
        LOGE("generate_crypto_header for %s failed", private_dir_path);
        return ret;
    }

    /* Generate 512 bits from password; the first 256 bits
	 * will used to protect fefek && fnek, and the rest will generate the IV
     */
    pbkdf2(passwd, passwd_len, header.salt, buffer, 2 * ECRYPTFS_KEY_LEN);
    /* Encrypt fefek and fnek */
    ret = encrypt_crypto_header(&header, encryption_key, IV);
    if (ret < 0) {
        LOGE("encrypt_crypto_header for %s failed", private_dir_path);
        return ret;
    }

    header.stat = STORAGE_ENCRYPTION_NOT_STARTED;
    /* Write header to storage */
    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("get_key_storage_path for %s failed", storage_path);
        return ret;
    }

    ret = write_crypto_header(&header, key_storage_path);
    if (ret < 0) {
        LOGE("fail to write crypto header to %s ", key_storage_path);
        return ret;
    }

    return 0;
}

/**
 * Validate a password string against a crypto header
 *
 * @param header crypto header
 * @param passwd input password
 *
 * @return 0 for success, negative value for error
 */
int check_passwd(struct crypto_header *header, char *passwd)
{
    int passwd_len = strlen(passwd);
    unsigned char buffer[2 * ECRYPTFS_KEY_LEN];
    unsigned char *encryption_key = buffer;
    unsigned char *IV = buffer + ECRYPTFS_KEY_LEN;
    unsigned char signature[SHA512_DIGEST_LENGTH];

    pbkdf2(passwd, passwd_len, header->salt, buffer, 2 * ECRYPTFS_KEY_LEN);

    /* Decrypt fefek, fnek and signature */
    decrypt_crypto_header(header, encryption_key, IV);

    /* Compute signature of the header */
    SHA512((unsigned char *)header,
           ECRYPTFS_KEY_LEN + ECRYPTFS_KEY_LEN + PASSWD_SALT_LEN,
           signature);

    /* Check if signature match */
    if (memcmp(signature, header->signature, SHA512_DIGEST_LENGTH) == 0)
        return 0;

    return -1;
}

/**
 * Change password for a secure storage
 *
 * @param storage_path secure storage path
 * @param old_passwd old password
 * @param new_passwd new password
 *
 * @return 0 for success, negative value for error
 */
int change_passwd(char *storage_path, char *old_passwd, char *new_passwd)
{
    struct crypto_header header;
    char key_storage_path[MAX_PATH_LENGTH];
    unsigned char buffer[2 * ECRYPTFS_KEY_LEN];
    unsigned char *encryption_key = buffer, *IV = buffer + ECRYPTFS_KEY_LEN;
    unsigned int new_passwd_len = strlen(new_passwd);
    int ret = -1;

    ret = get_key_storage_path(key_storage_path, storage_path);
    if (ret < 0) {
        LOGE("Invalid key storage");
        return ret;
    }

    ret = read_crypto_header(&header, key_storage_path);
    if (ret < 0) {
        LOGE("Unable to get crypto header");
        return ret;
    }

    /* If check_passwd succeed the header will be decrypted */
    ret = check_passwd(&header, old_passwd);
    if (ret < 0) {
        LOGE("Wrong old passwd");
        return ret;
    }

    /* Generate 256 bits from the new password; the first 128 bits will be used
     * to protect crypto keys, and the rest will be used as IV
     */
    pbkdf2(new_passwd, new_passwd_len, header.salt, buffer,
           2 * ECRYPTFS_KEY_LEN);

    /* Rencrypt crypto header with the new password */
    ret = encrypt_crypto_header(&header, encryption_key, IV);
    if (ret < 0) {
        LOGE("Failed to encrypt crypto header");
        return ret;
    }

    /* Write header to storage */
    ret = write_crypto_header(&header, key_storage_path);
    if (ret < 0) {
        LOGE("Failed to write crypto header to %s ", key_storage_path);
        return ret;
    }

    return 0;
}

/* Debug function */
void dump_crypto_header(struct crypto_header *header)
{
    char buffer[SHA512_DIGEST_LENGTH * 2 + 2];

    memset(buffer, 0, sizeof(buffer));
    convert_to_hex_format(header->fefek, buffer, ECRYPTFS_KEY_LEN);
    LOGE("fefek=%s", buffer);

    memset(buffer, 0, sizeof(buffer));
    convert_to_hex_format(header->fnek, buffer, ECRYPTFS_KEY_LEN);
    LOGE("fnek=%s", buffer);

    memset(buffer, 0, sizeof(buffer));
    convert_to_hex_format(header->salt, buffer, PASSWD_SALT_LEN);
    LOGE("salt=%s", buffer);

    memset(buffer, 0, sizeof(buffer));
    convert_to_hex_format(header->signature, buffer, SHA512_DIGEST_LENGTH);
    LOGE("signature=%s", buffer);
}
