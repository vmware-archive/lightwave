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
 * Module Name: VMware Certificate Server
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * VMware Certificate Server Error codes and Text
 *
 * Definitions
 *
 */

#ifndef __VECS_ERROR_H__
#define __VECS_ERROR_H__

typedef struct _VECS_ERROR_CODE_NAME_MAP
{
    int         code;
    const char* name;
    const char* desc;

} VECS_ERROR_CODE_NAME_MAP, *PVECS_ERROR_CODE_NAME_MAP;

#define UNKNOWN_STRING "UNKNOWN"

#define VECS_BASE_ERROR                     90000
#define VECS_GENERIC_FILE_IO                (VECS_BASE_ERROR +   1)
#define VECS_CRL_OPEN_ERROR                 (VECS_BASE_ERROR +   2)
#define VECS_BUFFER_SIZE_ERROR              (VECS_BASE_ERROR +   3)
#define VECS_CRL_WRITE_ERROR                (VECS_BASE_ERROR +   4)
#define VECS_ALIAS_NAME_ERROR				(VECS_BASE_ERROR +   5)
#define VECS_UNIQUE_ALIAS_ERROR				(VECS_BASE_ERROR +   6)
#define VECS_NO_CERT_FOUND					(VECS_BASE_ERROR +   7)
#define VECS_CRL_IO_ERROR                   (VECS_BASE_ERROR +   8)
#define VECS_TASK_QUEUE_NULL                (VECS_BASE_ERROR +   9)
#define VECS_NULL_TASK_ERROR                (VECS_BASE_ERROR +  10)
#define VECS_NO_START_ROUTINE               (VECS_BASE_ERROR +  11)
#define VECS_TASK_QUEUE_FULL                (VECS_BASE_ERROR +  12)
#define VECS_TASK_QUEUE_EMPTY               (VECS_BASE_ERROR +  13)
#define VECS_TASK_NOT_FOUND                 (VECS_BASE_ERROR +  14)
#define VECS_NO_READY_TASK                  (VECS_BASE_ERROR +  15)
#define VECS_OUT_OF_MEMORY                  (VECS_BASE_ERROR +  16)
#define VECS_CERT_IO_FAILURE                (VECS_BASE_ERROR +  17)
#define VECS_PRIVATE_KEY_MISMATCH           (VECS_BASE_ERROR +  18)
#define VECS_MISSING_DC_NAME                (VECS_BASE_ERROR +  19)
#define VECS_MISSING_CREDS                  (VECS_BASE_ERROR +  20)
#define VECS_NOT_JOINED                     (VECS_BASE_ERROR +  21)
#define VECS_UNKNOWN_ERROR                  (VECS_BASE_ERROR + 101)


#define VMCA_ERROR_TABLE_INITIALIZER \
{ \
    { VECS_GENERIC_FILE_IO         , "VECS_GENERIC_FILE_IO", "Unable to do file I/O."}, \
    { VECS_CRL_OPEN_ERROR          , "VECS_CRL_OPEN_ERROR", "Unable to open CRL file." }, \
    { VECS_BUFFER_SIZE_ERROR       , "VECS_BUFFER_SIZE_ERROR", "Buffer Size Error"}, \
    { VECS_CRL_WRITE_ERROR         , "VECS_CRL_WRITE_ERROR", "Unable to write to CRL File"} ,\
    { VECS_ALIAS_NAME_ERROR		   , "VECS_ALIAS_NAME_ERROR", "Illegal Alias Name"}, \
    { VECS_UNIQUE_ALIAS_ERROR	   , "VECS_UNIQUE_ALIAS_ERROR", "Unique Alias Name Constraint violation"}, \
    { VECS_NO_CERT_FOUND           , "VECS_NO_CERT_FOUND", "No certificate Found"}, \
    { VECS_CRL_IO_ERROR            , "VECS_CRL_IO_ERROR", "Unable to read the CRL"} , \
    { VECS_TASK_QUEUE_NULL         , "VECS_TASK_QUEUE_NULL", "Task Queue is NULL"}, \
    { VECS_NO_START_ROUTINE        , "VECS_NO_START_ROUTINE", "Missing Start Routine"}, \
    { VECS_TASK_QUEUE_FULL         , "VECS_TASK_QUEUE_FULL", "Task Queue is full"}, \
    { VECS_TASK_QUEUE_EMPTY        , "VECS_TASK_QUEUE_EMPTY", "Task Queue is empty"}, \
    { VECS_TASK_NOT_FOUND          , "VECS_TASK_NOT_FOUND","Task not found"}, \
    { VECS_NO_READY_TASK           , "VECS_NO_READY_TASK", "No task ready to run now"}, \
    { VECS_OUT_OF_MEMORY           , "VECS_OUT_OF_MEMORY", "Out of Memory error"}, \
    { VECS_CERT_IO_FAILURE         , "VECS_CERT_IO_FAILURE", "Not able to decode certificate"}, \
    { VECS_PRIVATE_KEY_MISMATCH    , "VECS_PRIVATE_KEY_MISMATCH", "Private key does not match certificate"}, \
    { VECS_MISSING_DC_NAME         , "VECS_MISSING_DC_NAME", "DC Name Required."} , \
    { VECS_MISSING_CREDS           , "VECS_MISSING_CREDS", "User Name or Password is missing"}, \
    { VECS_NOT_JOINED           , "VECS_NOT_JOINED", "The system is not joined to a domain"}, \
    { VECS_UNKNOWN_ERROR            , "VECS_UNKNOWN_ERROR"  , "Certificate End Point Unknown Error" }, \
};

#endif //__VECS_ERROR_H__

