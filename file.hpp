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
