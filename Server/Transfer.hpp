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

inline bool operator==(const FileTransferRequestInit &lhs, const FileTransferRequestInit &rhs) {
  if (lhs.srcuser().login() == rhs.srcuser().login()) {
    if (lhs.dstuser().login() == rhs.dstuser().login()) {
      if (lhs.srcfile().name() == rhs.srcfile().name()) {
        if (lhs.dstpath().name() == rhs.dstpath().name()) {
          return true;
        }
      }
    }
  }
  return false;
}

std::ostream& operator<< (std::ostream& out, FilePartInfo const& curr)
{
   out<<"File ("<<curr.file()<<"): offset"<<curr.offset()<<" "<<curr.part().size();
   return out;
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

  bool GetTransferAsSource(const std::string &InFrom, std::optional<FileTransferRequestInit> &Out) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    for (auto It = TransferMap.begin(); It != TransferMap.end(); It++) {
      if (It->srcuser().login() != InFrom)
        continue;
      std::cout << It->srcuser().login() << " found one " << TransferMap.size() << " " << InFrom << std::endl;
      Out = *It;
      return true;
    }
    return false;
  }

  bool GetTransferAsTarget(const std::string &InTo, std::optional<FileTransferRequestInit> &Out) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    for (auto It = TransferMap.begin(); It != TransferMap.end(); It++) {
      if (It->dstuser().login() != InTo)
        continue;
      std::cout << It->dstuser().login() << " found target one " << TransferMap.size() << " " << InTo << std::endl;
      Out = *It;
      return true;
    }
    return false;
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
    FilePartInfo P(InData);
    SourceFiles.emplace(InFrom, P);
    return;
  }

  bool GetTransferSource(const std::string &InFrom, FilePartInfo &InData) {
    std::lock_guard<std::mutex> guard(OperationMutex);
    int i=0;
    for (auto It = SourceFiles.begin(); It != SourceFiles.end(); It++, i++)
    {
      if(i>2) break;
      std::cout<<It->second<<std::endl;
    }
    for (auto It = SourceFiles.begin(); It != SourceFiles.end(); It++) {
      //std::cout<< It->first <<" from " <<InFrom<<std::endl;
      if (It->first != InFrom)
        continue;

      FilePartInfo &FileData = It->second;


      //std::cout<< FileData.file() <<" from " <<InData.file()<<std::endl;
      if (FileData.file() != InData.file())
        continue;

      //std::cout<< FileData.offset() <<" from " <<InData.offset()<<std::endl;
      if (FileData.offset() != InData.offset())
        continue;

      InData.Swap(&It->second);
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
asio::awaitable<void> Upload(UploadRPC &ptr) {
  google::protobuf::Empty response;

  const auto &peer = ptr.context().peer();
  std::cout << "Initiated Transfer::Upload " << peer << std::endl;
  FilePartInfo req;
  bool read_ok = false;
  //bool bNoTransfer = true;
  int i =0;
  do {
    read_ok = co_await ptr.read(req);
    if(i++<5){
    std::cout <<"Upload: " << req << std::endl;
    }
    trop.AddTransferSource(ptr.context().peer(), req);

  } while (read_ok);

  std::cout << "Finished Transfer::Upload " << peer << std::endl;
  co_await ptr.finish(response, ::grpc::Status::OK);
  co_return;
};

using DownloadRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFileTransferProcessDownload>;
asio::awaitable<void> Download(DownloadRPC &ptr, google::protobuf::Empty) {
  FilePartInfo res;
  const auto &peer = ptr.context().peer();
  std::cout << "Initiated Transfer::Download " << peer << std::endl;
  // bool read_ok = false;
  while (true) {
    std::optional<::FileTransferRequestInit> CurrentTransfer = {};
    while (!trop.GetTransferAsTarget(peer, CurrentTransfer)) {
      agrpc::Alarm alarm{ptr.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
    }
    res.set_file(CurrentTransfer.value().srcfile().name());
    const auto& srcuser = CurrentTransfer.value().srcuser().login();
    File srcfile;
    for(int ii=0;ii<FileMap[srcuser].value().files_size();ii++){
      srcfile = FileMap[srcuser].value().files(ii);
      if (srcfile.name() == CurrentTransfer.value().srcfile().name())
      {
        break;
      }
    }
    do {
      while(!trop.GetTransferSource(srcuser, res)){
        agrpc::Alarm alarm{ptr.get_executor().context()};
        co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);
      }

      std::cout << "Writing Transfer::Download " << peer << " "<< res << std::endl;
      co_await ptr.write(res);
      res.set_offset(res.offset() + res.part().size());
      std::cout << "Writing Transfer::Download " << peer << " "<< res <<" "<< std::endl;
    } while (uint64_t(res.offset()) < srcfile.filesize());
    trop.RemoveTransfer(CurrentTransfer.value());
  }

  std::cout << "Initiated Transfer::Download " << peer << std::endl;
  co_await ptr.finish(::grpc::Status::OK);
  co_return;
};

using ListenerRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFileTransferListener>;
asio::awaitable<void> Listener(ListenerRPC &ptr) {
  FileTransferRequestInit response;

  const auto &peer = ptr.context().peer();
  std::cout << "Initiated Transfer::Listener " << peer << std::endl;

  bool bWrite = false;
  bool bSame = true;
  google::protobuf::Empty req;
  do {
    std::cout << "Reading Transfer::Listener " << peer << std::endl;
    co_await ptr.read(req);

    std::cout << "Read Transfer::Listener " << peer << std::endl;
    std::optional<::FileTransferRequestInit> CurrentTransfer = {};
    do {
      agrpc::Alarm alarm{ptr.get_executor().context()};
      co_await alarm.wait(std::chrono::system_clock::now() + std::chrono::seconds(1), asio::use_awaitable);

      if (!trop.GetTransferAsSource(peer, CurrentTransfer)) {
        bSame = true;
        continue;
      }

      std::cout << (CurrentTransfer.has_value() ? CurrentTransfer.value().srcfile().name() : "No value") << std::endl;
      bSame = false;
      const auto &The = CurrentTransfer.value();
      if (The == response) {
        bSame = true;
      }
      //std::cout << (bSame ? "Same" : "Diff") << " Transfer::Listener " << peer << std::endl;
    } while (bSame);
    std::cout << "Write Transfer::Listener " << peer << std::endl;
    std::cout << "Transfer::Listener: " << CurrentTransfer.value().srcuser().login() << " " << CurrentTransfer.value().dstuser().login()
              << " " << CurrentTransfer.value().srcfile().name() << " " << CurrentTransfer.value().dstpath().name() << std::endl;
    bWrite = co_await ptr.write(CurrentTransfer.value());
    std::cout << "Writed Transfer::Listener " << peer << std::endl;
    response.CopyFrom(CurrentTransfer.value());
    std::cout << "Writed Transfer::Listener " << peer << " " << (bWrite ? "Written" : "Not written") << std::endl;
  } while (bWrite);
  std::cout << "Finished Transfer::Listener " << peer << std::endl;
  co_await ptr.finish(::grpc::Status::OK);
};

using ProgressRPC = AwaitableServerRPC<&::FileTransfer::AsyncService::RequestFIleTransferProgress>;
asio::awaitable<void> Progress(ProgressRPC &ptr, ProgressRPC::Request &InRequest) {
  ::FileTransferProgress response;
  const auto &peer = ptr.context().peer();
  std::cout << "Initiated Transfer::Progress " << peer << std::endl;
  trop.AddTransfer(InRequest);
  co_await ptr.write(response);
  std::cout << "Returned Transfer::Progress " << peer << std::endl;
  co_await ptr.finish(::grpc::Status::OK);
};
} // namespace FileTransferOp