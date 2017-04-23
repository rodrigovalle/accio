#include <string>
#include <stdexcept>

#define BACKLOG 10
#define SOCKBUF BUFSIZ
#define REUSEADDR 1
#define TIMEOUT 5
// TODO: if I set this to 5, socket operations time out in 10 seconds
// I don't know why :(

class sock_closed : public std::exception {};

class ConnectedSocket;
class ListeningSocket {
 public:
  ListeningSocket(const std::string& port);
  ListeningSocket(const ListeningSocket&) = delete;
  ListeningSocket& operator=(const ListeningSocket&) = delete;
  ~ListeningSocket();
  ConnectedSocket accept();

 private:
  int sockfd;
};

class File;
class ConnectedSocket {
 friend ListeningSocket;
 friend File;

 public:
  ConnectedSocket(const std::string& host, const std::string& port);
  ConnectedSocket(const ListeningSocket&) = delete;
  ConnectedSocket& operator=(const ConnectedSocket&) = delete;
  ~ConnectedSocket();
  std::string recv();

 private:
  ConnectedSocket(int fd);
  int sockfd;
  char buf[SOCKBUF];
};
