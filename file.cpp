#include "file.hpp"
#include "socket.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

FileDescriptor::FileDescriptor(int fd) : fd(fd) {}
FileDescriptor::FileDescriptor(FileDescriptor&& other) {
  fd = other.fd;
  other.fd = -1;
}
FileDescriptor::~FileDescriptor() {
  close(fd);
}

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) {
  fd = other.fd;
  other.fd = -1;
  return *this;
}

FileDescriptor FileDescriptor::open(const std::string& file, int flags) {
  int fd;
  fd = ::open(file.c_str(), flags);
  if (fd < 0) {
    throw std::runtime_error{"open(): " + std::string{strerror(errno)}};
  }
  return FileDescriptor{fd};
}

FileDescriptor FileDescriptor::openat(const FileDescriptor& dir,
                                      const std::string& file,
                                      int flags, int mode) {
  int fd;
  fd = ::openat(dir.fd, file.c_str(), flags, mode);
  if (fd < 0) {
    throw std::runtime_error{"openat(): " + std::string{strerror(errno)}};
  }
  return FileDescriptor{fd};
}

void FileDescriptor::write_all(const std::string& data) {
  const char *buf = data.c_str();
  size_t nbytes = data.size();
  size_t total = 0;
  ssize_t n;

  do {
    if ((n = ::write(fd, buf + total, nbytes - total)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error{"write(): " + std::string{strerror(errno)}};
    }
    total += n;
  } while (total < nbytes);
}

void FileDescriptor::sendfile(ConnectedSocket& sock) {
  struct stat info;
  if (fstat(fd, &info) == -1) {
    throw std::runtime_error{"fstat(): " + std::string{strerror(errno)}};
  }

  size_t size = info.st_size;
  size_t sent = 0;
  ssize_t n;

  do {
    if ((n = ::sendfile(sock.sockfd, fd, NULL, size - sent)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error{"sendfile(): " + std::string{strerror(errno)}};
    }
    sent += n;
  } while (sent < size);
}

void FileDescriptor::clear() {
  if (ftruncate(fd, 0) == -1) {
    throw std::runtime_error{"ftruncate(): " + std::string{strerror(errno)}};
  }
  if (lseek(fd, 0, SEEK_SET) == -1) {
    throw std::runtime_error{"lseek(): " + std::string{strerror(errno)}};
  }
}

FileDescriptor FileDescriptor::open_r(const std::string& file) {
  return FileDescriptor::open(file, O_RDONLY);
}

FileDescriptor FileDescriptor::create_w(const std::string& file) {
  return FileDescriptor::open(file, O_RDONLY | O_CREAT | O_TRUNC);
}

FileDescriptor FileDescriptor::opendir(const std::string& dir) {
  return FileDescriptor::open(dir, O_DIRECTORY);
}

FileDescriptor FileDescriptor::openat_cw(const FileDescriptor& dir,
                                         const std::string& file) {
  int flags = O_CREAT | O_WRONLY;
  int mode = S_IWUSR | S_IRUSR;
  return FileDescriptor::openat(dir, file, flags, mode);
}
