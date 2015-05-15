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



/*
 * Module Name: ThinAppRepoService
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Thinapp Repository Database
 *
 * Definitions
 *
 */

#define EVENTLOG_DB_MAX_NUM_CACHED_CONTEXTS (5)

#define BAIL_ON_EVENTLOG_ERROR(dwError)           \
    if (dwError)                                    \
    {                                               \
        goto error;                                 \
    }

#define UNKNOWN_STRING "UNKNOWN"

#define EVENTLOG_ERROR_SQLITE_TABLE_INITIALIZER \
{ \
    { 1  , "SQLITE_ERROR",     "SQL error or missing database" }, \
    { 2  , "SQLITE_INTERNAL",  "Internal logic error in SQLite" }, \
    { 3  , "SQLITE_PERM",      "Access permission denied" }, \
    { 4  , "SQLITE_ABORT",     "Callback routine requested an abort" }, \
    { 5  , "SQLITE_BUSY",      "The database file is locked" }, \
    { 6  , "SQLITE_LOCKED",    "A table in the database is locked" }, \
    { 7  , "SQLITE_NOMEM",     "A malloc() failed" }, \
    { 8  , "SQLITE_READONLY",  "Attempt to write a readonly database" }, \
    { 9  , "SQLITE_INTERRUPT", "Operation terminated by sqlite3_interrupt()"}, \
    { 10 , "SQLITE_IOERR",     "Some kind of disk I/O error occurred" }, \
    { 11 , "SQLITE_CORRUPT",   "The database disk image is malformed" }, \
    { 12 , "SQLITE_NOTFOUND",  "Unknown opcode in sqlite3_file_control()" }, \
    { 13 , "SQLITE_FULL",      "Insertion failed because database is full" }, \
    { 14 , "SQLITE_CANTOPEN",  "Unable to open the database file" }, \
    { 15 , "SQLITE_PROTOCOL",  "Database lock protocol error" }, \
    { 16 , "SQLITE_EMPTY",     "Database is empty" }, \
    { 17 , "SQLITE_SCHEMA",    "The database schema changed" }, \
    { 18 , "SQLITE_TOOBIG",    "String or BLOB exceeds size limit" }, \
    { 19 , "SQLITE_CONSTRAINT","Abort due to constraint violation" }, \
    { 20 , "SQLITE_MISMATCH",  "Data type mismatch" }, \
    { 21 , "SQLITE_MISUSE",    "Library used incorrectly" }, \
    { 22 , "SQLITE_NOLFS",     "Uses OS features not supported on host" }, \
    { 23 , "SQLITE_AUTH",      "Authorization denied" }, \
    { 24 , "SQLITE_FORMAT",    "Auxiliary database format error" }, \
    { 25 , "SQLITE_RANGE",     "2nd parameter to sqlite3_bind out of range" }, \
    { 26 , "SQLITE_NOTADB",    "File opened that is not a database file" }, \
    {100 , "SQLITE_ROW",       "sqlite3_step() has another row ready" }, \
    {101 , "SQLITE_DONE",      "sqlite3_step() has finished executing" }, \
};
