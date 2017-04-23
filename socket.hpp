#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <stdexcept>

#define BACKLOG 10
#define SOCKBUF BUFSIZ
#define REUSEADDR 1
#define TIMEOUT 5
// TODO: if I set this to 5, socket operations time out in 10 seconds
// I don't know why :(

class socket_closed_exception : public std::exception {};

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
