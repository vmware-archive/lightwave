/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* Tool to verify
 *   1. Whether two DBs have the same data. i.e. same checksum on all recrods.
 *   2. Whether a DB integrity is intact by iterating through all records.
 * */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <openssl/sha.h>
#include <pthread.h>
#include "lmdb.h"

static volatile sig_atomic_t gotsig;
int alldbs = 0, envflags = 0, list = 0;
char *subname = NULL;
int nenv = 1;
size_t mapsize = 5*107374182400;
MDB_env *env = NULL;
int multithread_iteration = 0;

static void dumpsig( int sig )
{
    gotsig=1;
}

static int calc_checksum(SHA_CTX *ctx, MDB_txn *txn, MDB_dbi dbi)
{
    MDB_cursor *mc = NULL;
    MDB_val key, data;
    int rc = 0;

    rc = mdb_cursor_open(txn, dbi, &mc);
    if (rc) return rc;

    /*Iterate through the DB*/
    while ((rc = mdb_cursor_get(mc, &key, &data, MDB_NEXT) == MDB_SUCCESS)) {
        if (gotsig) {
            rc = EINTR;
            break;
        }
        if (ctx)
        {
            SHA1_Update(ctx, key.mv_data, key.mv_size);
            SHA1_Update(ctx, data.mv_data, data.mv_size);
        }
    }
    if (rc == MDB_NOTFOUND)
        rc = MDB_SUCCESS;

    mdb_cursor_close(mc);

    return rc;
}

static void *subdb_thread(void *arg)
{
    int rc = 0;
    MDB_txn *txn = NULL;
    char *subdb = (char *) arg;
    MDB_dbi db2;

    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc)
    {
        fprintf(stderr, "mdb_txn_begin failed, error %d %s subdb:%s \n", rc, mdb_strerror(rc), subdb);
        return NULL;
    }

    rc = mdb_open(txn, subdb, 0, &db2);
    if (rc != MDB_SUCCESS)
    {
        fprintf(stderr, "Failed to open %s \n", subdb);
    }
    else
    {
        rc = calc_checksum(NULL, txn, db2);
        mdb_close(env, db2);
        printf("Finished iterating through %s SubDB \n", subdb);
    }

    mdb_txn_abort(txn);

    return NULL;
}

static void usage(char *prog)
{
    fprintf(stderr, "usage: %s [-l] [-n] [-i] [-a|-s subdb] dbpath [dbpath2]\n", prog);
    fprintf(stderr, "-l: List the names of subDBs\n-n: Use MDB_NOSUBDIR flag\n");
    fprintf(stderr, "-a: Iterate through all subDBs\n-s: Iterate through subDB provided in argument\n");
    fprintf(stderr, "-i: multi-threads mode (to quickly validate DB integrity)\n");
    fprintf(stderr, "-m: Memory map size in bytes. Default is 500 GB.\n");
    exit(EXIT_FAILURE);
}

static void parse_args(int argc, char *argv[])
{
    int i = 0;
    char *prog = argv[0];

    if (argc < 2) {
        usage(prog);
    }

    /* -a: parse main DB and all subDBs
     * -s: parse only the named subDB
     * -n: use NOSUBDIR flag on env_open
     * (default) parse only the main DB
     */
    while ((i = getopt(argc, argv, "alnis:m:")) != EOF) {
        switch(i) {
            case 'l':
                list = 1;
            case 'a':
                if (subname)
                    usage(prog);
                alldbs++;
                break;
            case 'n':
                envflags |= MDB_NOSUBDIR;
                break;
            case 's':
                if (alldbs)
                    usage(prog);
                subname = optarg;
                break;
            case 'm':
                mapsize = atol(optarg);
                break;
            case 'i':
                multithread_iteration = 1;
                break;
            default:
                usage(prog);
        }
    }

    if (optind < argc - 2 || optind >= argc)
        usage(prog);

    if (optind == argc - 2)
        nenv = 2;

    return;
}

int main(int argc, char *argv[])
{
    int rc = 0, i = 0;
    MDB_txn *txn = NULL;
    MDB_dbi dbi;
    char *envname = NULL;
    SHA_CTX ctx;
    unsigned char hash[2][SHA_DIGEST_LENGTH];
    int n = 0;

    parse_args(argc, argv);

#ifdef SIGPIPE
    signal(SIGPIPE, dumpsig);
#endif
#ifdef SIGHUP
    signal(SIGHUP, dumpsig);
#endif
    signal(SIGINT, dumpsig);
    signal(SIGTERM, dumpsig);

    n = nenv;
    while (nenv > 0) {
        SHA1_Init(&ctx);
        envname = argv[optind];
        optind++;
        nenv--;

        rc = mdb_env_create(&env);
        if (rc) {
            fprintf(stderr, "mdb_env_create failed, error %d %s\n", rc, mdb_strerror(rc));
            rc = EXIT_FAILURE;
            goto done;
        }

        if (alldbs || subname) {
            rc = mdb_env_set_maxdbs(env, 100);
            if (rc) {
                fprintf(stderr, "mdb_env_set_maxdbs failed, error %d %s\n", rc, mdb_strerror(rc));
                goto env_close;
            }
        }

        /*This step can be removed if we upgrade to MDB >= 0.9.14*/
        rc = mdb_env_set_mapsize(env, mapsize);
        if (rc) {
            fprintf(stderr, "mdb_env_set_mapsize failed, error %d %s\n", rc, mdb_strerror(rc));
            goto env_close;
        }

        rc = mdb_env_open(env, envname, envflags | MDB_RDONLY, 0600);
        if (rc) {
            fprintf(stderr, "mdb_env_open failed, error %d %s\n", rc, mdb_strerror(rc));
            goto env_close;
        }

        rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
        if (rc) {
            fprintf(stderr, "mdb_txn_begin failed, error %d %s\n", rc, mdb_strerror(rc));
            goto env_close;
        }

        /*If subname is not given, open MainDB*/
        rc = mdb_open(txn, subname, 0, &dbi);
        if (rc) {
            fprintf(stderr, "mdb_open failed, error %d %s\n", rc, mdb_strerror(rc));
            goto txn_abort;
        }

        if (alldbs) {
            MDB_cursor *cursor;
            MDB_val key;
            pthread_t thread_id[100];
            int db_count = 0;

            rc = mdb_cursor_open(txn, dbi, &cursor);
            if (rc) {
                fprintf(stderr, "mdb_cursor_open failed, error %d %s\n", rc, mdb_strerror(rc));
                goto txn_abort;
            }
            /*Iterate through MainDB*/
            while ((rc = mdb_cursor_get(cursor, &key, NULL, MDB_NEXT)) == 0) {
                char *str;
                MDB_dbi db2;
                SHA1_Update(&ctx, key.mv_data, key.mv_size);
                str = malloc(key.mv_size+1);
                if (str == NULL) {
                    fprintf(stderr, "malloc failed.");
                    rc = EXIT_FAILURE;
                    break;
                }
                memcpy(str, key.mv_data, key.mv_size);
                str[key.mv_size] = '\0';

                if (multithread_iteration)
                {
                    pthread_create(&thread_id[db_count], NULL, subdb_thread, (void *)str);
                    printf("Started thread %lu for subdb %s \n", thread_id[db_count], str);
                    db_count++;
                    free(str);

                    continue;
                }

                /*Open Sub DB*/
                rc = mdb_open(txn, str, 0, &db2);
                if (rc == MDB_SUCCESS) {
                    if (list) {
                        printf("%s\n", str);
                        list++;
                    } else {
                        rc = calc_checksum(&ctx, txn, db2);
                        if (rc)
                            break;
                    }
                    mdb_close(env, db2);
                }
                else {
                    fprintf(stderr, "Failed to open %s", str);
                }
                free(str);
                if (rc) break;
            }
            mdb_cursor_close(cursor);
            if (rc == MDB_NOTFOUND) {
                rc = MDB_SUCCESS;
            }

            if (multithread_iteration)
            {
                int thread_count = 0;
                for (thread_count = 0; thread_count < db_count; thread_count++)
                {
                    pthread_join(thread_id[thread_count], NULL);
                }
            }
        }else {
            /*Only calculate checksum of the specified Sub DB*/
            rc = calc_checksum(&ctx, txn, dbi);
        }
        if (rc && rc != MDB_NOTFOUND)
            fprintf(stderr, "%s: %s: %s\n", argv[0], envname, mdb_strerror(rc));

        mdb_close(env, dbi);
txn_abort:
        mdb_txn_abort(txn);
env_close:
        mdb_env_close(env);

        if (rc) {
            return EXIT_FAILURE;
        }

        SHA1_Final(hash[nenv], &ctx);
    }

    if (n == 2 && memcmp(hash[0], hash[1], SHA_DIGEST_LENGTH)) {
        fprintf(stderr, "DBs do not match.\n");
        rc = 1;
    }
    else if (n == 1 && !list) {
        printf("SHA1 Sum = ");
        for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
            printf("%d", (int) hash[0][i]);
        }
        printf("\n");
    }

done:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
