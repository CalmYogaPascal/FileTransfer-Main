#pragma once
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/write.hpp>
#include <grpc++/grpc++.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <memory>

#include <agrpc/asio_grpc.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <grpc++/grpc++.h>

#include "protos/Files.grpc.pb.h"
#include "protos/Transfer.grpc.pb.h"
#include "protos/User.grpc.pb.h"
#include <FileSystem.hpp>
#include <functional>

namespace asio = boost::asio;

inline bool operator<(const File &lhs, const File &rhs) {
  if (lhs.name() != rhs.name()) {
    return lhs.name() < rhs.name();
  }
  return false;
}
using FsRequestType = std::set<::File>;

template <auto PrepareAsync> using AwaitableClientRPC = boost::asio::use_awaitable_t<>::as_default_on_t<agrpc::ClientRPC<PrepareAsync>>;

void wait(const boost::system::error_code & /*e*/) {}
asio::awaitable<void> UserAuthorization_Connect(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub,
                                                UserFileSystemInfo::Stub &UserFileSystemInfoStub, ::FileTransfer::Stub &InFileTransferStub);
asio::awaitable<void> UserAuthorization_Disconnect(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub);

asio::awaitable<FsRequestType> UserFileSystemInfo_Request(agrpc::GrpcContext &grpc_context, ::UserFileSystemInfo::Stub &stub,
                                                          std::string Login);
asio::awaitable<std::set<std::string>> UserAuthorization_GetUsers(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub);

inline bool operator<(const FileTransferRequestInit &lhs, const FileTransferRequestInit &rhs) {
  if (lhs.srcuser().login() != rhs.srcuser().login()) {
    return lhs.srcuser().login() < rhs.srcuser().login();
  }
  if (lhs.dstuser().login() != rhs.dstuser().login()) {
    return lhs.dstuser().login() < rhs.dstuser().login();
  }
  return lhs.srcfile().name() > rhs.srcfile().name();
}

namespace FileTransferOp {
class TransferOperator {
public:
  TransferOperator(){

  };
  void AddTransfer(const FileTransferRequestInit &InRequest) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    TransferMap.emplace(InRequest);
    return;
  }

  std::optional<FileTransferRequestInit> GetTransfer() {
    std::lock_guard<std::mutex> guard(OperationMutex);
    if (TransferMap.begin() == TransferMap.end())
      return {};
    return *TransferMap.begin();
  }

  bool RemoveTransfer(::FileTransferRequestInit &Request) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    for (auto It = TransferMap.begin(); It != TransferMap.end(); It++) {
      if (It->srcuser().login() != Request.srcuser().login())
        continue;
      if (It->dstuser().login() != Request.dstuser().login())
        continue;
      if (It->srcfile().name() != Request.srcfile().name())
        continue;
      if (It->dstpath().name() != Request.dstpath().name())
        continue;
      TransferMap.erase(It);
      return true;
    }
    return false;
  }

  void AddTransferSource(const std::string &InFrom, const FilePartInfo &InData) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    SourceFiles.emplace(InFrom, InData);
    return;
  }

  bool GetTransferSource(const std::string &InFrom, FilePartInfo &InData) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    for (auto It = SourceFiles.begin(); It != SourceFiles.end(); It++) {
      if (It->first != InFrom)
        continue;

      FilePartInfo &FileData = It->second;

      if (FileData.file() != InData.file())
        continue;
      if (FileData.offset() != InData.offset())
        continue;

      InData.Swap(&InData);
      SourceFiles.erase(It);

      return true;
      break;
    }
    return false;
  }

private:
  std::mutex OperationMutex;
  std::set<FileTransferRequestInit> TransferMap;

  std::multimap<std::string, ::FilePartInfo> SourceFiles;
};

static TransferOperator trop;

asio::awaitable<void> Upload(agrpc::GrpcContext &grpc_context, ::FileTransfer::Stub &stub) {
  using RPC = AwaitableClientRPC<&::FileTransfer::Stub::PrepareAsyncFileTransferProcessUpload>;

  RPC ptr{grpc_context};
  ptr.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3600));
  google::protobuf::Empty resp;
  if (!co_await ptr.start(stub, resp)) {
    const grpc::Status status = co_await ptr.finish();
    std::cerr << "Rpc failed: " << status.error_message();
    co_return;
  }
  FilePartInfo res;
  // bool read_ok = false;
  while (true) {
    std::optional<::FileTransferRequestInit> CurrentTransfer = {};
    while (!CurrentTransfer.has_value()) {
      agrpc::Alarm alarm{ptr.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
      CurrentTransfer = trop.GetTransfer();
    }
    res.set_file(CurrentTransfer.value().srcfile().name());
    uint64_t offset = 0;
    do {
      auto resutl = FFilesystem::ReeadFIle(CurrentTransfer.value().srcfile().name(), offset);
      res.set_offset(offset);
      res.set_part(std::string(resutl.data(), resutl.size()));
      co_await ptr.write(res);
      offset += resutl.size();
    } while (offset < FFilesystem::FileSize(CurrentTransfer.value().srcfile().name()));
    trop.RemoveTransfer(CurrentTransfer.value());
  }

  co_await ptr.finish();
  co_return;
};

asio::awaitable<void> Download(agrpc::GrpcContext &grpc_context, ::FileTransfer::Stub &stub) {
  using RPC = AwaitableClientRPC<&::FileTransfer::Stub::PrepareAsyncFileTransferProcessDownload>;

  RPC ptr{grpc_context};
  ptr.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3600));
  google::protobuf::Empty resp;
  if (!co_await ptr.start(stub, resp)) {
    const grpc::Status status = co_await ptr.finish();
    std::cerr << "Rpc failed: " << status.error_message();
    co_return;
  }
  FilePartInfo res;
  // bool read_ok = false;
  // google::protobuf::Empty req;
  while (co_await ptr.read(res)) {
    FFilesystem::write(res.file(), res.offset(), res.part());
  }
  co_await ptr.finish();
  co_return;
};

asio::awaitable<void> Listener(agrpc::GrpcContext &grpc_context, ::FileTransfer::Stub &stub) {
  using RPC = AwaitableClientRPC<&::FileTransfer::Stub::PrepareAsyncFileTransferListener>;

  RPC ptr{grpc_context};
  ptr.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3600));

  if (!co_await ptr.start(stub)) {
    const grpc::Status status = co_await ptr.finish();
    std::cerr << "Rpc failed: " << status.error_message();
    co_return;
  }
  google::protobuf::Empty request;
  FileTransferRequestInit result;
  bool bWrite = true;
  bool bRead = true;
  while (bWrite && bRead) {
    bWrite = co_await ptr.write(request);
    if (!bWrite)
      break;
    bRead = co_await ptr.read(result);
    trop.AddTransfer(result);
  }
  co_await ptr.finish();
};

asio::awaitable<void> Progress(::agrpc::GrpcContext &grpc_context, ::FileTransfer::Stub &stub, ::FileTransferRequestInit request) {
  using RPC = AwaitableClientRPC<&::FileTransfer::Stub::PrepareAsyncFIleTransferProgress>;

  RPC ptr{grpc_context};
  ptr.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));

  FileTransferProgress result;
  if (!co_await ptr.start(stub, request)) {
    const grpc::Status status = co_await ptr.finish();
    std::cerr << "Rpc failed: " << status.error_message();
    co_return;
  }
  co_await ptr.read(result);
  co_await ptr.finish();
};
} // namespace FileTransferOp
class GrpcTransferContext {
public:
  GrpcTransferContext() { Connect("localhost:50051"); }
  void GetConnectedList(std::function<void(std::exception_ptr, std::set<std::string>)> func) {
    std::cout << "Spawning Task" << std::endl;
    asio::co_spawn(grpc_context, UserAuthorization_GetUsers(grpc_context, *UserAuthorizationStub), func);
  }

  void GetFileMap(const std::string& login, std::function<void(std::exception_ptr, FsRequestType)> func) {
    asio::co_spawn(grpc_context, UserFileSystemInfo_Request(grpc_context, *UserFileSystemInfoStub, login), func);
  }

  void FileTransferRequest(const FileTransferRequestInit& request) {
    asio::co_spawn(grpc_context, FileTransferOp::Progress(grpc_context, *FileTransferStub,request), asio::detached);
  }
  void Connect(const char *connData) {

    if (Channel.get()) {
      std::cout << "Disconnect: " << Channel.use_count() << std::endl;
      Disconnect();
    }
    std::cout << "Creating channel" << std::endl;

    Channel = grpc::CreateChannel(connData, grpc::InsecureChannelCredentials());

    UserAuthorizationStub = UserAuthorization::NewStub(Channel);
    UserFileSystemInfoStub = ::UserFileSystemInfo::NewStub(Channel);
    FileTransferStub = ::FileTransfer::NewStub(Channel);

    asio::co_spawn(grpc_context,
                   UserAuthorization_Connect(grpc_context, *UserAuthorizationStub, *UserFileSystemInfoStub, *FileTransferStub),
                   boost::asio::detached);
    poll();
    poll();
  }

  void Disconnect() {
    if (Channel) {
      asio::co_spawn(grpc_context, UserAuthorization_Disconnect(grpc_context, *UserAuthorizationStub), boost::asio::detached);
      poll();
    }
    std::cout << "Disconnecting..." << std::endl;
    Channel.reset();
    UserAuthorizationStub.reset();
    UserFileSystemInfoStub.reset();
    grpc_context.stop();
    grpc_context.reset();
  }
  bool poll() { return grpc_context.poll(); }

private:
  std::shared_ptr<grpc::Channel> Channel = nullptr;
  std::unique_ptr<UserAuthorization::Stub> UserAuthorizationStub;
  std::unique_ptr<UserFileSystemInfo::Stub> UserFileSystemInfoStub;
  std::unique_ptr<FileTransfer::Stub> FileTransferStub;

  ::grpc::CompletionQueue cq;
  ::grpc::ClientContext clientContext;

  agrpc::GrpcContext grpc_context;
};

asio::awaitable<void> UserFileSystemInfo_SendFileInfo(agrpc::GrpcContext &grpc_context, ::UserFileSystemInfo::Stub &stub);
asio::awaitable<void> UserAuthorization_Ping(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub) {
  using RPC = AwaitableClientRPC<&::UserAuthorization::Stub::PrepareAsyncPing>;

  RPC::Response response;
  RPC rpc{grpc_context};
  rpc.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(30));
  if (!co_await rpc.start(stub, response)) {
    const grpc::Status status = co_await rpc.finish();
    std::cerr << "Rpc failed: " << status.error_message() << std::endl;
    co_return;
  }

  while (true) {

    RPC::Request request;

    const bool write_ok = co_await rpc.write(request);
    if (!write_ok) {
      std::cout << "Notgood\n" << std::endl;
      break;
    }
  }

  const grpc::Status status = co_await rpc.finish();

  if (!status.ok()) {
    std::cout << "Finished not good\n" << std::endl;
  }
  std::cout << "Don ping" << std::endl;
}

asio::awaitable<std::set<std::string>> UserAuthorization_GetUsers(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub) {
  using RPC = AwaitableClientRPC<&::UserAuthorization::Stub::PrepareAsyncGetUsers>;
  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(30));

  RPC::Request request;
  RPC::Response response;
  const auto status = co_await RPC::request(grpc_context, stub, client_context, request, response);

  std::cout << "COnnectedLst " << response.users_size() << std::endl;
  std::set<std::string> Reunit;
  for (int I = 0; I < response.users_size(); I++) {
    Reunit.emplace(response.users(I).login());
  }
  co_return Reunit;
}

asio::awaitable<void> UserAuthorization_Connect(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub,
                                                UserFileSystemInfo::Stub &UserFileSystemInfoStub,
                                                ::FileTransfer::Stub &InFileTransferStub) {
  using RPC = AwaitableClientRPC<&::UserAuthorization::Stub::PrepareAsyncConnect>;
  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(30));
  RPC::Request request;
  RPC::Response response;
  const auto status = co_await RPC::request(grpc_context, stub, client_context, request, response);

  std::cout << "Connection status " << (status.ok() ? std::string("Ok") : status.error_message()) << std::endl;
  asio::co_spawn(grpc_context, UserFileSystemInfo_SendFileInfo(grpc_context, UserFileSystemInfoStub), boost::asio::detached);
  //   asio::co_spawn(grpc_context, UserAuthorization_Ping(grpc_context, stub),
  //                  boost::asio::detached);
  asio::co_spawn(grpc_context, FileTransferOp::Listener(grpc_context, InFileTransferStub), boost::asio::detached);
  asio::co_spawn(grpc_context, FileTransferOp::Download(grpc_context, InFileTransferStub), boost::asio::detached);
  asio::co_spawn(grpc_context, FileTransferOp::Upload(grpc_context, InFileTransferStub), boost::asio::detached);
  co_return;
}

asio::awaitable<void> UserAuthorization_Disconnect(agrpc::GrpcContext &grpc_context, ::UserAuthorization::Stub &stub) {
  using RPC = AwaitableClientRPC<&::UserAuthorization::Stub::PrepareAsyncDisconnect>;
  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  RPC::Request request;
  RPC::Response response;

  const auto status = co_await RPC::request(grpc_context, stub, client_context, request, response);

  co_return;
}

asio::awaitable<void> UserFileSystemInfo_SendFileInfo(agrpc::GrpcContext &grpc_context, ::UserFileSystemInfo::Stub &stub) {
  using RPC = AwaitableClientRPC<&::UserFileSystemInfo::Stub::PrepareAsyncSendFileInfo>;
  /// grpc::ClientContext client_context;

  RPC ptr{grpc_context};
  ptr.context().set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3600));
  Directory request;

  if (!co_await ptr.start(stub)) {
    const grpc::Status status = co_await ptr.finish();
    std::cerr << "Rpc failed: " << status.error_message();
    co_return;
  }
  bool write_ok = true;
  bool read_ok = true;
  google::protobuf::Empty response;
  std::cout << "fs_result" << (write_ok ? "Yes" : "No") << (read_ok ? "Yes" : "No") << std::endl;
  while (write_ok && read_ok) {
    std::cout << "fs_result" << (write_ok ? "Yes" : "No") << (read_ok ? "Yes" : "No") << std::endl;
    FFilesystem fs;
    std::cout << "fs_result" << (write_ok ? "Yes" : "No") << (read_ok ? "Yes" : "No") << std::endl;
    auto Files = fs.GetFiles();
    std::cout << "fs_result" << (write_ok ? "Yes" : "No") << (read_ok ? "Yes" : "No") << std::endl;
    std::cout << Files.size() << std::endl;
    for (const auto &File : Files) {
      std::cout << File.Path << std::endl;
      ::File *file = request.add_file();

      file->set_name(File.Path);
      file->set_filesize(File.FileSize);
      file->set_lastdate(File.nanoSeconds);
    }

    std::cout << "fs_result sending" << std::endl;
    write_ok = co_await ptr.write(request);
    std::cout << "fs_result sent" << std::endl;
    read_ok = co_await ptr.read(response);
  }
  std::cout << "fs_result ended" << std::endl;
  // const auto status = co_await RPC::request(grpc_context, stub,
  // client_context, request, response);
  co_return;
}

asio::awaitable<std::set<::File>> UserFileSystemInfo_Request(agrpc::GrpcContext &grpc_context, ::UserFileSystemInfo::Stub &stub,
                                                             std::string login) {
  using RPC = AwaitableClientRPC<&::UserFileSystemInfo::Stub::PrepareAsyncRequest>;
  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  RPC::Request request;
  request.set_login(login);
  RPC::Response response;

  const auto status = co_await RPC::request(grpc_context, stub, client_context, request, response);

  std::set<::File> Reunit;
  for (int I = 0; I < response.file_size(); I++) {
    Reunit.emplace(response.file(I));
  }
  co_return Reunit;
}
