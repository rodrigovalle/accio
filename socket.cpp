#include "socket.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#include <cstring>

// Ugly hack to be POSIX compliant
#if EAGAIN == EWOULDBLOCK
#define ESOCKWOULDBLOCK EAGAIN
#else
#define ESOCKWOULDBLOCK EAGAIN: case EWOULDBLOCK
#endif

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

    if (connect(sockfd, res_i->ai_addr, res->ai_addrlen) != 0) {
      cause = "connect(): " + std::string(strerror(errno));
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
    acc += std::string(buf, nbytes);
  }
  if (nbytes == 0) {
    throw sock_closed();
  } else if (nbytes == -1) {
    switch (errno) {
      case ESOCKWOULDBLOCK:
        break;
      default:
        throw std::runtime_error("recv(): " + std::string(strerror(errno)));
    }
  }

  return acc;
}

void ConnectedSocket::send(std::string& msg) {
  ssize_t nbytes = 0;
  size_t total = 0;
  std::string acc;
  const char *strbuf = msg.c_str();

  while (total < msg.length()) {
    if ((nbytes = ::send(sockfd, strbuf + total, msg.length(), 0)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("send(): " + std::string(strerror(errno)));
    }
    total += nbytes;
  }
}
