/**
 * @file   EncryptedFileStorageCmd.cpp
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Wed Oct 16 12:25:40 2013
 *
 * @brief
 * Implementations of EFS commands in vold service
 * This file is directly included in the vold source code
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

#include <efs/efs.h>
#include "EncryptedFileStorageCmd.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/log.h>
#include <sysutils/SocketClient.h>
#include <private/android_filesystem_config.h>
#include "ResponseCode.h"

#include "CommandListener.h"

EncryptedFileStorageCmd::EncryptedFileStorageCmd():
                 VoldCommand("efs") {
}

int EncryptedFileStorageCmd::runCommand(SocketClient *cli, int argc, char **argv) {
    int rc = 0;

    if ((cli->getUid() != 0) && (cli->getUid() != AID_SYSTEM)) {
        cli->sendMsg(ResponseCode::CommandNoPermission, "No permission to run EFS commands", false);
        return 0;
    }

    if (argc < 3) {
        cli->sendMsg(ResponseCode::CommandSyntaxError, "Missing Argument", false);
        return 0;
    }

    if (!strcmp(argv[1], "create")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs create <storage_path> <passwd>", false);
            return 0;
        }
        rc = EFS_create(argv[2],argv[3]);
    } else if (!strcmp(argv[1], "unlock")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs unlock <storage_path> <passwd>", false);
            return 0;
        }
        rc = EFS_unlock(argv[2], argv[3]);
    } else if (!strcmp(argv[1], "lock")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs lock <storage_path>", false);
            return 0;
        }
        rc = EFS_lock(argv[2]);
    } else if (!strcmp(argv[1], "change_passwd")) {
        if (argc != 5) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs change_passwd <storage_path> <old_passwd> <new_passwd>", false);
            return 0;
        }
        rc = EFS_change_password(argv[2],argv[3],argv[4]);
    } else if (!strcmp(argv[1], "remove")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs remove <storage_path>", false);
            return 0;
        }
        rc = EFS_remove(argv[2]);
    } else if (!strcmp(argv[1], "recover")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs recover <storage_path> <password>", false);
            return 0;
        }
        rc = EFS_recover_data_and_remove(argv[2], argv[3]);
    } else if (!strcmp(argv[1], "stat")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs stat <storage_path>", false);
            return 0;
        }
        rc = EFS_get_status(argv[2]);
    } else if (!strcmp(argv[1], "encrypt_user_data")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs encrypt_user_data <userId> <password>", false);
            return 0;
        }
        rc = android_encrypt_user_data(atoi(argv[2]), argv[3]);
    } else if (!strcmp(argv[1], "unlock_user_data")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs unlock_user_data <userId> <password>", false);
            return 0;
        }
        rc = android_unlock_user_data(atoi(argv[2]), argv[3]);
    }  else if (!strcmp(argv[1], "lock_user_data")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs lock_user_data <userId>", false);
            return 0;
        }
        rc = android_lock_user_data(atoi(argv[2]));
    } else if (!strcmp(argv[1], "change_user_data_passwd")) {
        if (argc != 5) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs change_user_data_passwd <userId> <old_passwd> <new_passwd>", false);
            return 0;
        }
        rc = android_change_user_data_password(atoi(argv[2]),argv[3],argv[4]);
    } else if (!strcmp(argv[1], "decrypt_user_data")) {
        if (argc != 4) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs decrypt_user_data <userId> <password>", false);
            return 0;
        }
        rc = android_decrypt_user_data(atoi(argv[2]), argv[3]);
    }  else if (!strcmp(argv[1], "remove_user_encrypted_data")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs remove_user_encrypted_data <userId>", false);
            return 0;
        }
        rc = android_remove_user_encrypted_data(atoi(argv[2]));
    }   else if (!strcmp(argv[1], "user_stat")) {
        if (argc != 3) {
            cli->sendMsg(ResponseCode::CommandSyntaxError, "Usage: efs user_stat <userId>", false);
            return 0;
        }
        rc = android_get_encrypted_user_status(atoi(argv[2]));
    }
    else {
        cli->sendMsg(ResponseCode::CommandSyntaxError, "Unknown efs-tools cmd", false);
    }

    // Always report that the command succeeded and return the error code.
    // The caller will check the return value to see what the error was.
    char msg[255];
    snprintf(msg, sizeof(msg), "%d", rc);
    cli->sendMsg(ResponseCode::CommandOkay, msg, false);

    return 0;
}

