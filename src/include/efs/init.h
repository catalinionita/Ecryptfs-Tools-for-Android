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

#ifndef EFS_INIT_H
#define EFS_INIT_H

/* The strange values are computed by doing SHA512(ANDROID_PRYMARY_USER_DATA_PATH) and SHA512(ANDROID_VIRTUAL_SDCARD_PATH)
 * I couldn't afford to statically link openssl just to compute two predifined sha512 values
 */
#define ANDROID_CRYPTO_HEADER_1 "/data/misc/keystore/.keys.8771124df617880f724ed3190daf5c9ba065296dbc20eaeffd4600e3f236e33272acea8aaa81b7d0990885b19ad197614909baa5fb1d0f456233b8a661e274dd"
#define ANDROID_CRYPTO_HEADER_2 "/data/misc/keystore/.keys.8bf79046ffff0816fcd98590a51b793e19f4419c9290ad83e5b50a9efd335e188c31ff9cd3aaa9740eab480ad3bf1f553d83f129889f65e1fccc525cb67b538d"
#define CRYPTO_HEADER_COPY "/mnt/secure/.keys.8771124df617880f724ed3190daf5c9ba065296dbc20eaeffd4600e3f236e33272acea8aaa81b7d0990885b19ad197614909baa5fb1d0f456233b8a661e274dd"

#define MAX_PATH_LENGTH 1024

#ifdef __cplusplus
extern "C" {
#endif
        extern int android_check_primary_user_encrypted();
#ifdef __cplusplus
}
#endif
#endif                          /* EFS_INIT_H */
