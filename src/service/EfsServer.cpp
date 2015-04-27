/**
 * @file   mount_utils.c
 * @author Catalin Ionita <catalin.ionita>
 * @date   Mon Oct 14 18:13:30 2013
 *
 * @brief
 * API for managing ecryptfs mounts
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "cutils/klog.h"
#include "cutils/log.h"
#include "efs/CommandListener.h"

#define LOG_TAG "EFSNativeService"

int main() {

    CommandListener *cl;

    SLOGI("EFSNativeService starting");

    cl = new CommandListener();

    if (cl->startListener()) {
        SLOGE("Unable to start CommandListener (%s)", strerror(errno));
        exit(1);
    }

    while(1) {
        sleep(1000);
    }

    SLOGI("EFSNativeService exiting");
    exit(0);
}
