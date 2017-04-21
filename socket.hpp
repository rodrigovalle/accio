#include <string>
#include <stdexcept>

#define BACKLOG 10
#define SOCKBUF BUFSIZ

class sock_closed : public std::exception {};

class ConnectedSocket;
class ListeningSocket {
 public:
  ListeningSocket(const std::string& port);
  ~ListeningSocket();
  ListeningSocket(const ListeningSocket& that) = delete;
  ConnectedSocket accept();

 private:
  int sockfd;
};

class ConnectedSocket {
 friend ListeningSocket;

 public:
  ConnectedSocket(const std::string& host, const std::string& port);
  ~ConnectedSocket();
  ConnectedSocket(const ListeningSocket& that) = delete;
  std::string recv();
  void send(std::string& msg);

 private:
  ConnectedSocket(int fd);
  int sockfd;
  char buf[SOCKBUF];
};
