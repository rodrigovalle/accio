#include "socket.hpp"

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

static const struct timeval timeout = {.tv_sec=TIMEOUT, .tv_usec=0};

ListeningSocket::ListeningSocket(const std::string& port) {
  struct addrinfo hints = {0};
  struct addrinfo *res, *res_i;

  int err;
  int reuseaddr = REUSEADDR;
  std::string cause;

  hints.ai_flags = AI_PASSIVE;      // we will bind to this socket
  hints.ai_family = AF_INET;        // use IPV4
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

  err = getaddrinfo(nullptr, port.c_str(), &hints, &res);
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

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(reuseaddr)) == -1) {
      cause = "setsockopt(SO_REUSEADDR): " + std::string(strerror(errno));
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout)) == -1) {
      cause = "setsockopt(SO_SNDTIMEO): " + std::string(strerror(errno));
      continue;
    }

    if (bind(sockfd, res_i->ai_addr, res->ai_addrlen) != 0) {
      cause = "bind(): " + std::string(strerror(errno));
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
    throw std::runtime_error("accept(): " + std::string(strerror(errno)));
  }
  return ConnectedSocket{connfd};
}


ConnectedSocket::ConnectedSocket(const std::string& host,
                                 const std::string& port) {
  struct addrinfo hints = {0};
  struct addrinfo *res, *res_i;
  std::string cause;
  int err;

  hints.ai_family = AF_INET;        // IPV4
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

  err = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
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

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
      cause = "setsockopt(SO_SNDTIMEO): " + std::string(strerror(errno));
      close(sockfd);
      sockfd = -1;
      continue;
    }

    if (connect(sockfd, res_i->ai_addr, res->ai_addrlen) == -1) {
      switch (errno) {
        case EINPROGRESS:
          cause = "connect(): connection timed out";
          break;
        default:
          cause = "connect(): " + std::string(strerror(errno));
          break;
      }
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

ConnectedSocket::ConnectedSocket(int fd) : sockfd(fd) {}

ConnectedSocket::~ConnectedSocket() { close(sockfd); }

std::string ConnectedSocket::recv() {
  ssize_t nbytes;
  std::string acc;

  while ((nbytes = ::recv(sockfd, buf, SOCKBUF, 0)) > 0) {
    if (nbytes == 0) {
      throw sock_closed();
    } else if (nbytes == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EAGAIN:
          throw std::runtime_error("recv(): connection timed out");
        default:
          throw std::runtime_error("recv(): " + std::string(strerror(errno)));
      }
    }
    acc += std::string(buf, nbytes);
  }

  return acc;
}
