#include <iostream>
#include <optional>
#include <thread>

#include "Common.hpp"
#include "Transfer.hpp"
#include "User.hpp"
#include "Files.hpp"

std::string what(const std::exception_ptr &eptr = std::current_exception()) {
  if (!eptr) {
    throw std::bad_exception();
  }

  try {
    std::rethrow_exception(eptr);
  } catch (const std::exception &e) {
    return e.what();
  } catch (const std::string &e) {
    return e;
  } catch (const char *e) {
    return e;
  } catch (...) {
    return "who knows";
  }
}

void DefaultHandler(const std::exception_ptr& ex) {
  if (ex) {
      std::cout << what(ex) << std::endl;
  }
}

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

    agrpc::register_awaitable_rpc_handler<UserFileSystemInfoOp::SendFileInfoRPC>(grpc_context, service2, UserFileSystemInfoOp::StreamDirectory,
                                                                                 asio::detached);
    agrpc::register_awaitable_rpc_handler<UserFileSystemInfoOp::FilemapRequestRPC>(grpc_context, service2, UserFileSystemInfoOp::RequestFilesFromUser,
                                                                                   asio::detached);

    agrpc::register_awaitable_rpc_handler<FileTransferOp::DownloadRPC>(grpc_context, service3, FileTransferOp::Download, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::UploadRPC>(grpc_context, service3, FileTransferOp::Upload, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::ListenerRPC>(grpc_context, service3, FileTransferOp::Listener, asio::detached);
    agrpc::register_awaitable_rpc_handler<FileTransferOp::ProgressRPC>(grpc_context, service3, FileTransferOp::Progress, asio::detached);

    while (true) {
      std::cout << "Perf" << std::endl;
      //  grpc_context.poll();
      //  std::this_thread::sleep_for(std::chrono::seconds(1));
      grpc_context.run_until(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }
  } catch (std::exception &Exc) {
    std::cout << Exc.what() << std::endl;
  }
  return 0;
}