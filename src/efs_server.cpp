/**
 * Licenses and stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define LOG_TAG "EFSNativeService"

#include "cutils/klog.h"
#include "cutils/log.h"

#include "efs/CommandListener.h"

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
