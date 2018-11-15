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

#ifdef BAZEL_BUILD
#include "examples/protos/domath.grpc.pb.h"
#else
#include "domath.grpc.pb.h"
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
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;

using domath::OneIntValue;
using domath::TwoIntValues;
using domath::IntResult;
using domath::MathComputation;

using namespace std;

// Assembles the client's payload, sends it and presents the response back
// from the server.
int64 AddTwoIntValues(
    unique_ptr<domath::MathComputation::Stub>& stub_,
    const int64 i,
    const int64 j,
    bool& ok)
{
  Status status;
  TwoIntValues request; // Data we are sending to the server.
  IntResult reply; // Container for the data we expect from the server.
  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  request.set_i(i);
  request.set_j(j);

  // The actual RPC.
  status = stub_->Add2Values(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    ok = true;
    return reply.v();
  } else {
    cout << status.error_code() << ": " << status.error_message()
              << endl;
    ok = false;
    return 0;
  }
}

int64 AddIntValues(
    unique_ptr<domath::MathComputation::Stub>& stub_,
    int argc,
    char *argv[],
    bool& ok)
{
  ClientContext context;
  OneIntValue one_value;
  IntResult total_sum;
  int val;
  Status status;
  int64 ret_sum = 0;
  bool grpc_ok = false;

  std::unique_ptr<ClientWriter<OneIntValue> > writer(
    stub_->AddValues(&context, &total_sum));

  for (int i=1; i<argc; i++)
  {
    val = atoi(argv[i]);
    one_value.set_i(val);
    grpc_ok = writer->Write(one_value);
    if (!grpc_ok)
    {
        ok = grpc_ok;
        return 0;
    }
  }
  writer->WritesDone();
  status = writer->Finish();
  if (status.ok())
  {
    ok = true;
    ret_sum = total_sum.v();
  }
  else
  {
    ok = false;
  }

  return ret_sum;
}

int64 AddValuesBiStreamClient(
    unique_ptr<domath::MathComputation::Stub>& stub_,
    int argc,
    char *argv[],
    bool& ok)
{
    ClientContext context;
    int64 sum;
    IntResult server_sum;

    std::shared_ptr<ClientReaderWriter<OneIntValue, IntResult> > stream(
        stub_->AddValuesBiStream(&context));

    // Create writer thread
    std::thread writer([stream]() {
        int i = 0;
        OneIntValue int_val;

        for (i=0; i<10; i++) {
            int_val.set_i(i);
            stream->Write(int_val);
        }
        stream->WritesDone();
    });


    while (stream->Read(&server_sum)) {
        cout << "Server sum: " << server_sum.v() << endl;
    }
    writer.join();

    return sum;
}

int main(int argc, char** argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  int i=1;
  string server_addr = "localhost:50053";
  bool ok = false;
  int64 reply = 0;
  unique_ptr<domath::MathComputation::Stub> stub_;


  stub_ = domath::MathComputation::NewStub(
              grpc::CreateChannel(server_addr,
                                  grpc::InsecureChannelCredentials()));

  cout << "Calling blocking RPC to add numbers" << endl;
  while (i<argc) {
    reply = AddTwoIntValues(stub_, reply, atoi(argv[i]), ok);
    if (!ok) {
        cout << "Error calling AddTwoIntValues!\n";
        exit(1);
    }
    i++;
  }
  cout << endl;

  cout << "DoMath client sum: " << reply << endl;
  AddValuesBiStreamClient(
    stub_,
    argc,
    argv,
    ok);

  cout << "Calling AddIntValues, c->s stream, to add numbers" << endl;
  reply = AddIntValues(stub_,
                       argc,
                       argv,
                       ok);
  if (ok) {
    cout << "sum=" << reply << endl;
  }
  else
  {
    cout << "AddIntValues failed for some reason!" << endl;
  }
  cout << endl;

  return 0;
}
