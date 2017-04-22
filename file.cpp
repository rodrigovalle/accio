#include "file.hpp"
#include "socket.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

File::File(int fd) : fd(fd) {}
File::File(File&& other) {
  fd = other.fd;
  other.fd = -1;
}
File::~File() {
  close(fd);
}

File& File::operator=(File&& other) {
  fd = other.fd;
  other.fd = -1;
  return *this;
}

File File::open(const std::string& file, int flags) {
  int fd;
  fd = ::open(file.c_str(), flags);
  if (fd < 0) {
    throw std::runtime_error("open(): " + std::string(strerror(errno)));
  }
  return File{fd};
}

void File::write_all(const char *buf, size_t nbytes) {
  size_t total = 0;
  ssize_t n;

  while (total < nbytes) {
    if ((n = ::write(fd, buf + total, nbytes - total)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("write(): " + std::string(strerror(errno)));
    }
    total += n;
  }
}

void File::sendfile(ConnectedSocket& sock) {
  struct stat info;
  if (fstat(fd, &info) == -1) {
    throw std::runtime_error("fstat(): " + std::string(strerror(errno)));
  }

  size_t sent = 0;
  size_t size = info.st_size;
  ssize_t n;

  while (sent < size) {
    if ((n = ::sendfile(sock.sockfd, fd, NULL, size - sent)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("sendfile(): " + std::string(strerror(errno)));
    }
    sent += n;
  }
}

File File::open_r(const std::string& file) {
  return File::open(file, O_RDONLY);
}

File File::create_w(const std::string& file) {
  return File::open(file, O_RDONLY | O_CREAT | O_TRUNC);
}
