#include <string>
#include <sys/types.h>

class ConnectedSocket;
class FileDescriptor {
 public:
  FileDescriptor() = delete;
  FileDescriptor(const FileDescriptor&) = delete;
  FileDescriptor(FileDescriptor&&);
  ~FileDescriptor();

  FileDescriptor& operator=(const FileDescriptor&) = delete;
  FileDescriptor& operator=(FileDescriptor&&);

  void write_all(const char *buf, size_t nbyte);
  void sendfile(ConnectedSocket& sock);

  static FileDescriptor open_r(const std::string& file);
  static FileDescriptor create_w(const std::string& file);
  static FileDescriptor openat(FileDescriptor dir, 

 private:
  static FileDescriptor open(const std::string& file, int flags);
  FileDescriptor(int fd);
  int fd;
};
