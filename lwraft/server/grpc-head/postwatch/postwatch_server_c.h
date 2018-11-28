/*
 * Copyright ©2018 VMware, Inc.  All Rights Reserved.
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

#ifndef _POSTWATCH_SERVER_C_H_
#define _POSTWATCH_SERVER_C_H_

#ifdef  __cplusplus
extern "C" {
#endif


#define GRPC_POSTWATCH_PORT 50153
#define GRPC_LISTENING_IP "0.0.0.0"

#ifndef STRINGIFY
#define STRINGIFY(m) #m
#endif
#ifndef TOSTRING
#define TOSTRING(s) STRINGIFY(s)
#endif

typedef struct _WATCH_CREATE_REQUEST_T
{
/* [in]  */    const char *subtree;
/* [in]  */    const char *filter;
/* [in]  */    int64_t start_revision;
/* [in]  */    void *stream_handle;
/* [out] */    int64_t watch_id;
/* [in]  */    uint8_t prev_value;      // boolean value
} watch_create_request_t;

typedef struct _POSTWATCH_CALLBACKS_T
{
    int (*create_request_cb)(watch_create_request_t *);
    int (*cancel_request_cb)(int64_t);
} postwatch_callbacks_t, *ppostwatch_callbacks_t;

typedef enum _POSTWATCH_EVENT_TYPE_E
{
    // use Post Watch Enum (PWE) prefix to prevent namespace pollution
    PWE_ADD = 1,
    PWE_MOD,
    PWE_DELETE,
} postwatch_event_type_e;

typedef struct _POSTWATCH_ATTRIBUTE_T
{
    char *name;
    int64_t value_count;
    char **values;
} postwatch_attribute_t;

typedef struct _POSTWATCH_OBJECT_T
{
    int version;
    char *dn;
    int64_t attribute_count;
    postwatch_attribute_t *attribute_list;
} postwatch_object_t;

typedef struct _POSTWATCH_EVENT_T
{
    int revision;
    postwatch_event_type_e event_type;
    postwatch_object_t *current_object;
    postwatch_object_t *prev_object;
} postwatch_event_t, *ppostwatch_event_t;


typedef struct _POSTWATCH_RESPONSE_T
{
    int64_t watch_id;
    uint8_t created;
    uint8_t canceled;
    int64_t compact_revision;
    int64_t events_count;
    postwatch_event_t events[];
} postwatch_response_t, *ppostwatch_response_t;

void PostWatchRunServer(int argc, char **argv);

uint32_t WatchSendEvents(void *grpc_stream, postwatch_response_t *watch_response);

postwatch_callbacks_t *PostWatchGetCallbacks(void);

#ifdef  __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef _POSTWATCH_SERVER_C_H_ */
