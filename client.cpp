/*
 * The client connects to the server and as soon as the connection is
 * established, sends the content of a file to the server.
 *
 *
 * USAGE
 *   ./client <HOSTNAME-OR-IP> <PORT> <FILENAME>
 *
 * hostname-or-ip:  hostname or IP address of the server to connect
 * port:            port number of the server to connect
 * filename:        name of the file to transfer to the server after the
 *                  connection is established
 *
 *
 * REQUIREMENTS
 *   - The client must be able to connect to the specified server and port,
 *     transfer the specified file, and gracefully terminate the connection
 *   - The client should gracefully process incorrect hostname and port number
 *     and exit with a non-zero exit code (you can assume that the specified
 *     file is always correct)
 *   - In addition to exiting, the server must print out on standard error an
 *     error message that starts with 'ERROR:'
 *   - The client should exit with code zero after successful transfer of the
 *     file to the server.
 *   - The client should support transfer of files up to 100 MiB in size
 *   - The client should handle connection and transmission errors; the
 *     reaction to network or server errors should be no longer than 10
 *     seconds:
 *       - Timeout to connect to the server should be no longer than 10 seconds
 *       - Timeout for not being able to send more data to server (not being
 *         able to write to send buffer) should be no longer than 10 seconds
 *     Whenever timeout occurs, the client should abort the connection, print
 *     an error string starting with 'ERROR:' to standard error, and exit with
 *     non-zero code
 */
#include "client.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <iostream>
#include <string>

Client::Client(const std::string& host, const std::string& port)
    : sockfd(-1), filefd(-1) {
  struct addrinfo hints = {0};
  struct addrinfo *srv;

  hints.ai_family = AF_INET;        // Looking for server using IPV4
  hints.ai_socktype = SOCK_STREAM;  // Use TCP stream sockets

  getaddrinfo(host.c_str(), port.c_str(), &hints, &srv);
  sockfd = socket(srv->ai_family, srv->ai_socktype, srv->ai_protocol);
  if (sockfd < 0) {
    throw std::runtime_error("socket() failed");
  }

  if (connect(sockfd, srv->ai_addr, srv->ai_addrlen) < 0) {
    throw std::runtime_error("connect() failed");
  }
}

Client::~Client() {
  close(sockfd);
  close(filefd); /* call close in case we errored out */
}

void Client::send_file(const std::string& filename) {
  int nbytes;

  if ((filefd = open(filename.c_str(), O_RDONLY)) < 0) {
    throw std::runtime_error("open() failed");
  }

  while ((nbytes = read(filefd, buf, READSIZ)) > 0) {
    if (send(sockfd, buf, nbytes, 0) < 0) {
      throw std::runtime_error("send() failed");
    }
  }
  if (nbytes < 0) {
    throw std::runtime_error("read() failed");
  }
  close(sockfd);
  close(filefd);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <HOSTNAME-OR-IP> <PORT> <FILENAME>"
              << std::endl;
    return EXIT_FAILURE;
  }
  try {
    Client c(argv[1], argv[2]);
    c.send_file(argv[3]);
  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
