#include <map>
#include <mutex>
#include <set>

#include "protos/Transfer.grpc.pb.h"
#include "protos/User.grpc.pb.h"

struct TransferOperation {
  std::string To;
  std::string File;
  uint64_t FileSize;
};

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

  std::optional<FileTransferRequestInit> GetTransfer(const std::string &InFrom) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    for (auto It = TransferMap.begin(); It != TransferMap.end(); It++) {
      if (It->srcuser().login() != InFrom)
        continue;
      return *It;
    }
    return {};
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
  std::multimap<std::string, TransferOperation> FileMap;
};

static TransferOperator trop;

using UploadRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFileTransferProcessUpload>;
asio::awaitable<void> Upload(UploadRPC &rpc) {
  if (!co_await rpc.send_initial_metadata()) {
    co_return;
  }
  google::protobuf::Empty response;

  FilePartInfo req;
  bool read_ok = false;
  bool bNoTransfer = true;
  do {
    read_ok = co_await rpc.read(req);
    trop.AddTransferSource(rpc.context().peer(), req);

  } while (read_ok || bNoTransfer);

  co_await rpc.finish(response, ::grpc::Status::OK);
  co_return;
};

using DownloadRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFileTransferProcessDownload>;
asio::awaitable<void> Download(DownloadRPC &rpc, google::protobuf::Empty) {
  FilePartInfo res;
  // bool read_ok = false;
  while (true) {
    std::optional<::FileTransferRequestInit> CurrentTransfer = {};
    while (!CurrentTransfer.has_value()) {
      agrpc::Alarm alarm{rpc.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
      CurrentTransfer = trop.GetTransfer(rpc.context().peer());
    }
    res.set_file(CurrentTransfer.value().srcfile().name());
    do {
      bool bHave = false;
      do {
        bHave = trop.GetTransferSource(CurrentTransfer.value().srcuser().login(), res);
      } while (!bHave);
      co_await rpc.write(res);
      res.set_offset(res.offset() + res.part().size());
    } while (res.offset() < FileMap[CurrentTransfer.value().srcuser().login()].value().file_size());
    trop.RemoveTransfer(CurrentTransfer.value());
  }
  co_await rpc.finish(::grpc::Status::OK);
  co_return;
};

using ListenerRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFileTransferListener>;
asio::awaitable<void> Listener(ListenerRPC &rpc) {
  FileTransferRequestInit response;
  auto It = UserMap.find(rpc.context().peer());
  std::string CurrentUser = *It;
  bool bWrite = false;
  bool bSame = false;
  google::protobuf::Empty req;
  do {
    co_await rpc.read(req);
    std::optional<::FileTransferRequestInit> CurrentTransfer = {};
    while (bSame) {
      agrpc::Alarm alarm{rpc.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
      CurrentTransfer = trop.GetTransfer(CurrentUser);
      if (!CurrentTransfer.has_value()) {
        bSame = true;
        continue;
      }
      bSame = false;
      const auto &The = CurrentTransfer.value();
      if (The.srcuser().login() == response.srcuser().login()) {
        if (The.dstuser().login() == response.dstuser().login()) {
          if (The.srcfile().name() == response.srcfile().name()) {
            if (The.dstpath().name() == response.dstpath().name()) {
              bSame = true;
            }
          }
        }
      }
    }
    bWrite = co_await rpc.write(CurrentTransfer.value());
    response.CopyFrom(CurrentTransfer.value());
  } while (bWrite);
  co_await rpc.finish(::grpc::Status::OK);
};

using ProgressRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFIleTransferProgress>;
asio::awaitable<void> Progress(ProgressRPC &ptr, ProgressRPC::Request &Request) {
  ::FileTransferProgress response;
  trop.AddTransfer(Request);
  co_await ptr.write(response);
  co_await ptr.finish(::grpc::Status::OK);
};
} // namespace FileTransfer