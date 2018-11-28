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
/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "postwatch.pb.h"

#ifdef BAZEL_BUILD
#include "examples/protos/postwatch.grpc.pb.h"
#else
#include "postwatch.grpc.pb.h"
#endif

#include "postwatch_server_c.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::protobuf::int64;

// Needed for stream I/O
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;

using postwatch::PostWatch;
using postwatch::WatchRequest;
using postwatch::WatchCreateRequest;
using postwatch::WatchCancelRequest;
using postwatch::WatchPingRequest;
using postwatch::WatchResponse;
using postwatch::Event;
using postwatch::Object;
using postwatch::Attribute;

using namespace std;

#ifdef  __cplusplus
extern "C" {
#endif

// Callback functions provided by POST Watch core middle-layer logic
static  int (*pfn_WatchCreateRequestCallback)(watch_create_request_t *create_ctx);
static  int (*pfn_WatchCancelRequestCallback)(int64_t watch_id);

#if 0 // set to if 1 for debugging
#define  _POSTWATCH_SERVER_DEBUG
#endif

#ifdef  __cplusplus
}
#endif

// Logic and data behind the server's behavior.
class PostWatchImpl final : public PostWatch::Service {

public:
Status RpcWatchRequest(
    ServerContext* context,
    ServerReaderWriter<WatchResponse, WatchRequest>* stream) override
{
    WatchRequest request;
    WatchResponse *response = NULL;
    Event event;

    while (stream->Read(&request)) {
      response = new(WatchResponse);
      switch (request.request_union_case()) {
        case WatchRequest::RequestUnionCase::kCreateRequest:
          {
            int sts = 0;
            watch_create_request_t create_request = {0};
            string tmp_str1;
            string tmp_str2;
#ifdef _POSTWATCH_SERVER_DEBUG
            cout << "CreateRequest called!" << endl;
#endif

            // Stuff protobuf arguments into C structure
            tmp_str1 = request.create_request().subtree();
            create_request.subtree = tmp_str1.c_str();

            // Search filter
            tmp_str2 = request.create_request().filter();
            create_request.filter = tmp_str2.c_str();

            // Boolean prev_value
            create_request.prev_value = request.create_request().prev_value();

            // Start revision
            create_request.start_revision = request.create_request().start_revision();

            // gRPC stream; opaque type to called function
            create_request.stream_handle = (void *) stream;
            if (pfn_WatchCreateRequestCallback) {
              sts =  pfn_WatchCreateRequestCallback(&create_request);
              if (sts == 0) {
                response->set_created(true);
                response->set_watch_id(create_request.watch_id);
              }

              // Send create response to client
              stream->Write(*response);
            }
          }
          break;

        case WatchRequest::RequestUnionCase::kCancelRequest:
          {
            int sts = 0;
            if (pfn_WatchCancelRequestCallback)
            {
              sts = pfn_WatchCancelRequestCallback(request.cancel_request().watch_id());
              if (sts == 0) {
                response->set_canceled(true);
              }

              // Send cancel response to client
              stream->Write(*response);
            }
          }
          break;

        // Reply to "ping" from within the postd gRPC server
        case WatchRequest::RequestUnionCase::kProgressRequest:
            stream->Write(*response);
#ifdef _POSTWATCH_SERVER_DEBUG
            cout << "PingRequest called!" << endl;
#endif
          break;

        default:
          break;
      }
      delete(response);
    }
    return Status::OK;
  }
};

void PostWatchRunServer_CXX(int argc, char **argv) {
  PostWatchImpl service;
  string server_address;

  ServerBuilder builder;

  if (argc > 1) {
    server_address = argv[1];
  }
  else {
    server_address = "0.0.0.0:50153";
  }

  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);

  // Finally assemble the server.
  unique_ptr<Server> server(builder.BuildAndStart());
#ifdef _POSTWATCH_SERVER_DEBUG
  cout << "Server listening on " << server_address << endl;
#endif

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

static
int
WatchSendEventsNewObject(postwatch_object_t *pobject,
                         Object **pObject)
{
    /* Refactor this into a method, so the prev_object can use the same code */
    Object *cur_obj = new(Object);
    cur_obj->set_version(pobject->version);
    cur_obj->set_dn(pobject->dn);

    /* Populate attributes */
    for (int attrib_count = 0;
         attrib_count < pobject->attribute_count;
         attrib_count++)
    {
        postwatch_attribute_t *attribute =
            &pobject->attribute_list[attrib_count];
        Attribute *attrib = cur_obj->add_attributelist();

        attrib->set_name(attribute->name);
        // Add one or more attribute values
        for (int value_count = 0;
             value_count < attribute->value_count;
             value_count++)
        {
            attrib->add_value(attribute->values[value_count]);
        }
    }
    *pObject  = cur_obj;

    return 0;
}

uint32_t
WatchSendEvents_CXX(void *stream_ptr,
                    postwatch_response_t *response)
{
    int events_count = 0;
    ppostwatch_event_t pevent;
    WatchResponse *resp = NULL;
    ServerReaderWriter<WatchResponse, WatchRequest>* stream =
        (ServerReaderWriter<WatchResponse, WatchRequest>*) stream_ptr;

    resp = new(WatchResponse);
    resp->set_created(response->created);
    resp->set_canceled(response->canceled);
    resp->set_compact_revision(response->compact_revision);

    while  (events_count < response->events_count)
    {
        Event *event = NULL;
        event = resp->add_events();
        pevent = &response->events[events_count];
        event->set_revision(pevent->revision);
        event->set_event_type((Event::EventType) pevent->event_type);

        // Marshal current object C response structure to a WatchResponse protobuf
        if (pevent->current_object)
        {
            Object *cur_obj = NULL;
            int sts = 0;
            sts = WatchSendEventsNewObject(pevent->current_object, &cur_obj);
            if (sts) {
                return sts;
            }
            event->set_allocated_currentobject(cur_obj);
        }

        // Marshal prev object C response structure to a WatchResponse protobuf
        if (pevent->prev_object)
        {
            Object *cur_obj = NULL;
            int sts = 0;
            sts = WatchSendEventsNewObject(pevent->prev_object, &cur_obj);
            if (sts) {
                return sts;
            }
            event->set_allocated_currentobject(cur_obj);
        }
        events_count++;
    }

    // Send the response
    stream->Write(*resp);

    delete(resp);

    return 0;
}

#ifdef  __cplusplus
extern "C" {
#endif


void PostWatchRunServer(int argc, char **argv)
{
    PostWatchRunServer_CXX(argc, argv);
}

void SetPostWatchCallbacks(postwatch_callbacks_t *callbacks)
{
    pfn_WatchCreateRequestCallback = callbacks->create_request_cb;
    pfn_WatchCancelRequestCallback = callbacks->cancel_request_cb;
}

uint32_t
WatchSendEvents(void *grpc_stream,
                postwatch_response_t *response)
{
    return WatchSendEvents_CXX(
               grpc_stream,
               response);
}

#ifdef  __cplusplus
}
#endif
