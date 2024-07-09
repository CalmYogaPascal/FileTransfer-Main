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

#include <map>

namespace asio = boost::asio;

using boost::system::error_code;
std::map<std::string, asio::deadline_timer> DeadLines;
template <auto RequestRPC>
using AwaitableServerRPC = boost::asio::use_awaitable_t<>::as_default_on_t<
    agrpc::ServerRPC<RequestRPC>>;

inline constexpr asio::use_awaitable_t<::agrpc::GrpcExecutor> USE_AWAITABLE{};

std::set<std::string> UserMap;
std::map<std::string, bool> FileRequestMap;
std::map<std::string, std::optional<Directory>> FileMap;

std::multimap<std::pair<std::string, std::string>, ::FilePartInfo> TransferMap;