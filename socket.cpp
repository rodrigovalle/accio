#include "socket.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

ListeningSocket::ListeningSocket(const std::string& port) {
  struct addrinfo hints = {0};
  struct addrinfo *res, *res_i;
  int err;
  std::string cause;

  hints.ai_flags = AI_PASSIVE;      // we will bind to this socket
  hints.ai_family = AF_INET;        // use IPV4
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

  err = getaddrinfo(nullptr, port.c_str(), &hints, &res)
  if (err) {
    throw std::runtime_error("getaddrinfo(): " +
                             std::string(gai_strerror(err)));
  }

  for (res_i = res; res_i != nullptr; res_i = res_i->ai_next) {
    sockfd = socket(res_i->ai_family, res_i->ai_socktype, res_i->ai_protocol);
    if (sockfd == -1) {
      cause = "socket(): " + std::string(strerror(errno));
      continue;
    }

    if (bind(sockfd, res_i->ai_addr, res->ai_addrlen) != 0) {
      cause = "bind(): " + std::string(sterror(errno));
      close(sockfd);
      sockfd = -1;
      continue;
    }

    if (listen(sockfd, BACKLOG) != 0) {
      cause = "listen(): " + std::string(strerror(errno));
      close(sockfd);
      sockfd = -1;
      continue;
    }

    break;
  }

  freeaddrinfo(res);
  if (res_i == nullptr) {
    throw std::runtime_error(cause);
  }
}

ListeningSocket::~ListeningSocket() {
  close(sockfd);
}

ConnectedSocket ListeningSocket::accept() {
  int connfd = ::accept(sockfd, nullptr, nullptr);
  if (connfd == -1) {
    throw std::runtime_error("accept(): " + strerror(errno));
  }
  return ConnectedSocket{connfd};
}
}
