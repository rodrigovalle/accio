#include <string>
#include <stdexcept>
#include <time.h>

#define BACKLOG 10
#define SOCKBUF BUFSIZ
#define TIMEOUT 10

static struct timeval timeout;
timeout.tv_sec = TIMEOUT;
timeout.tv_usec = 0;

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

  /* Sends the entire string over the connection.
   * TODO: fix the SOCK_NONBLOCK case */
  void send(std::string& msg);

 private:
  ConnectedSocket(int fd);
  int sockfd;
  char buf[SOCKBUF];
};
