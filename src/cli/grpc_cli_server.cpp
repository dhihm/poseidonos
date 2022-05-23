#include <iostream>
#include <memory>
#include <string>

#include "src/cli/grpc_cli_server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc_cli::PosCli;
using grpc_cli::SystemInfoRequest;
using grpc_cli::SystemInfoResponse;
using grpc_cli::SystemStopRequest;
using grpc_cli::SystemStopResponse;

CommandProcessor* pc;

class PosCliServiceImpl final : public PosCli::Service {
  Status SystemInfo(ServerContext* context, const SystemInfoRequest* request,
                  SystemInfoResponse* reply) override {

    Status status = pc->ExecuteSystemInfoCommand(request, reply);
    
    return status;
  }

  Status SystemStop(ServerContext* context, const SystemStopRequest* request,
                  SystemStopResponse* reply) override {

    Status status = pc->ExecuteSystemStopCommand(request, reply);
    
    return status;
  }

    Status GetSystemProperty(ServerContext* context, const GetSystemPropertyRequest* request,
                  GetSystemPropertyResponse* reply) override {

    Status status = pc->ExecuteGetSystemPropertyCommand(request, reply);
    
    return status;
  }

  Status SetSystemProperty(ServerContext* context, const SetSystemPropertyRequest* request,
                  SetSystemPropertyResponse* reply) override {

    Status status = pc->ExecuteSetSystemPropertyCommand(request, reply);
    
    return status;
  }
};

void RunGrpcServer() {
  pc = new CommandProcessor();

  std::string server_address(GRPC_SERVER_ADDRESS);
  PosCliServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}