/* keyutil.c: key utility library
 *
 * Copyright (C) 2005 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <asm/unistd.h>
#include "keyutil.h"

#ifdef NO_GLIBC_KEYERR
static int error_inited;
static void (*libc_perror)(const char *msg);
static char *(*libc_strerror_r)(int errnum, char *buf, size_t n);
//static int (*libc_xpg_strerror_r)(int errnum, char *buf, size_t n);
#define RTLD_NEXT      ((void *) -1L)
#endif

#define __weak __attribute__((weak))

key_serial_t __weak add_key(const char *type,
                            const char *description,
                            const void *payload,
                            size_t plen,
                            key_serial_t ringid)
{
  return syscall(__NR_add_key,
		 type, description, payload, plen, ringid);
}

key_serial_t __weak request_key(const char *type,
                                const char *description,
                                const char * callout_info,
                                key_serial_t destringid)
{
  return syscall(__NR_request_key,
		 type, description, callout_info, destringid);
}

static inline long __keyctl(int cmd,
                            unsigned long arg2,
                            unsigned long arg3,
                            unsigned long arg4,
                            unsigned long arg5)
{
  return syscall(__NR_keyctl,
		 cmd, arg2, arg3, arg4, arg5);
}

long __weak keyctl(int cmd, ...)
{
  va_list va;
  unsigned long arg2, arg3, arg4, arg5;

  va_start(va, cmd);
  arg2 = va_arg(va, unsigned long);
  arg3 = va_arg(va, unsigned long);
  arg4 = va_arg(va, unsigned long);
  arg5 = va_arg(va, unsigned long);
  va_end(va);

  return __keyctl(cmd, arg2, arg3, arg4, arg5);
}

key_serial_t keyctl_get_keyring_ID(key_serial_t id, int create)
{
  return keyctl(KEYCTL_GET_KEYRING_ID, id, create);
}

key_serial_t keyctl_join_session_keyring(const char *name)
{
  return keyctl(KEYCTL_JOIN_SESSION_KEYRING, name);
}

long keyctl_update(key_serial_t id, const void *payload, size_t plen)
{
  return keyctl(KEYCTL_UPDATE, id, payload, plen);
}

long keyctl_revoke(key_serial_t id)
{
  return keyctl(KEYCTL_REVOKE, id);
}

long keyctl_chown(key_serial_t id, uid_t uid, gid_t gid)
{
  return keyctl(KEYCTL_CHOWN, id, uid, gid);
}

long keyctl_setperm(key_serial_t id, key_perm_t perm)
{
  return keyctl(KEYCTL_SETPERM, id, perm);
}

long keyctl_describe(key_serial_t id, char *buffer, size_t buflen)
{
  return keyctl(KEYCTL_DESCRIBE, id, buffer, buflen);
}

long keyctl_clear(key_serial_t ringid)
{
  return keyctl(KEYCTL_CLEAR, ringid);
}

long keyctl_link(key_serial_t id, key_serial_t ringid)
{
  return keyctl(KEYCTL_LINK, id, ringid);
}

long keyctl_unlink(key_serial_t id, key_serial_t ringid)
{
  return keyctl(KEYCTL_UNLINK, id, ringid);
}

long keyctl_search(key_serial_t ringid,
                   const char *type,
                   const char *description,
                   key_serial_t destringid)
{
  return keyctl(KEYCTL_SEARCH, ringid, type, description, destringid);
}

long keyctl_read(key_serial_t id, char *buffer, size_t buflen)
{
  return keyctl(KEYCTL_READ, id, buffer, buflen);
}

long keyctl_instantiate(key_serial_t id,
                        const void *payload,
                        size_t plen,
                        key_serial_t ringid)
{
  return keyctl(KEYCTL_INSTANTIATE, id, payload, plen, ringid);
}

long keyctl_negate(key_serial_t id, unsigned timeout, key_serial_t ringid)
{
  return keyctl(KEYCTL_NEGATE, id, timeout, ringid);
}

long keyctl_set_reqkey_keyring(int reqkey_defl)
{
  return keyctl(KEYCTL_SET_REQKEY_KEYRING, reqkey_defl);
}

long keyctl_set_timeout(key_serial_t id, unsigned timeout)
{
  return keyctl(KEYCTL_SET_TIMEOUT, id, timeout);
}

long keyctl_assume_authority(key_serial_t id)
{
  return keyctl(KEYCTL_ASSUME_AUTHORITY, id);
}

/*****************************************************************************/
/*
 * fetch key description into an allocated buffer
 * - resulting string is NUL terminated
 * - returns count not including NUL
 */
int keyctl_describe_alloc(key_serial_t id, char **_buffer)
{
  char *buf;
  long buflen, ret;

  ret = keyctl_describe(id, NULL, 0);
  if (ret < 0)
    return -1;

  buflen = ret;
  buf = malloc(buflen);
  if (!buf)
    return -1;

  for (;;) {
    ret = keyctl_describe(id, buf, buflen);
    if (ret < 0)
      return -1;

    if (buflen >= ret)
      break;

    buflen = ret;
    buf = realloc(buf, buflen);
    if (!buf)
      return -1;
  }

  *_buffer = buf;
  return buflen - 1;

} /* end keyctl_describe_alloc() */

/*****************************************************************************/
/*
 * fetch key contents into an allocated buffer
 * - resulting buffer has an extra NUL added to the end
 * - returns count (not including extraneous NUL)
 */
int keyctl_read_alloc(key_serial_t id, void **_buffer)
{
  void *buf;
  long buflen, ret;

  ret = keyctl_read(id, NULL, 0);
  if (ret < 0)
    return -1;

  buflen = ret;
  buf = malloc(buflen + 1);
  if (!buf)
    return -1;

  for (;;) {
    ret = keyctl_read(id, buf, buflen);
    if (ret < 0)
      return -1;

    if (buflen >= ret)
      break;

    buflen = ret;
    buf = realloc(buf, buflen + 1);
    if (!buf)
      return -1;
  }

  ((unsigned char *) buf)[buflen] = 0;
  *_buffer = buf;
  return buflen;

} /* end keyctl_read_alloc() */

#ifdef NO_GLIBC_KEYERR
/*****************************************************************************/
/*
 * initialise error handling
 */
static void error_init(void)
{
  char *err;

  error_inited = 1;

  dlerror();

  libc_perror = dlsym(RTLD_NEXT,"perror");
  if (!libc_perror) {
    fprintf(stderr, "Failed to look up next perror\n");
    err = dlerror();
    if (err)
      fprintf(stderr, "%s\n", err);
    abort();
  }

  //fprintf(stderr, "next perror at %p\n", libc_perror);

  libc_strerror_r = dlsym(RTLD_NEXT,"strerror_r");
  if (!libc_strerror_r) {
    fprintf(stderr, "Failed to look up next strerror_r\n");
    err = dlerror();
    if (err)
      fprintf(stderr, "%s\n", err);
    abort();
  }

  //fprintf(stderr, "next strerror_r at %p\n", libc_strerror_r);

#if 0
  libc_xpg_strerror_r = dlsym(RTLD_NEXT,"xpg_strerror_r");
  if (!libc_xpg_strerror_r) {
    fprintf(stderr, "Failed to look up next xpg_strerror_r\n");
    err = dlerror();
    if (err)
      fprintf(stderr, "%s\n", err);
    abort();
  }

  //fprintf(stderr, "next xpg_strerror_r at %p\n", libc_xpg_strerror_r);
#endif

} /* end error_init() */

/*****************************************************************************/
/*
 * overload glibc's strerror_r() with a version that knows about key errors
 */
char *strerror_r(int errnum, char *buf, size_t n)
{
  const char *errstr;
  int len;

  printf("hello\n");

  if (!error_inited)
    error_init();

  switch (errnum) {
  case ENOKEY:
    errstr = "Requested key not available";
    break;

  case EKEYEXPIRED:
    errstr = "Key has expired";
    break;

  case EKEYREVOKED:
    errstr = "Key has been revoked";
    break;

  case EKEYREJECTED:
    errstr = "Key was rejected by service";
    break;

  default:
    return libc_strerror_r(errnum, buf, n);
  }

  len = strlen(errstr) + 1;
  if (n > len) {
    errno = ERANGE;
    if (n > 0) {
      memcpy(buf, errstr, n - 1);
      buf[n - 1] = 0;
    }
    return NULL;
  }
  else {
    memcpy(buf, errstr, len);
    return buf;
  }

} /* end strerror_r() */

#if 0
/*****************************************************************************/
/*
 * overload glibc's strerror_r() with a version that knows about key errors
 */
int xpg_strerror_r(int errnum, char *buf, size_t n)
{
  const char *errstr;
  int len;

  if (!error_inited)
    error_init();

  switch (errnum) {
  case ENOKEY:
    errstr = "Requested key not available";
    break;

  case EKEYEXPIRED:
    errstr = "Key has expired";
    break;

  case EKEYREVOKED:
    errstr = "Key has been revoked";
    break;

  case EKEYREJECTED:
    errstr = "Key was rejected by service";
    break;

  default:
    return libc_xpg_strerror_r(errnum, buf, n);
  }

  len = strlen(errstr) + 1;
  if (n > len) {
    errno = ERANGE;
    if (n > 0) {
      memcpy(buf, errstr, n - 1);
      buf[n - 1] = 0;
    }
    return -1;
  }
  else {
    memcpy(buf, errstr, len);
    return 0;
  }

} /* end xpg_strerror_r() */
#endif

/*****************************************************************************/
/*
 *
 */
void perror(const char *msg)
{
  if (!error_inited)
    error_init();

  switch (errno) {
  case ENOKEY:
    fprintf(stderr, "%s: Requested key not available\n", msg);
    return;

  case EKEYEXPIRED:
    fprintf(stderr, "%s: Key has expired\n", msg);
    return;

  case EKEYREVOKED:
    fprintf(stderr, "%s: Key has been revoked\n", msg);
    return;

  case EKEYREJECTED:
    fprintf(stderr, "%s: Key was rejected by service\n", msg);
    return;

  default:
    libc_perror(msg);
    return;
  }

} /* end perror() */
#endif
