#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <sys/types.h>

class ConnectedSocket;
class FileDescriptor {
 public:
  FileDescriptor() : FileDescriptor(-1) {};
  FileDescriptor(const FileDescriptor&) = delete;
  FileDescriptor(FileDescriptor&&);
  ~FileDescriptor();

  FileDescriptor& operator=(const FileDescriptor&) = delete;
  FileDescriptor& operator=(FileDescriptor&&);

  void write_all(const std::string& data);
  void sendfile(ConnectedSocket& sock);

  static FileDescriptor open_r(const std::string& file);
  static FileDescriptor create_w(const std::string& file);
  static FileDescriptor opendir(const std::string& dir);
  static FileDescriptor openat_cw(const FileDescriptor& dir,
                                  const std::string& file);

 private:
  static FileDescriptor open(const std::string& file, int flags);
  static FileDescriptor openat(const FileDescriptor& dir,
                               const std::string& file, int flags, int mode);
  FileDescriptor(int fd);
  int fd;
};

#endif // FILE_HPP
