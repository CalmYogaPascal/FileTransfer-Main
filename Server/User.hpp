#pragma once
#include "Common.hpp"

namespace Users {
using ConnectRPC = AwaitableServerRPC<&::UserAuthorization::AsyncService::RequestConnect>;
asio::awaitable<void> Connect(ConnectRPC &ptr, ConnectRPC::Request &) {
  google::protobuf::Empty response;

  std::cout << "Connected " << ptr.context().peer() << std::endl;

  UserMap.AddUser(ptr.context().peer());
  FileRequestMap.emplace(ptr.context().peer(), false);
  
  co_await ptr.finish(response, ::grpc::Status::OK, asio::use_awaitable);
}

using DisconnectRPC = AwaitableServerRPC<&::UserAuthorization::AsyncService::RequestDisconnect>;
asio::awaitable<void> Disconnect(DisconnectRPC &ptr, DisconnectRPC::Request &) {
  google::protobuf::Empty response;

  // agrpc::Alarm alarm{ptr.get_executor()};
  std::cout << "Removed " << ptr.context().peer() << std::endl;

  UserMap.RemoveUser(ptr.context().peer());

  co_await ptr.finish(response, ::grpc::Status::OK, asio::use_awaitable);

  co_return;
}

using PingRPC = AwaitableServerRPC<&::UserAuthorization::AsyncService::RequestPing>;
asio::awaitable<void> Ping(PingRPC &rpc) {
  bool read_ok;

  google::protobuf::Empty request;
  do {
    read_ok = false;
    std::cout << "Before " << rpc.context().peer() << std::endl;
    if (UserMap.CheckUser(rpc.context().peer())) {
      read_ok = co_await rpc.read(request);
    }
    std::cout << "Ended" << rpc.context().peer() << std::endl;
  } while (read_ok);

  google::protobuf::Empty response;
  co_await rpc.finish(response, grpc::Status::OK);
}

using GetUsersRPC = AwaitableServerRPC<&::UserAuthorization::AsyncService::RequestGetUsers>;
asio::awaitable<void> GetUsers(GetUsersRPC &ptr, google::protobuf::Empty) {

  UsersInfo Response;

  std::cout << "Requested " << UserMap.size() << std::endl;
  if (!UserMap.CheckUser(ptr.context().peer())) {
    std::cout << "Canceled get users" << ptr.context().peer() << std::endl;
    co_await ptr.finish_with_error(grpc::Status::CANCELLED);
    co_return;
  }
  for (const auto &User : UserMap.Map()) {
    Response.add_users()->set_login(User.first);
  }

  std::cout << "Returned " << UserMap.size() << std::endl;
  co_await ptr.finish(Response, grpc::Status::OK);
}
} // namespace Users