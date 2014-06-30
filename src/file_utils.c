/**
 * @file   file_utils.c
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Mon Oct 14 18:47:36 2013
 *
 * @brief
 * Utilities to copy, move and remove a file/directory
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include <fcntl.h>
#include <dirent.h>
#include <selinux/selinux.h>
#include <efs/efs.h>
#include <efs/file_utils.h>
#include <efs/key_chain.h>
#include <cutils/properties.h>

typedef struct file_info file_info;
struct file_info {
    struct stat st;
    security_context_t con;
    file_info *next;
    char *path;
};

/**
 * Check whether there is enough space on disk to create storage
 *
 * @param path Path to storage
 *
 * @return 1 if enough space exists, 0 otherwise
 */
int check_space(const char *path)
{
    struct statvfs stat;
    off_t size;
    int ret;

    ret = get_dir_size(path, &size);
    if (ret < 0) {
        LOGE("Failed to compute storage size %s", path);
        return ret;
    }

    ret = statvfs(path, &stat);
    if (ret < 0) {
        LOGE("Failed to compute free space for storage %s", path);
        return ret;
    }

    if ((unsigned)size < stat.f_bsize * stat.f_bfree)
        return 1;

    return 0;
}

/**
 * Create a file node to hold informations about a file
 *
 * @param path File path
 * @param st File stat
 *
 * @return A pointer to the new file node
 */
static file_info *create_node(const char *path, struct stat *st,
                security_context_t con)
{
    int len = strlen(path);

    file_info *fi = malloc(sizeof(file_info) + len + 1);
    if (fi == 0) {
        LOGE("insufficient memory\n");
        exit(EXIT_FAILURE);;
    }

    memset(fi, 0, sizeof(file_info) + len + 1);
    memcpy(&fi->st, st, sizeof(struct stat));
    fi->con = con;
    fi->next = NULL;
    fi->path = (char *)(fi + 1);
    strncpy(fi->path, path, len);

    return fi;
}

/**
 * Generate all files from a Directory path
 * Similar functionality with ls -R
 * @param path Directory path
 * @param file_list List of files
 * @param dir_list  List of directories
 *
 * @return 0 on success, negative number in case of an error
 */
static int generate_file_list(const char *path, file_info ** file_list,
                  file_info ** dir_list)
{
    DIR *dir;
    struct dirent *dirent;
    struct stat st;
    security_context_t con;
    int len;
    char *file_path;
    file_info *node;
    int ret = -1;

    len = strlen(path);
    if (len > MAX_PATH_LENGTH) {
        LOGE("Invalid argument %s\n", path);
        return -1;
    }

    dir = opendir(path);
    if (!dir) {
        LOGE("open dir %s failed", path);
        return -1;
    }

    while ((dirent = readdir(dir))) {
        int file_len = strlen(dirent->d_name);

        /* This should not happen */
        if (file_len > MAX_FILE_LENGTH) {
            LOGE("file system error\n");
            closedir(dir);
            return -1;
        }
        /* Ignore /. and /.. */
        if (file_len == 1 || file_len == 2) {
            if (dirent->d_name[0] == '.') {
                if (dirent->d_name[1] == '\0')
                    continue;
                if ((dirent->d_name[1] == '.')
                    && (dirent->d_name[2] == '\0'))
                    continue;
            }
        }

        file_path =
            (char *)malloc(MAX_PATH_LENGTH + MAX_FILE_LENGTH + 1);
        if (!file_path) {
            LOGE("insufficient memory\n");
            exit(EXIT_FAILURE);
        }
        memset(file_path, 0, MAX_PATH_LENGTH + MAX_FILE_LENGTH + 1);
        strcpy(file_path, path);
        *(file_path + len) = '/';
        strcpy(file_path + len + 1, dirent->d_name);

        ret = lstat(file_path, &st);
        if (ret < 0) {
            LOGE("lstat failed on %s\n", file_path);
            closedir(dir);
            return ret;
        }

        ret = lgetfilecon(file_path, &con);
        if (ret < 0) {
            LOGE("lgetfilecon failed on %s\n", file_path);
            closedir(dir);
            return ret;
        }

        node = create_node(file_path, &st, con);
        if (dirent->d_type == DT_DIR) {
            node->next = *dir_list;
            *dir_list = node;
            ret =
                generate_file_list(file_path, file_list, dir_list);
            if (ret < 0) {
                free(file_path);
                return ret;
            }
        } else {
            node->next = *file_list;
            *file_list = node;
        }

        free(file_path);
    }

    closedir(dir);
    return 0;
}

/**
 * Get size on disk for a directory
 * Similar functionality with du -c
 * @param path Directory path
 * @param size Directory size
 *
 * @return 0 for success, negative value in case of an error
 */
int get_dir_size(const char *path, off_t * size)
{
    DIR *dir;
    struct dirent *dirent;
    struct stat st;
    int len, ret = -1;
    char *file_path;

    len = strlen(path);
    if (len > MAX_PATH_LENGTH) {
        LOGE("Invalid argument %s\n", path);
        return -1;
    }

    dir = opendir(path);
    if (!dir) {
        LOGE("open dir %s failed", path);
        return -1;
    }

    while ((dirent = readdir(dir))) {
        int file_len = strlen(dirent->d_name);

        /* This should not happen */
        if (file_len > MAX_FILE_LENGTH) {
            LOGE("file system error\n");
            closedir(dir);
            return -1;
        }

        /* ignore /. and /.. */
        if (file_len == 1 || file_len == 2) {
            if (dirent->d_name[0] == '.') {
                if (dirent->d_name[1] == '\0')
                    continue;
                if ((dirent->d_name[1] == '.')
                    && (dirent->d_name[2] == '\0'))
                    continue;
            }
        }

        file_path =
            (char *)malloc(MAX_PATH_LENGTH + MAX_FILE_LENGTH + 1);
        if (!file_path)
            return -1;
        memset(file_path, 0, MAX_PATH_LENGTH + MAX_FILE_LENGTH + 1);
        strcpy(file_path, path);
        *(file_path + len) = '/';
        strcpy(file_path + len + 1, dirent->d_name);

        ret = lstat(file_path, &st);
        if (ret < 0) {
            LOGE("lstat failed on %s\n", file_path);
            closedir(dir);
            free(file_path);
            return ret;
        }

        *size += st.st_size;

        if (dirent->d_type == DT_DIR) {
            ret = get_dir_size(file_path, size);
            if (ret < 0) {
                free(file_path);
                return ret;
            }
        }

        free(file_path);
    }

    closedir(dir);
    return 0;
}

/**
 * Create directory infrastructure and store it on disk
 * Used in context with copy file or copy directory
 * @param dir_list Linked list with directories names and attributes
 * @param src_path Source path
 * @param dst_path Destination path
 *
 * @return 0 on success, negative value on error
 */
static int create_dirs(file_info ** dir_list, char *src_path, char *dst_path)
{
    file_info *iter = *dir_list;
    char *path;
    int len;
    int ret = -1;
    struct utimbuf time;

    if (!iter)
        return 0;

    len = strlen(dst_path) + strlen(iter->path) - strlen(src_path) + 1;
    if (len > MAX_PATH_LENGTH || strlen(dst_path) > MAX_PATH_LENGTH
        || strlen(src_path) > MAX_PATH_LENGTH) {
        LOGE("Can't copy %s", iter->path);
        return -1;
    }

    path = (char *)malloc(MAX_PATH_LENGTH + 1);
    if (!path) {
        LOGE("insufficient memory\n");
        exit(EXIT_FAILURE);
    }

    memset(path, 0, MAX_PATH_LENGTH + 1);
    strcpy(path, dst_path);
    *(path + strlen(dst_path)) = '/';
    strcpy(path + strlen(dst_path) + 1, iter->path + strlen(src_path));

    /* The "list" is a stack; recurency will restore FIFO order */
    if (!iter->next) {
        ret = mkdir(path, iter->st.st_mode);
        if (ret < 0) {
            LOGE("mkdir %s fail\n", path);
            free(path);
            return ret;
        }
        ret = setfilecon(path, iter->con);
        if (ret < 0) {
            LOGE("setfilecon %s fail\n", path);
            free(path);
            return ret;
        }
        ret = chown(path, iter->st.st_uid, iter->st.st_gid);
        if (ret < 0) {
            LOGE("chown %s fail\n", path);
            free(path);
            return ret;
        }
        time.actime = iter->st.st_atime;
        time.modtime = iter->st.st_mtime;
        ret = utime(path, &time);
        if (ret != 0) {
            LOGE("utime on %s failed", path);
            free(path);
            return -1;
        }
        return 0;
    }

    ret = create_dirs(&iter->next, src_path, dst_path);
    if (ret < 0) {
        free(path);
        return ret;
    };

    ret = mkdir(path, iter->st.st_mode);
    if (ret < 0) {
        LOGE("mkdir %s fail", path);
        free(path);
        return ret;
    }

    ret = setfilecon(path, iter->con);
    if (ret < 0) {
        LOGE("setfilecon %s fail\n", path);
        free(path);
        return ret;
    }

    ret = chown(path, iter->st.st_uid, iter->st.st_gid);
    if (ret < 0) {
        LOGE("chown %s fail\n", path);
        free(path);
        return ret;
    }

    time.actime = iter->st.st_atime;
    time.modtime = iter->st.st_mtime;
    ret = utime(path, &time);
    if (ret != 0) {
        LOGE("utime on %s failed", path);
        free(path);
        return -1;
    }

    free(path);
    return 0;
}

/**
 * Remove all files sprecified by a file_info list
 *
 * @param file_list List with informations about files
 *
 * @return 0 on success, negative value in case of an error
 */
static int delete_files(file_info ** file_list)
{
    file_info *iter = *file_list;
    int ret = 0;

    while (iter) {
        if (remove(iter->path) < 0) {
            LOGE("unable to remove %s\n", iter->path);
            ret = -1;
        }
        iter = iter->next;
    }

    return ret;
}

/**
 * Copy a file
 *
 * @param src_path Source
 * @param dst_path Destination
 * @param st File attributes
 *
 * @return 0 for success, negative value in case of an error
 */
static int copy_file(char *src_path, char *dst_path, struct stat *st,
                security_context_t con)
{
    char buffer[MAX_PATH_LENGTH], buff[PROPERTY_VALUE_MAX];
    int n = 0, m = 0, ret = -1;
    int fd_src, fd_dst;
    struct utimbuf time;
    off_t size = 0;

    fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        LOGE("open %s failed\n", src_path);
        return fd_src;
    }

    fd_dst = open(dst_path, O_CREAT | O_WRONLY, st->st_mode);
    if (fd_dst < 0) {
        LOGE("open %s failed\n", dst_path);
        close(fd_src);
        return fd_dst;
    }

    do {
        n = read(fd_src, buffer, sizeof(buffer));
        if (n < 0) {
            LOGE("Error reading from file\n");
            close(fd_src);
            close(fd_dst);
            return n;
        }
        if (n == 0)
            break;
        m = write(fd_dst, buffer, n);
        if (m != n) {
            close(fd_src);
            close(fd_dst);
            return -1;
        }

        memset(buff, 0, sizeof(buff));
        property_get("efs.encrypt.progress", buff, "0");
        size = atoi(buff);
        size += m;
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "%d", size);
        property_set("efs.encrypt.progress", buff);

    } while (n);

    close(fd_src);
    close(fd_dst);

    ret = setfilecon(dst_path, con);
    if (ret < 0) {
        LOGE("setfilecon %s fail\n", dst_path);
        return ret;
    }

    ret = chown(dst_path, st->st_uid, st->st_gid);
    if (ret < 0) {
        LOGE("chown %s fail\n", dst_path);
        return ret;
    }

    /* Save access times */
    time.actime = st->st_atime;
    time.modtime = st->st_mtime;
    ret = utime(dst_path, &time);
    if (ret != 0) {
        LOGE("utime on %s failed", dst_path);
        return -1;
    }

    return 0;
}

/**
 * Copy multiple files from source to destination
 *
 * @param file_list File list
 * @param src_path Source
 * @param dst_path Destination
 *
 * @return 0 on success, negative value if an error occurs
 */
static int copy_files(file_info ** file_list, char *src_path, char *dst_path)
{
    file_info *iter = *file_list;
    char path[MAX_PATH_LENGTH + 1], *linkname;
    int len = 0, ret = -1;

    if (strlen(src_path) > MAX_PATH_LENGTH
        || strlen(dst_path) > MAX_PATH_LENGTH) {
        LOGE("Invalid arguments\n");
        return ret;
    }

    while (iter) {
        len =
            strlen(dst_path) + strlen(iter->path) - strlen(src_path) +
            1;
        if (len > MAX_PATH_LENGTH) {
            LOGE("Invalig len\n");
            return -1;
        }

        strcpy(path, dst_path);
        path[strlen(dst_path)] = '/';
        strcpy(path + strlen(dst_path) + 1,
               iter->path + strlen(src_path));

        if (S_ISLNK(iter->st.st_mode)) {
            linkname = malloc(iter->st.st_size + 1);
            if (linkname == NULL) {
                LOGE("insufficient memory\n");
                exit(EXIT_FAILURE);
            }
            ret =
                readlink(iter->path, linkname,
                     iter->st.st_size + 1);
            if (ret < 0) {
                free(linkname);
                LOGE("lstat failed\n");
                return -1;
            }
            linkname[iter->st.st_size] = '\0';
            ret = symlink(linkname, path);
            if (ret < 0) {
                free(linkname);
                LOGE("can't create symlink %s", path);
                return ret;
            }

            ret = lsetfilecon(path, iter->con);
            if (ret < 0) {
                LOGE("lsetfilecon %s fail\n", iter->path);
                free(linkname);
                return ret;
            }

            ret = lchown(path, iter->st.st_uid, iter->st.st_gid);
            if (ret < 0) {
                LOGE("lchown %s fail\n", iter->path);
                free(linkname);
                return ret;
            }
            free(linkname);
            iter = iter->next;
            continue;
        }

        ret = copy_file(iter->path, path, &iter->st, iter->con);
        if (ret < 0) {
            LOGE("Copying file form %s to %s failed\n", iter->path,
                 path);
            return ret;
        }

        iter = iter->next;
    }
    return 0;
}

/**
 * Free linked file list
 *
 * @param file_list File list
 */
static void free_list(file_info ** file_list)
{
    file_info *iter = *file_list;
    file_info *aux;

    while (iter) {
        freecon(iter->con);
        aux = iter;
        iter = iter->next;
        free(aux);
    }

}

/**
 * Copy content of a directory
 *
 * @param dst_path Destination path
 * @param src_path Source path
 *
 * @return 0 on success, negative value in case of an error
 */
int copy_dir_content(char *dst_path, char *src_path)
{
    file_info *file_list = 0, *dir_list = 0;
    int ret = -1;

    ret = generate_file_list(src_path, &file_list, &dir_list);
    if (ret < 0) {
        LOGE("generate_file_list for %s failed\n", src_path);
        goto err;
    }

    ret = create_dirs(&dir_list, src_path, dst_path);
    if (ret < 0) {
        LOGE("create_dirs for %s failed\n", src_path);
        goto err;
    }

    ret = copy_files(&file_list, src_path, dst_path);
    if (ret < 0) {
        LOGE("copy_files for %s failed\n", src_path);
        goto err;
    }

    free_list(&file_list);
    free_list(&dir_list);
    return 0;

err:
    free_list(&file_list);
    free_list(&dir_list);
    return ret;
}

/**
 * Remove files from directory
 *
 * @param path Directory path
 *
 * @return 0 on success, negative value in case of an error
 */
int remove_dir_content(char *path)
{
    file_info *file_list = 0, *dir_list = 0;
    int ret = -1;

    ret = generate_file_list(path, &file_list, &dir_list);
    if (ret < 0) {
        LOGE("generate_file_list for %s failed\n", path);
        goto err;
    }

    ret = delete_files(&file_list);
    if (ret < 0) {
        LOGE("delete_files %s failed\n", path);
        goto err;
    }

    ret = delete_files(&dir_list);
    if (ret < 0) {
        LOGE("delete dirs failed %s\n", path);
        goto err;
    }

    free_list(&file_list);
    free_list(&dir_list);

    return 0;

err:
    free_list(&file_list);
    free_list(&dir_list);
    return ret;
}

/**
 * Remove a directory
 *
 * @param path Directory path
 *
 * @return 0 for success, negative value in case of an error
 */
int remove_dir(char *path)
{
    int ret = -1;

    ret = remove_dir_content(path);
    if (ret < 0) {
        LOGE("unable to remove dir content %s\n", path);
        return ret;
    }

    ret = remove(path);
    if (ret < 0) {
        LOGE("unable to remove root directory %s\n", path);
        return ret;
    }

    return 0;
}

/* Debug function */
static void print_nodes(file_info ** list)
{
    file_info *iter = *list;
    while (iter) {
        LOGE("%s\n", iter->path);
        iter = iter->next;
    }
}
