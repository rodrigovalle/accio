#include <string>

#define READSIZ BUFSIZ

class Client {
 public:
  Client(const std::string& host, const std::string& port);
  ~Client();
  void send_file(const std::string& filename);

 private:
  int sockfd;
  int filefd;
  char *buf[READSIZ];
};
