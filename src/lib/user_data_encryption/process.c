/**
 * @file   process.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 18:42:11 2013
 *
 * @brief
 * API for killing processes that hold a mounted filesystem busy
 * Adapted from AOSP code
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
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <signal.h>
#include <efs/file_utils.h>
#include <cutils/log.h>

int readSymLink(const char *path, char *link, size_t max)
{
    struct stat s;
    int length;

    if (lstat(path, &s) < 0)
        return 0;
    if ((s.st_mode & S_IFMT) != S_IFLNK)
        return 0;

    // we have a symlink
    length = readlink(path, link, max - 1);
    if (length <= 0)
        return 0;
    link[length] = 0;
    return 1;
}

int pathMatchesMountPoint(const char *path, const char *mountPoint)
{
    int length = strlen(mountPoint);
    if (length > 1 && strncmp(path, mountPoint, length) == 0) {
        // we need to do extra checking if mountPoint does not end in a '/'
        if (mountPoint[length - 1] == '/')
            return 1;
        // if mountPoint does not have a trailing slash, we need to make sure
        // there is one in the path to avoid partial matches.
        return (path[length] == 0 || path[length] == '/');
    }

    return 0;
}

void getProcessName(int pid, char *buffer, size_t max)
{
    int fd;
    snprintf(buffer, max, "/proc/%d/cmdline", pid);
    fd = open(buffer, O_RDONLY);
    if (fd < 0) {
        strcpy(buffer, "???");
    } else {
        int length = read(fd, buffer, max - 1);
        buffer[length] = 0;
        close(fd);
    }
}

int checkfileDescriptorSymLinks(int pid, const char *mountPoint,
                char *openFilename, size_t max)
{

    // compute path to process's directory of open files
    char path[PATH_MAX];
    sprintf(path, "/proc/%d/fd", pid);
    DIR *dir = opendir(path);
    if (!dir)
        return 0;

    // remember length of the path
    int parent_length = strlen(path);
    // append a trailing '/'
    path[parent_length++] = '/';

    struct dirent *de;
    while ((de = readdir(dir))) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")
            || strlen(de->d_name) + parent_length + 1 >= PATH_MAX)
            continue;

        // append the file name, after truncating to parent directory
        path[parent_length] = 0;
        strcat(path, de->d_name);

        char link[PATH_MAX];

        if (readSymLink(path, link, sizeof(link))
            && pathMatchesMountPoint(link, mountPoint)) {
            if (openFilename) {
                memset(openFilename, 0, max);
                strncpy(openFilename, link, max - 1);
            }
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

int checkFileDescriptorSymLinks(int pid, const char *mountPoint)
{
    return checkfileDescriptorSymLinks(pid, mountPoint, NULL, 0);
}

int checkfileMaps(int pid, const char *mountPoint, char *openFilename,
          size_t max)
{
    FILE *file;
    char buffer[PATH_MAX + 100];

    sprintf(buffer, "/proc/%d/maps", pid);
    file = fopen(buffer, "r");
    if (!file)
        return 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        // skip to the path
        const char *path = strchr(buffer, '/');
        if (path && pathMatchesMountPoint(path, mountPoint)) {
            if (openFilename) {
                memset(openFilename, 0, max);
                strncpy(openFilename, path, max - 1);
            }
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int checkFileMaps(int pid, const char *mountPoint)
{
    return checkfileMaps(pid, mountPoint, NULL, 0);
}

int checkSymLink(int pid, const char *mountPoint, const char *name)
{
    char path[PATH_MAX];
    char link[PATH_MAX];

    sprintf(path, "/proc/%d/%s", pid, name);
    if (readSymLink(path, link, sizeof(link))
        && pathMatchesMountPoint(link, mountPoint))
        return 1;
    return 0;
}

int getPid(const char *s)
{
    int result = 0;
    while (*s) {
        if (!isdigit(*s))
            return -1;
        result = 10 * result + (*s++ - '0');
    }
    return result;
}

/*
 * Hunt down processes that have files open at the given mount point.
 * action = 0 to just warn,
 * action = 1 to SIGHUP,
 * action = 2 to SIGKILL
 */
// hunt down and kill processes that have files open on the given mount point
void killProcessesWithOpenFiles(const char *path, int action)
{
    DIR *dir;
    struct dirent *de;

    if (!(dir = opendir("/proc"))) {
        SLOGE("opendir failed (%s)", strerror(errno));
        return;
    }

    while ((de = readdir(dir))) {
        int pid = getPid(de->d_name);
        char name[PATH_MAX];

        if (pid == -1)
            continue;
        getProcessName(pid, name, sizeof(name));

        char openfile[PATH_MAX];

        if (checkfileDescriptorSymLinks
            (pid, path, openfile, sizeof(openfile))) {
            SLOGE("Process %s (%d) has open file %s", name, pid,
                  openfile);
        } else if (checkfileMaps(pid, path, openfile, sizeof(openfile))) {
            SLOGE("Process %s (%d) has open filemap for %s", name,
                  pid, openfile);
        } else if (checkSymLink(pid, path, "cwd")) {
            SLOGE("Process %s (%d) has cwd within %s", name, pid,
                  path);
        } else if (checkSymLink(pid, path, "root")) {
            SLOGE("Process %s (%d) has chroot within %s", name, pid,
                  path);
        } else if (checkSymLink(pid, path, "exe")) {
            SLOGE("Process %s (%d) has executable path within %s",
                  name, pid, path);
        } else {
            continue;
        }
        if (action == 1) {
            SLOGW("Sending SIGHUP to process %d", pid);
            kill(pid, SIGTERM);
        } else if (action == 2) {
            SLOGE("Sending SIGKILL to process %d", pid);
            kill(pid, SIGKILL);
        }
    }
    closedir(dir);
}
