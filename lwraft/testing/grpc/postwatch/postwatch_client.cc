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

// C++ includes
#include <iostream>
#include <memory>
#include <string>
#include <thread>

// Linux includes
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef BAZEL_BUILD
#include "examples/protos/postwatch.grpc.pb.h"
#else
#include "postwatch.grpc.pb.h"
#endif

// gRPC includes
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

// gRPC Namespaces
using grpc::ClientContext;
using grpc::Status;
using grpc::protobuf::int64;

// Needed for stream rpc
using grpc::ClientReaderWriter;

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

struct ClientReaderThreadContext {
    std::shared_ptr<ClientReaderWriter<WatchRequest, WatchResponse> > stream;
    time_t t_cur;
    time_t t_prev;
};

void client_msg_thread(ClientReaderThreadContext *tctx) 
{
        WatchRequest union_request;
        WatchCreateRequest *create_request = new(WatchCreateRequest);
        WatchCancelRequest *cancel_request = new(WatchCancelRequest);
        WatchPingRequest *progress_request = new(WatchPingRequest);
        time_t t_prev = time(0);
        time_t t_cur = time(0);
        std::shared_ptr<ClientReaderWriter<WatchRequest, WatchResponse> > stream = tctx->stream;
        int64 watchId = 0;

        // Fill in the create_request object here!
        create_request->set_subtree("dc=lightwave,dc=local");
        create_request->set_filter("(objectlass=*)");
        create_request->set_prev_value(false);
        create_request->set_start_revision(1000);

t_cur = time(0);
printf("%s\n", ctime(&t_cur));

        // Fill in the request union and send to postwatch
        union_request.set_allocated_create_request(create_request);
        stream->Write(union_request);

        // Fill in the request union and send to postwatch
        watchId = getpid();
        cancel_request->set_watch_id(watchId);
        union_request.set_allocated_cancel_request(cancel_request);
        stream->Write(union_request);

        union_request.set_allocated_progress_request(progress_request);
        stream->Write(union_request);

        for (int i=0; i<10; i++)
        {
            /* This is a ping message */
            cout << "client_msg_thread: ping count=" << i << endl;
            stream->Write(union_request);
            sleep(5);
t_cur = time(0);
printf("%s\n", ctime(&t_cur));
        }
        sleep(100);  // TBD:Adam hack
t_cur = time(0);
printf("%s\n", ctime(&t_cur));
        stream->WritesDone();
t_cur = time(0);
printf("%s\n", ctime(&t_cur));
}

int64 PostWatchSendRpc(
    unique_ptr<postwatch::PostWatch::Stub>& stub_,
    bool& ok)
{
    ClientContext context;
    WatchResponse response;
    ClientReaderThreadContext *tctx = (ClientReaderThreadContext *) malloc(sizeof(ClientReaderThreadContext));

    std::shared_ptr<ClientReaderWriter<WatchRequest, WatchResponse> > stream(
        stub_->RpcWatchRequest(&context));

    tctx->stream = stream;
    // Create writer thread
    std::thread client_msg_thread_h (client_msg_thread, tctx); 

    while (stream->Read(&response)) {
        cout << "Num Events in response: " << response.events_size() << endl;

        /* Determine the reply type */
        if (!response.created() && !response.canceled() && response.events_size() == 0) {
            cout << "response type: PING " << response.events_size() << endl;
        }
        else if (response.canceled()) {
            cout << "response type: CANCEL " << response.canceled() << endl;
        }
        else if (response.created()) {
            cout << "response type: CREATED " << response.created() << " watchid=" <<  response.watch_id() << endl;
        }
        else if (response.events_size() > 0) {
            int events_size = response.events_size();
             
            cout << "response type: WATCH event count=" << response.events_size() << endl;
            cout << "response type: WATCH watch_id=" << response.watch_id() << endl;
            cout << "response type: WATCH compact_revision=" << response.compact_revision() << endl;
            for (int event_count = 0; event_count < events_size; event_count++)
            {
                Event event = response.events(event_count);
                int attrib_count = event.currentobject().attributelist_size();

                cout << "response type: WATCH event[" << event_count << "]"
                        " revision=" << event.revision() << endl;
                cout << "response type: WATCH event[" << event_count << "]"
                        " event_type=" << event.event_type() << endl;
                cout << "response type: WATCH event[" << event_count << "]"
                        " version=" << event.currentobject().version() << endl;
                cout << "response type: WATCH event[" << event_count << "]"
                        " dn=" << event.currentobject().dn() << endl;
                for (int i_attrib = 0; i_attrib < attrib_count; i_attrib++) {
                    Attribute attrib = event.currentobject().attributelist(i_attrib);
                    int i_value_count = attrib.value_size();
                    
                    cout << "response type: WATCH event[" << event_count << "]"
                            " attributelist_size=" << (i_attrib+1) << " of " << attrib_count << endl;
                    cout << "                     Attribute[" << i_attrib << "] Name=" << attrib.name() << " value_count=" << i_value_count << endl;
                    for (int i_value = 0; i_value < i_value_count; i_value++) {
                        const char *value = NULL;
                        string cxx_value = attrib.value(i_value);
                        value = cxx_value.c_str();
                        cout << "                         value[" << i_value << "]=" << value << endl;
                    }
                }
                cout << endl;
            }
        }
        else {
            cout << "response type: UNKNOWN" << endl;
        }
    }
    client_msg_thread_h.join();

    return true;
}

int main(int argc, char** argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  int i=1;
  string server_addr = "localhost:50153";
  bool ok = false;
  int64 reply = 0;
  unique_ptr<postwatch::PostWatch::Stub> stub_;


  stub_ = postwatch::PostWatch::NewStub(
              grpc::CreateChannel(server_addr,
                                  grpc::InsecureChannelCredentials()));

  reply = PostWatchSendRpc(stub_, ok);
  if (ok) {
    cout << "RPC worked" << endl;
  }
  else
  {
    cout << "RPC somehow failed" << endl;
  }

  return 0;
}
