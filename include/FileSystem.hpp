#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
struct PathA {
  fs::path Path;
  bool bFolder;
  uint64_t FileSize;
  int64_t nanoSeconds;
};

class FFilesystem {
private:
  fs::path currentFolder;

public:
  explicit FFilesystem(const fs::path &InCurrentFolder = fs::current_path()) : currentFolder(InCurrentFolder) {}

  std::vector<PathA> GetFiles() {
    std::vector<PathA> Result;
    for (auto const &dir_entry : fs::directory_iterator(currentFolder)) {
      PathA path;
      path.Path = fs::relative(dir_entry);
      path.bFolder = dir_entry.is_directory();
      if (!path.bFolder) {
        path.FileSize = fs::file_size(path.Path);
        path.nanoSeconds = fs::last_write_time(path.Path).time_since_epoch().count();
        Result.emplace_back(path);
      }
    }
    std::cout << Result.size() << std::endl;
    return Result;
  }
  void PrintCurrentFolder() {
    std::cout << currentFolder << std::endl;
    for (auto const &dir_entry : fs::recursive_directory_iterator(currentFolder))
      std::cout << fs::relative(dir_entry, currentFolder) << std::endl;
  }

  static uint64_t FileSize(const std::string& path) { return fs::file_size(path); }

  static std::vector<char> ReeadFIle(const std::string& path, int64_t offset) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(offset, std::ios::beg);
    std::vector<char> buffer(std::min(65536l,size-offset));
    if (file.read(buffer.data(), size)) {
      return buffer;
    }
    return {};
  }
  static void write(const std::string& path, int64_t offset, const std::string& part) {
    std::ofstream file(path, std::ios::binary);
    file.seekp(offset, std::ios::end);
    file.write(part.c_str(), part.size());
  }
};