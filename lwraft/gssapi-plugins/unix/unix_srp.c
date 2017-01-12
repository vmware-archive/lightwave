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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <crypt.h>
#include <stdio.h>

#include <csrp/srp.h>

static SRP_HashAlgorithm G_alg     = SRP_SHA1;
static SRP_NGType        G_ng_type = SRP_NG_2048;
static const char        *G_n_hex  = 0;
static const char        *G_g_hex  = 0;


#define CRYPT_MD5         "$1$"
#define CRYPT_BLOWFISH_2A "$2a$"
#define CRYPT_BLOWFISH_2B "$2b$"
#define CRYPT_BLOWFISH_2X "$2x$"
#define CRYPT_BLOWFISH_2Y "$2y$"
#define CRYPT_SHA_256     "$5$"
#define CRYPT_SHA_512     "$6$"

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

    spval = getspnam(username);
    if (!spval)
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
            salt = NULL;
        }
    }
    return st;
}



/* Create the temporary SRP secret using username shadow pwd entry */
int
srpVerifierInit(
    char *username,
    char *password,
    unsigned char **ret_bytes_s,
    int *ret_len_s,
    unsigned char **ret_bytes_v,
    int *ret_len_v)
{
    int sts = 0;
    const unsigned char *bytes_s = NULL;
    int len_s = 0;
    const unsigned char *bytes_v = NULL;
    int len_v = 0;

    if (!username || !password || !ret_bytes_s || !ret_bytes_v)
    {
        sts = -1;
        goto error;
    }

    srp_create_salted_verification_key(
        G_alg,
        G_ng_type,
        username,
        (const unsigned char *) password,
        (int) strlen(password),
        &bytes_s,
        &len_s,
        &bytes_v,
        &len_v,
        G_n_hex,
        G_g_hex);
    
    *ret_bytes_s = (unsigned char *) bytes_s;
    *ret_len_s   = len_s;

    *ret_bytes_v = (unsigned char *) bytes_v;
    *ret_len_v = len_v;

error:
    return 0;
}


/*
 * SRP "exchange". These helper functions form the different client / server
 * SRP authentication routines, which use the password hash as the SRP password.
 */
/* ===================== Client routines =================================== */
struct SRPUser *
srpClientNew(
    char *username,
    char *password)
{
    struct SRPUser *usr = NULL;

    usr =  srp_user_new(
               G_alg,
               G_ng_type,
               username,
               (const unsigned char *) password,
               (int) strlen(password),
               G_n_hex,
               G_g_hex);
    return usr;
}

int
srpClientStartAuthentication(
    struct SRPUser *usr,
    char **ret_auth_username,
    unsigned char **ret_bytes_A,
    int *ret_len_A)
{
    int sts = 0;
    const char *auth_username = NULL;
    const unsigned char *bytes_A = NULL;
    int len_A = 0;

    srp_user_start_authentication(usr, &auth_username, &bytes_A, &len_A);
    if (!auth_username || !bytes_A)
    {
        sts = -1;
        goto error;
    }
    *ret_auth_username = (char *) auth_username, auth_username = NULL;
    *ret_bytes_A = (char *) bytes_A, bytes_A = NULL;
    *ret_len_A = len_A;

error:
    if (sts == -1)
    {
        if (auth_username)
        {
            free((char *) auth_username);
        }
        if (bytes_A)
        {
            free((char *) bytes_A);
        }
    }
    return sts;
}
    
int
srpClientChallenge(
    struct SRPUser *usr,
    unsigned char *bytes_s,
    int len_s,
    unsigned char *bytes_B,
    int len_B,
    unsigned char **ret_bytes_M,
    int *ret_len_M)
{
    int sts = 0;
    const unsigned char *bytes_M = NULL;
    int len_M = 0;

    srp_user_process_challenge(
        usr,
        bytes_s,
        len_s,
        bytes_B,
        len_B,
        &bytes_M,
        &len_M);
    if (!bytes_M)
    {
        sts = -1;
        goto error;
    }

    *ret_bytes_M = (unsigned char *) bytes_M;
    *ret_len_M = len_M;

error:
    return sts;
}

int srpClientVerifySession(
    struct SRPUser *usr,
    unsigned char *bytes_HAMK)
{
    srp_user_verify_session(usr, bytes_HAMK);
    return srp_user_is_authenticated(usr);
}

void srpClientDestroy(
    struct SRPUser *usr)
{
    if (usr)
    {
        srp_user_delete(usr);
    }
}


/* ===================== Server routines =================================== */

struct SRPVerifier *
srpServerNew(
    char *username,
    unsigned char *bytes_s,
    int len_s,
    unsigned char *bytes_v,
    int len_v,
    unsigned char *bytes_A,
    int len_A,
    unsigned char **ret_bytes_B,
    int *ret_len_B)
{
    int sts = 0;
    const unsigned char *bytes_B = NULL;
    int len_B = 0;
    struct SRPVerifier *ver = NULL;

    ver = srp_verifier_new(
              G_alg,
              G_ng_type,
              username,
              bytes_s,
              len_s,
              bytes_v,
              len_v,
              bytes_A,
              len_A,
              &bytes_B,
              &len_B,
              G_n_hex,
              G_g_hex);
    if (!bytes_B)
    {
        /* Verifier SRP-6a safety check violated! */
        sts = -1;
        goto error;
    }

    *ret_bytes_B = (unsigned char *) bytes_B;
    *ret_len_B = len_B;

error:
    if (sts == -1)
    {
        ver = NULL;
    }
    return ver;
}


int
srpServerVerify(
    struct SRPVerifier *ver,
    unsigned char *bytes_M,
    unsigned char **ret_bytes_HAMK)
{
    const unsigned char *bytes_HAMK = NULL;
    int sts = 0;
    srp_verifier_verify_session(ver, bytes_M, &bytes_HAMK);

    if ( !bytes_HAMK )
    {
        sts = -1;
        goto error;
    }

    *ret_bytes_HAMK = (unsigned char *) bytes_HAMK;

error:

    return sts;
}

void
srpServerDestroy(
    struct SRPVerifier *ver)
{
    if (ver)
    {
        srp_verifier_delete(ver);
    }
}

/*
 * Perform SRP gssapi_unix password authentication.
 * 
 * Known only by client: username / client_srp_pwd
 * Known only by server: server_srp_pwd
 */
int srpAuthenticate(
    char *username,
    char *client_srp_pwd,
    char *server_srp_pwd)
{
    struct SRPUser *cli_srp = NULL;
    struct SRPVerifier *svr_srp = NULL;
    int sts = 0;
    unsigned char *bytes_s = NULL;
    int len_s = 0;
    unsigned char *bytes_v = NULL;
    int len_v = 0;
    char *auth_username = NULL;
    unsigned char *bytes_A = NULL;
    int len_A = 0;
    unsigned char *bytes_B = NULL;
    int len_B = 0;
    unsigned char *bytes_M = NULL;
    int len_M = 0;
    unsigned char *bytes_HAMK = NULL;

    /* 
     * This call creates the temporary server-side SRP secret
     *
     * bytes_s: SRP salt, publically known to client/server
     * bytes_v: SRP secret, privately known only by server
     */
    sts = srpVerifierInit(
              username,
              server_srp_pwd,
              &bytes_s,
              &len_s,
              &bytes_v,
              &len_v);
    if (sts == -1)
    {
        fprintf(stderr, "  srpVerifierInit: Failed\n");
        goto error;
    }

    /* 1: Client initiates authentication sequence */
    cli_srp = srpClientNew(
                  username,
                  client_srp_pwd);
    if (!cli_srp)
    {
        fprintf(stderr, "  1) srpClientNew: Failed\n");
        sts = -1;
        goto error;
    }

    /* 2: Client generates public "A" value which is passed to server */
    sts = srpClientStartAuthentication(
              cli_srp,
              &auth_username,
              &bytes_A,
              &len_A);
    if (sts == -1)
    {
        fprintf(stderr, "  2) srpClientStartAuthentication: Failed\n");
        goto error;
    }

    /* 3: Server initializes its context, and returns public "B" value */
    svr_srp = srpServerNew(
                  auth_username,
                  bytes_s,
                  len_s,
                  bytes_v,
                  len_v,
                  bytes_A,
                  len_A,
                  &bytes_B,
                  &len_B);
    if (!svr_srp)
    {
        fprintf(stderr, "  3) srpServerNew: Failed\n");
        sts = -1;
        goto error;
    }

    /* 4: Client processes server challenge, generates mutual auth data */
    sts = srpClientChallenge(
              cli_srp,
              bytes_s,
              len_s,
              bytes_B,
              len_B,
              &bytes_M,
              &len_M);
    if (sts == -1)
    {
        fprintf(stderr, "  4) srpClientChallenge: Failed\n");
        goto error;
    }

    /* 5: Server verifies mutual auth data, generates mutual handshake data */
    sts = srpServerVerify(
              svr_srp,
              bytes_M,
              &bytes_HAMK);
    if (sts == -1)
    {
        fprintf(stderr, "  5) srpServerVerify: Failed\n");
        goto error;
    }

    /* 6: client verifies mutual auth handshake, and completes or fails auth */
    sts = srpClientVerifySession(
              cli_srp,
              bytes_HAMK);
    if (sts == -1)
    {
        fprintf(stderr, "  6) srpClientVerifySession: Failed\n");
        goto error;
    }

error:
    srpClientDestroy(cli_srp);
    srpServerDestroy(svr_srp);
    if (bytes_s)
    {
        free(bytes_s);
    }
    if (bytes_v)
    {
        free(bytes_v);
    }
    return sts;
}


int main(int argc, char *argv[])
{
    int sts = 0;
    char *username = NULL;
    char *user_salt = NULL;
    char *server_sp_hash = NULL;
    char *pwd = NULL;
    struct crypt_data cryptbuf;
    char *client_sp_hash = NULL;

    if (argc == 1)
    {
        fprintf(stderr, "usage: %s username\n", argv[0]);
        return 1;
    }
    username = argv[1];

    memset(&cryptbuf, 0, sizeof(cryptbuf));
    /*
     * UNIX shadow pwd file is queried to find user_salt
     *
     * user_salt: returned to client
     * server_sp_hash: used to generate temporary server SRP secret
     */
    sts = get_sp_salt(username, &user_salt, &server_sp_hash);
    if (sts == -1)
    {
        if (errno == EACCES)
        {
            fprintf(stderr,
                    "%s: must be run with root privilege (setuid root?)\n",
                    argv[0]);
        }
        else
        {
            fprintf(stderr, "%s: user %s does not exist\n", argv[1], username);
        }
        return 1;
    }
    printf("salt=%s\n", user_salt);
    printf("hash=%s\n", server_sp_hash);

    /* Get user password to compute client-side pwd hash */
    pwd = getpass("Password: ");

    /* client_sp_hash is generated by known password + user_salt */
    client_sp_hash = crypt_r(pwd, user_salt, &cryptbuf);
    printf("client_hash=%s\n", client_sp_hash);

    /*
     * srpAuthenticate: Simulate GSSAPI client/server exchange. 
     *
     * This does perform actual SRP authentication. Mock up to prove this
     * approach works without implementing complete GSS unix_pwd plugin.
     */
    sts = srpAuthenticate(username, client_sp_hash, server_sp_hash);
    if (sts == -1)
    {
        printf("srpAuthenticate failed!\n");
    }
    else
    {
        printf("srpAuthenticate passed!!!\n");
    }
    memset(&cryptbuf, 0, sizeof(cryptbuf));
    if (user_salt)
    {
        free(user_salt);
    }
    if (server_sp_hash)
    {
        free(server_sp_hash);
    }
    return 0;
}
