#pragma once

#include <protos/Files.grpc.pb.h>
#include <protos/Files.pb.h>
#include <protos/User.pb.h>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <agrpc/asio_grpc.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/write.hpp>

#include "protos/Transfer.grpc.pb.h"
#include "protos/User.grpc.pb.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/error.hpp>

#include <chrono>
#include <map>

namespace asio = boost::asio;

using boost::system::error_code;
template <auto RequestRPC> using AwaitableServerRPC = boost::asio::use_awaitable_t<>::as_default_on_t<agrpc::ServerRPC<RequestRPC>>;

inline constexpr asio::use_awaitable_t<::agrpc::GrpcExecutor> USE_AWAITABLE{};

class FUsers {
  using TimePoint_t = std::chrono::time_point<std::chrono::system_clock>;

public:
  FUsers() {}
  void AddUser(const std::string &User) {
    std::lock_guard<std::mutex> Guard(MapMutex);
    UserMap.emplace(User, GetTime());
  };
  bool CheckUser(const std::string &User, bool LastActionIO = true) {
    std::lock_guard<std::mutex> Guard(MapMutex);
    if (!UserMap.contains(User)) {
      return false;
    }
    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(GetTime() - UserMap[User]);
    if (elapsedTime > std::chrono::seconds(30)) {
      if (!LastActionIO) {
        UserMap.erase(User);
      } else {
        UserMap[User] = GetTime();
      }
      return LastActionIO;
    }
    if (LastActionIO) {
      UserMap[User] = GetTime();
    }
    return true;
  };
  size_t size(){
    std::lock_guard<std::mutex> Guard(MapMutex);
    return UserMap.size();
  }
  void RemoveUser(const std::string &User) {
    std::lock_guard<std::mutex> Guard(MapMutex);
    UserMap.erase(User);
  };

  auto Map(){
    return UserMap;
  }

private:
  TimePoint_t GetTime() const { return std::chrono::system_clock::now(); }
  std::map<std::string, TimePoint_t> UserMap;
  std::mutex MapMutex;
};

FUsers UserMap;

std::map<std::string, bool> FileRequestMap;
std::map<std::string, std::optional<Directory>> FileMap;

std::multimap<std::pair<std::string, std::string>, ::FilePartInfo> TransferMap;