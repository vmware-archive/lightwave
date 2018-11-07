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

#include <stdio.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/stub_options.h>


#ifdef BAZEL_BUILD
#include "examples/protos/domath.grpc.pb.h"
#else
#include "domath.grpc.pb.h"
#endif

using grpc::ClientContext;
using grpc::Status;
using grpc::protobuf::int64;
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

  while (i<argc) {
    reply = AddTwoIntValues(stub_, reply, atoi(argv[i]), ok);
    if (!ok) {
        cout << "Error calling AddTwoIntValues!\n";
        exit(1);
    }
    i++;
  }
  cout << "DoMath client sum: " << reply << endl;

  return 0;
}
