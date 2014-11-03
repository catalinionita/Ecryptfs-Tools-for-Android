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

#define MAX_PATH_LENGTH 1024

static const char ANDROID_CRYPTO_HEADER_1[17][MAX_PATH_LENGTH] = {
    "/data/misc/keystore/.keys.8771124df617880f724ed3190daf5c9ba065296dbc20eaeffd4600e3f236e33272acea8aaa81b7d0990885b19ad197614909baa5fb1d0f456233b8a661e274dd",
    "", "", "", "", "", "", "", "", "",
    "/data/misc/keystore/.keys.41c6b01488b4aff6b026bd5fb644a1aace9e83aab695586b9f2f4701fc3842ad84711d5d955dd5e2b344fa725d2aceffe13dd61c887cd7ecb1fcc16c65ff1d84",
    "/data/misc/keystore/.keys.580c6285029525863c4312696f8af668f19e16417af4b111dd4a644e7704474ec98558e8655195ac1f19a2534e7a61c270a7c051a3d4190b64b67ab84c546e1b",
    "/data/misc/keystore/.keys.fe76ab8437589b9a79d5e75624786984bfb1ee1106b3c3718c1b8b8071b17489f7a3cdd722aa0427753e5b3d5a7d09ed0ffe5d500897676fa0e59b4205c57121",
    "/data/misc/keystore/.keys.45b752d5806159be8d30c1e2f9268be80be54e8f04015943070c686ef7fb6a378b9d1f80036c7ee4dae0dcb6945b9ae929ffb691dd3d5f5d240e3cc0ea4a96f1",
    "/data/misc/keystore/.keys.925ea013f88e9254f6c8e24efc8aad477f224e72cc770a33e13ba08dc56f31bd0431286c2a18254e8122863ffb04f2ea1516b0c6fb9a0bb90bef588e7bdb310b",
    "/data/misc/keystore/.keys.3f3ebf63f6ce70e41df392ecc42f6a44fbfc68435fd867fd19821c8ff29e652c47882c571e140f1d1de4f32cb38a76a602504f62374d63d4f262d7a4676de05a",
    "/data/misc/keystore/.keys.a7b89e19bd752ed375d0ec1b9be8036cb60b20d460b1022789e0bcaefb95f9c2024125fe7d9b639dfedc36679d1c289c9ba131fb99ffcbd93d83eac369a43235"
};
static const char ANDROID_CRYPTO_HEADER_2[17][MAX_PATH_LENGTH] = {
    "/data/misc/keystore/.keys.8bf79046ffff0816fcd98590a51b793e19f4419c9290ad83e5b50a9efd335e188c31ff9cd3aaa9740eab480ad3bf1f553d83f129889f65e1fccc525cb67b538d",
    "", "", "", "", "", "", "", "", "",
    "/data/misc/keystore/.keys.917f7e10b7c2d4f4243813b472a51896f1fe72203fec5320c94cfba7411394ad47bd6c37cc6ee6ac17a3a1d246332eb542b663a648a3d62a49d4298dbafab56c",
    "/data/misc/keystore/.keys.f05a47d310a30f89dd00db599a1c7a56525c84f9c9bd256fedf4900712418653e81e531dc0b9098ba0215f678feb5373992a3cb1bb1f6ad1b705d9ca20c09194",
    "/data/misc/keystore/.keys.444ed7cf36e0ea845a4eb90126ec80b22c0c2eece1ef06272bf3735558d8c5fd85ffa1990b30209655f02777abbb7ec64dbe33d2e668f019d121ba6467027ec3",
    "/data/misc/keystore/.keys.b1ca1c82780ade0ee51d81dc2f6cb53030e5e611893f2fa37f6b6cbe0146e8d0d3a320d7ff7d4d169b3919e14cc493f62ede7d1c38534316da781b0820e03456",
    "/data/misc/keystore/.keys.10e87b8e0324b1d74cd941af036de59f9a34ab3c05866109c8cc50dcc4cf4f2eab7f55e92c23113acd63df1204d64b6b368a26f3436a0b51f43084993fead878",
    "/data/misc/keystore/.keys.97b56d18ffaffe9c2339f77ac3c3f694e65f5859abac10b4a1c4cbcab5a02be70660840d764475e6e50619ca40a193b683e5da47d5c0468d72cc6550c263293f",
    "/data/misc/keystore/.keys.5ac3db625f265f3d51460185663aa0444f3da242caef0cfc15a0efc0c6d6c0befb2bc4b31f4f458bec23f8b525017a12f76edabbe26549f9d0e11aa75f9cfe3f"
};
static const char CRYPTO_HEADER_COPY[17][MAX_PATH_LENGTH] = {
    "/mnt/secure/.keys.8771124df617880f724ed3190daf5c9ba065296dbc20eaeffd4600e3f236e33272acea8aaa81b7d0990885b19ad197614909baa5fb1d0f456233b8a661e274dd",
    "", "", "", "", "", "", "", "", "",
    "/mnt/secure/.keys.41c6b01488b4aff6b026bd5fb644a1aace9e83aab695586b9f2f4701fc3842ad84711d5d955dd5e2b344fa725d2aceffe13dd61c887cd7ecb1fcc16c65ff1d84",
    "/mnt/secure/.keys.580c6285029525863c4312696f8af668f19e16417af4b111dd4a644e7704474ec98558e8655195ac1f19a2534e7a61c270a7c051a3d4190b64b67ab84c546e1b",
    "/mnt/secure/.keys.fe76ab8437589b9a79d5e75624786984bfb1ee1106b3c3718c1b8b8071b17489f7a3cdd722aa0427753e5b3d5a7d09ed0ffe5d500897676fa0e59b4205c57121",
    "/mnt/secure/.keys.45b752d5806159be8d30c1e2f9268be80be54e8f04015943070c686ef7fb6a378b9d1f80036c7ee4dae0dcb6945b9ae929ffb691dd3d5f5d240e3cc0ea4a96f1",
    "/mnt/secure/.keys.925ea013f88e9254f6c8e24efc8aad477f224e72cc770a33e13ba08dc56f31bd0431286c2a18254e8122863ffb04f2ea1516b0c6fb9a0bb90bef588e7bdb310b",
    "/mnt/secure/.keys.3f3ebf63f6ce70e41df392ecc42f6a44fbfc68435fd867fd19821c8ff29e652c47882c571e140f1d1de4f32cb38a76a602504f62374d63d4f262d7a4676de05a",
    "/mnt/secure/.keys.a7b89e19bd752ed375d0ec1b9be8036cb60b20d460b1022789e0bcaefb95f9c2024125fe7d9b639dfedc36679d1c289c9ba131fb99ffcbd93d83eac369a43235"
};

#ifdef __cplusplus
extern "C" {
#endif
        extern int android_check_primary_user_encrypted();
        extern int android_check_user_encrypted(int);
        extern int android_check_for_encrypted_users();
#ifdef __cplusplus
}
#endif
#endif                          /* EFS_INIT_H */
