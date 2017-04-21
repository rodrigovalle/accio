#include "file.hpp"

#include <unistd.h>
#include <fcntl.h>

#include <cstring>


File::File(int fd) : fd(fd) {}
File::~File() { close(fd); }

File File::open(std::string& file, int flags) {
  int fd;
  fd = ::open(file.c_str(), flags)
  if (fd < 0) {
    throw std::runtime_error("open(): " + std::string(strerror(errno)));
  }
  return FileDescriptor{fd};
}

File File::open_r(std::string& file) {
  return File::open(file, O_RDONLY);
}

File File::create_w(std::string& file) {
  return File::open(file, O_RDONLY | O_CREAT | O_TRUNC);
}
