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

#include "addint64.h"

#ifdef BAZEL_BUILD
#include "examples/protos/domath.grpc.pb.h"
#else
#include "domath.grpc.pb.h"
#endif

#include "domath_server_c.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::protobuf::int64;
using domath::TwoIntValues;
using domath::IntResult;
using domath::MathComputation;
using namespace std;

// Logic and data behind the server's behavior.
class MathComputationImpl final : public MathComputation::Service {

public:
  Status Add2Values(ServerContext* context,
                    const TwoIntValues* request,
                    IntResult* reply) override
  {
    int64 retval;
    // gRPC request protobuf
    int64 i = request->i();
    int64 j = request->j();

    retval = AddInt64(i, j); // This is implemented with C code

    // gRPC response protobuf
    reply->set_v(retval);

    return Status::OK;
  }
};

void RunServer(int argc, char **argv) {
  MathComputationImpl service;
  string server_address;

  ServerBuilder builder;

  if (argc > 1) {
    server_address = argv[1];
  }
  else {
    server_address = "0.0.0.0:50053";
  }

  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);

  // Finally assemble the server.
  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

#ifdef  __cplusplus
extern "C" {
#endif

void RunServer_CXX(int argc, char **argv)
{
    RunServer(argc, argv);
}

#ifdef  __cplusplus
}
#endif
