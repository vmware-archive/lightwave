/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <shadow.h>
#include <errno.h>

/* Needed for SLES11, not sure about other UNIX platforms */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <crypt.h>
#include <stdio.h>

#define CRYPT_MD5         "$1$"
#define CRYPT_BLOWFISH_2A "$2a$"
#define CRYPT_BLOWFISH_2B "$2b$"
#define CRYPT_BLOWFISH_2X "$2x$"
#define CRYPT_BLOWFISH_2Y "$2y$"
#define CRYPT_SHA_256     "$5$"
#define CRYPT_SHA_512     "$6$"

#ifdef _WIN32
int get_sp_salt(const char *username,
                char **ret_salt,
                char **ret_encpwd)
{
    /*
     * This cannot be supported on Windows, as there is no
     * /etc shadow password file. The equivalent "local provider"
     * hash is not accessable, making this functionality impossible to 
     * support on Win32 platforms.
     */
    return ERROR_NOT_SUPPORTED;
}
#else
/*
 * This function looks up "username" in the shadow password file, determines
 * the hash algorithm type, and returns the salt and the password
 * hash for that user.
 * 
 * Given the salt and the user password, then the hash can be created.
 * The generated hash is used as an SRP password (client side), and
 * the generator for the SRP secret (server side).
 *
 * Crypt password file format references:
 * http://php.net/manual/en/function.crypt.php
 * http://en.wikipedia.org/wiki/Crypt_%28C%29#Blowfish-based_scheme
 *
 * Look up from the shadow password file the specified user, and if found,
 * return the salt field parsed out from the hash entry
 *
 * Algorithm ID
 * $1$  MD5
 * 12 characters salt follows
 *
 * $2a$ Blowfish
 * $2b$ Blowfish
 * $2x$ Blowfish
 * $2y$ Blowfish
 * Blowfish salt format:
 * $id$NN$-----22 chars-salt----++++++hash+++++:
 *
 * SHA salt format
 * $5$  SHA-256
 * $6$  SHA-512
 * $ID$salt$hash
 */
int get_sp_salt(const char *username,
                char **ret_salt,
                char **ret_encpwd)
{
    int st = 0;
    int is_locked = 0;
    struct spwd *spval = NULL;
    struct spwd spval_buf = {0};
    char *spbuf_str = NULL;
    int spbuf_str_len = 256;
    int salt_len = 0;
    char *salt = NULL;
    char *encpwd = NULL;
    char *sp = NULL;
    int cur_uid = 0;
    
    if (!username || !ret_salt || !ret_encpwd)
    {
        st = -1;
        errno = EINVAL;
        goto error;
    }

    /* Must be root to read shadow password file */
    cur_uid = getuid();
    seteuid(0);

    /* Obtain password file lock, and hold minimum amount of time */
    st = lckpwdf();
    if (st == -1)
    {
        goto error;
    }
    is_locked = 1;

    spbuf_str = calloc(spbuf_str_len, sizeof(char));
    if (!spbuf_str)
    {
        st = -1;
        goto error;
    }
    st = getspnam_r(username,
                    &spval_buf,
                    spbuf_str,
                    spbuf_str_len,
                    &spval);

    if (!spval || st == -1)
    {
        /* Failed due to permissions or entry not found */
        st = -1;
        goto error;
    }
    salt = strdup(spval->sp_pwdp);
    if (!salt)
    {
        /* errno is set */
        st = -1;
        goto error;
    }
    encpwd = strdup(spval->sp_pwdp);
    if (!encpwd)
    {
        /* errno is set */
        st = -1;
        goto error;
    }
    ulckpwdf();
    seteuid(cur_uid);
    is_locked = 0;
   
    /* CRYPT_DES hash is not supported; how to test? */

    /* Determine the hash algorithn, and therefore the salt length */
    if (!strncmp(salt, CRYPT_MD5, strlen(CRYPT_MD5)))
    {
        /* $1$123456789012 */
        salt_len = 12 + 3;
    }
    else if (!strncmp(salt, CRYPT_BLOWFISH_2A, strlen(CRYPT_BLOWFISH_2A)) ||
             !strncmp(salt, CRYPT_BLOWFISH_2B, strlen(CRYPT_BLOWFISH_2B)) ||
             !strncmp(salt, CRYPT_BLOWFISH_2X, strlen(CRYPT_BLOWFISH_2X)) ||
             !strncmp(salt, CRYPT_BLOWFISH_2Y, strlen(CRYPT_BLOWFISH_2Y)))
    {
        /* $2a$05$1234567890123456789012 */
        salt_len = 22 + 7;
    }
    else if (!strncmp(salt, CRYPT_SHA_256, strlen(CRYPT_SHA_256)) ||
             !strncmp(salt, CRYPT_SHA_512, strlen(CRYPT_SHA_512)))
    {
        sp = strrchr(salt, '$');
        salt_len = sp - salt + 1;
    }
    salt[salt_len] = '\0';
    *ret_salt = salt;
    *ret_encpwd = encpwd;
    salt = NULL;

error:
    if (is_locked)
    {
        ulckpwdf();
        seteuid(cur_uid);
    }
    if (spbuf_str)
    {
        free(spbuf_str);
    }
    if (st == -1)
    {
        if (salt)
        {
            free(salt);
            salt = NULL;
        }
        if (encpwd)
        {
            free(encpwd);
            encpwd = NULL;
        }
    }
    return st;
}
#endif
