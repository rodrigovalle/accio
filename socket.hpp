#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <stdexcept>

#define BACKLOG 10
#define SOCKBUF BUFSIZ
#define REUSEADDR 1
#define TIMEOUT 10

class socket_closed_exception : public std::exception {};
class socket_timeout_error : public std::runtime_error {
 public:
  socket_timeout_error() : std::runtime_error("socket timed out") {}
};

class ConnectedSocket;
class ListeningSocket {
 public:
  ListeningSocket(const std::string& port);
  ListeningSocket(const ListeningSocket&) = delete;
  ~ListeningSocket();
  // TODO: declare move constructor & move assignment for ListeningSocket?
  ListeningSocket& operator=(const ListeningSocket&) = delete;
  ConnectedSocket accept();

 private:
  int sockfd;
};

class FileDescriptor;
class ConnectedSocket {
 friend ListeningSocket;
 friend FileDescriptor;

 public:
  ConnectedSocket(const std::string& host, const std::string& port);
  ConnectedSocket(const ListeningSocket&) = delete;
  ConnectedSocket(ConnectedSocket&&);
  ~ConnectedSocket();

  ConnectedSocket& operator=(const ConnectedSocket&) = delete;
  ConnectedSocket& operator=(ConnectedSocket&&);

  std::string recv();

 private:
  ConnectedSocket(int fd);
  int sockfd;
  char buf[SOCKBUF];
};

#endif // SOCKET_HPP
