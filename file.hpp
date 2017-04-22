#include <string>
#include <sys/types.h>

class ConnectedSocket;
class File {
 public:
  File() = delete;
  File(const File&) = delete;
  File(File&&);
  ~File();

  File& operator=(const File&) = delete;
  File& operator=(File&&);

  void write_all(const char *buf, size_t nbyte);
  void sendfile(ConnectedSocket& sock);

  static File open_r(const std::string& file);
  static File create_w(const std::string& file);

 private:
  static File open(const std::string& file, int flags);
  File(int fd);
  int fd;
};
