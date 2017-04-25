#include "socket.hpp"

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

/* TODO: these helper functions should probably be a static function of some
 *       Socket superclass */
void set_socket_sndtimeout(int sockfd) {
  // affects connect() and send()
  struct timeval val;
  val.tv_sec = TIMEOUT/2;  // XXX: make client connect timeout 10s (kernel bug?)
  val.tv_usec = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val)) == -1) {
    throw std::runtime_error("setsockopt(SO_SNDTIMEO): " +
                             std::string(strerror(errno)));
  }
}

void set_socket_rcvtimeout(int sockfd) {
  // affects accept() and recv()
  struct timeval val;
  val.tv_sec = TIMEOUT;
  val.tv_usec = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val)) == -1) {
    throw std::runtime_error("setsockopt(SO_RCVTIMEO): " +
                             std::string(strerror(errno)));
  }
}

void set_socket_reuseaddr(int sockfd) {
  int val = REUSEADDR;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
    throw std::runtime_error("setsockopt(SO_REUSEADDR): " +
                             std::string(strerror(errno)));
  }
}

ListeningSocket::ListeningSocket(const std::string& port) {
  struct addrinfo hints = {0};
  struct addrinfo *res, *res_i;

  int err;
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

    try {
      set_socket_reuseaddr(sockfd);
    } catch (std::runtime_error& e) {
      cause = e.what();
      close(sockfd);
      sockfd = -1;
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
  int connfd;

  while (1) {
    connfd = ::accept(sockfd, nullptr, nullptr);
    if (connfd == -1) {
      switch (errno) {
        case EINTR:
          continue;
        default:
          throw std::runtime_error("accept(): " + std::string(strerror(errno)));
      }
    }
    set_socket_rcvtimeout(connfd);  // TODO: server class should set timeout
    return ConnectedSocket{connfd};
  }
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

    try {
      set_socket_sndtimeout(sockfd);
    } catch (std::runtime_error& e) {
      cause = e.what();
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

ConnectedSocket::ConnectedSocket(ConnectedSocket&& other) {
  sockfd = other.sockfd;
  other.sockfd = -1;
}

ConnectedSocket::~ConnectedSocket() { close(sockfd); }

ConnectedSocket& ConnectedSocket::operator=(ConnectedSocket&& other) {
  sockfd = other.sockfd;
  other.sockfd = -1;
  return *this;
}

std::string ConnectedSocket::recv() {
  ssize_t nbytes;

  while (1) {
    nbytes = ::recv(sockfd, buf, SOCKBUF, 0);
    if (nbytes == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EAGAIN:
          throw socket_timeout_error();
        default:
          throw std::runtime_error("recv(): " + std::string(strerror(errno)));
      }
    }
    else if (nbytes == 0) {
      throw socket_closed_exception();
    }
    return std::string(buf, nbytes);
  }
}
