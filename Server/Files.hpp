#include "Common.hpp"

namespace UserFileSystemInfoOp {
using SendFileInfoRPC = AwaitableServerRPC<&::UserFileSystemInfo::AsyncService::RequestStreamDirectory>;
asio::awaitable<void> StreamDirectory(SendFileInfoRPC &ptr) {

  Directory request;
  google::protobuf::Empty response;
  const auto &peer = ptr.context().peer();
  std::cout << "Fileinfo request" << peer << std::endl;
  while (UserMap.CheckUser(peer)) {

    co_await ptr.read(request);
    FileMap[peer] = Directory(request);
    std::cout << request.files_size() << " files received" << std::endl;

    for (int i = 0; i < request.files_size(); i++) {
      // std::cout << "Files " << peer << " __ " << request.file(i).name() << std::endl;
    }
    FileRequestMap[peer] = false;
    while (FileRequestMap.contains(peer) && !FileRequestMap[peer]) {
      agrpc::Alarm alarm{ptr.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::milliseconds(2000), asio::use_awaitable);
    }
    if (FileRequestMap.contains(peer)) {
      if (!co_await ptr.write(response)) {
        std::cout << "Canceled send file info3 " << peer << std::endl;
        co_return;
      } else {

        std::cout << "Continue info3 " << peer << std::endl;
      }
    }
  }
  if (!UserMap.CheckUser(peer)) {
    std::cout << "Canceled send file info2 " << peer << std::endl;
    co_await ptr.finish(::grpc::Status::CANCELLED);
    co_return;
  }
  std::cout << "Fileinfo streaming ended" << peer << std::endl;
  co_await ptr.write(response, grpc::WriteOptions{}.set_last_message());
  co_await ptr.finish(grpc::Status::OK);
}

using FilemapRequestRPC = AwaitableServerRPC<&::UserFileSystemInfo::AsyncService::RequestRequestFilesFromUser>;
asio::awaitable<void> RequestFilesFromUser(FilemapRequestRPC &ptr, UserInfo request) {

  Directory response;
  const auto &peer = ptr.context().peer();
  std::cout << "File request request " << peer << std::endl;
  if (!UserMap.CheckUser(peer)) {
    std::cout << "Canceled request" << peer << std::endl;
    co_await ptr.finish_with_error(grpc::Status::CANCELLED);
    co_return;
  }

  FileMap[request.login()] = {};
  FileRequestMap[request.login()] = true;
  while (!FileMap[request.login()].has_value()) {
    if (!UserMap.CheckUser(request.login())) {
      std::cout << "File request canceled 1" << peer << std::endl;
      co_await ptr.finish_with_error(grpc::Status::CANCELLED);
      co_return;
    }
    // std::cout << "File request no answer: " << peer << std::endl;
    agrpc::Alarm alarm{ptr.get_executor().context()};
    co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
  }
  response.CopyFrom(FileMap[request.login()].value());

  for (int i = 0; i < response.files_size(); i++) {
    //std::cout << "Response " << i << " | " << peer << " __ " << response.file(i).name() << std::endl;
  }
  std::cout << "File request finished" << peer << std::endl;
  co_await ptr.finish(response, grpc::Status::OK);
}
} // namespace UserFileSystemInfoOp