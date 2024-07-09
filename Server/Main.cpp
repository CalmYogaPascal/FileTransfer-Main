#include <iostream>
#include <optional>
#include <thread>

#include "Common.hpp"
#include "Transfer.hpp"
#include "User.hpp"

namespace UserFileSystemInfoOp {
using SendFileInfoRPC = AwaitableServerRPC<&::UserFileSystemInfo::AsyncService::RequestSendFileInfo>;
asio::awaitable<void> SendFileInfo(SendFileInfoRPC &ptr) {

  Directory request;
  google::protobuf::Empty response;
  while (UserMap.contains(ptr.context().peer())) {

    co_await ptr.read(request);
    FileMap[ptr.context().peer()] = Directory(request);
    for (int i = 0; i < request.file_size(); i++) {
      std::cout << ptr.context().peer() << " __ " << request.file(i).name() << std::endl;
    }
    FileRequestMap[ptr.context().peer()] = false;
    while (FileRequestMap.contains(ptr.context().peer()) && !FileRequestMap[ptr.context().peer()]) {

      agrpc::Alarm alarm{ptr.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::milliseconds(100), asio::use_awaitable);
    }
    if (FileRequestMap.contains(ptr.context().peer())) {
      if (!co_await ptr.write(response)) {
        co_return;
      }
    }
  }
  if (!UserMap.contains(ptr.context().peer())) {
    std::cout<<"Canceled send file info"<<ptr.context().peer()<<std::endl;
    co_await ptr.finish(::grpc::Status::CANCELLED);
    co_return;
  }
  co_await ptr.write(response, grpc::WriteOptions{}.set_last_message());
  co_await ptr.finish(grpc::Status::OK);
}

using FilemapRequestRPC = AwaitableServerRPC<&::UserFileSystemInfo::AsyncService::RequestRequest>;
asio::awaitable<void> Request(FilemapRequestRPC &ptr, UserInfo request) {

  Directory response;
  if (!UserMap.contains(ptr.context().peer())) {
    std::cout<<"Canceled request"<<ptr.context().peer()<<std::endl;
    co_await ptr.finish_with_error(grpc::Status::CANCELLED);
    co_return;
  }

  FileMap[request.login()] = {};
  FileRequestMap[request.login()] = true;
  while (!FileMap[request.login()].has_value()) {

    agrpc::Alarm alarm{ptr.get_executor().context()};
    co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
  }
  response.CopyFrom(FileMap[request.login()].value());
  co_await ptr.finish(response, grpc::Status::OK);
}
} // namespace UserFileSystemInfoOp
// using Channel = asio::experimental::channel<void(boost::system::error_code,
// FileInfoRPC::Request)>;

int main() {
  std::string server_address("0.0.0.0:50051");

  UserAuthorization::AsyncService service;
  UserFileSystemInfo::AsyncService service2;
  FileTransfer::AsyncService service3;

  ::grpc::ServerBuilder builder;
  agrpc::GrpcContext grpc_context{builder.AddCompletionQueue()};

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  builder.RegisterService(&service2);
  builder.RegisterService(&service3);

  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();

  asio::io_context io_context{1};
  std::optional guard{asio::require(io_context.get_executor(), asio::execution::outstanding_work_t::tracked)};

  try {
    agrpc::register_awaitable_rpc_handler<Users::ConnectRPC>(grpc_context, service, Users::Connect, asio::detached);
    agrpc::register_awaitable_rpc_handler<Users::DisconnectRPC>(grpc_context, service, Users::Disconnect, asio::detached);
    // agrpc::register_awaitable_rpc_handler<PingRPC>(
    //     grpc_context, service, UserAuthorization_Ping, asio::detached);
    agrpc::register_awaitable_rpc_handler<Users::GetUsersRPC>(grpc_context, service, Users::GetUsers, asio::detached);

    agrpc::register_awaitable_rpc_handler<UserFileSystemInfoOp::SendFileInfoRPC>(grpc_context, service2, UserFileSystemInfoOp::SendFileInfo, asio::detached);
    agrpc::register_awaitable_rpc_handler<UserFileSystemInfoOp::FilemapRequestRPC>(grpc_context, service2, UserFileSystemInfoOp::Request, asio::detached);

    agrpc::register_awaitable_rpc_handler<FileTransferOp::DownloadRPC>(grpc_context, service3, FileTransferOp::Download, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::UploadRPC>(grpc_context, service3, FileTransferOp::Upload, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::ListenerRPC>(grpc_context, service3, FileTransferOp::Listener, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::ProgressRPC>(grpc_context, service3, FileTransferOp::Progress, asio::detached);

    while (true) {
      std::cout << "Perf" << std::endl;
      // grpc_context.poll();
      // std::this_thread::sleep_for(std::chrono::seconds(1));
      grpc_context.run_until(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }
  } catch (std::exception &Exc) {
    std::cout << Exc.what() << std::endl;
  }
  return 0;
}