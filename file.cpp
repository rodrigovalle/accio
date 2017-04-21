#include "file.hpp"

#include <unistd.h>
#include <fcntl.h>

#include <cstring>


File::File(int fd) : fd(fd) {}
File::File(File&& other) {
  fd = other.fd;
  other.fd = -1;
}
File::~File() { close(fd); }

File& File::operator=(File&& other) {
  fd = other.fd;
  other.fd = -1;
}

File File::open(std::string& file, int flags) {
  int fd;
  fd = ::open(file.c_str(), flags)
  if (fd < 0) {
    throw std::runtime_error("open(): " + std::string(strerror(errno)));
  }
  return FileDescriptor{fd};
}

ssize_t write(const char *buf, size_t nbytes) {
  int r;
  if ((r = ::write(fd, buf, nbytes)) == -1) {
    throw std::runtime_error("write(): " + std::string(strerror(errno)));
  }
  return r;
}

void write_all(const char *buf, size_t nbytes) {
  ssize_t total = 0;
  ssize_t left = nbytes;
  ssize_t n;

  while (total_written < nbytes) {
    if ((n = write(fd, buf + total_written, left)) == -1) {
      throw std::runtime_error("write(): " + std::string(strerror(errno)));
    }
    left -= n;
    total += n;
  }
}

File File::open_r(std::string& file) {
  return File::open(file, O_RDONLY);
}

File File::create_w(std::string& file) {
  return File::open(file, O_RDONLY | O_CREAT | O_TRUNC);
}
