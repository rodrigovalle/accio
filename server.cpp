/* 
 * The server listens for TCP connections and saves all the received data
 * from the client in a file.
 *
 *
 * USAGE
 *   ./server <PORT> <FILE-DIR>
 *
 * port:      the port number on which the server will listen to connections;
 *            the server must accept connections coming from any interface
 * file-dir:  directory name where to save the received files
 *
 *
 * REQUIREMENTS
 *   - The server must open a listening socket on the specified port number
 *   - The server should gracefully process incorrect port number and exit
 *     with a nonzero error code (you can assume that the folder is always
 *     correct); in addition, the server must print out an error message
 *     starting with 'ERROR:'
 *   - The server should exit with code zero when receiving SIGQUIT/SIGTERM
 *   - The server should be able to accept and process multiple connections
 *     from clients at the same time
 *   - The server must count all established connections (1-indexed); the
 *     received file over the connection must be saved to
 *     <FILE_DIR>/<CONNECTION_ID>.file
 *   - If the client doesn't send any data during a gracefully terminated TCP
 *     connection, the server should create an empty file with the name that
 *     corresponds to the connection number.
 *   - The server must assume error if no data is received fromt the client
 *     for over 10 seconds; it should abort the connection and write a single
 *     ERROR string (without EOL/carriage return) into the corresponding file,
 *     discarding any partial input
 *   - The server should be able to accept and save files up to 100 MiB
 */
#include "server.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>
#include <fcntl.h>

#include <cstdlib>
#include <iostream>
#include <thread>
#include <string>

#define BACKLOG 10

Server::Server(const std::string& port, const std::string& file_directory)
{
  int r;
  struct addrinfo hints, *res;

  /* check your privilege
   * NOTE: man pages warn that access() has a race condition, but we're not
   *       using it to give permissions, just perform a sanity check */
  dir_fd = open(file_directory.c_str(), O_DIRECTORY);
  if (dir_fd < 0) {
    throw std::runtime_error("invalid directory " + file_directory);
  }
  r = faccessat(dir_fd, ".", W_OK, 0);
  if (r < 0) {
    throw std::runtime_error("no write permissions in " + file_directory);
  }

  /* create a new socket */
  hints.ai_family = AF_UNSPEC;     // Use IPV4 or IPV6
  hints.ai_socktype = SOCK_STREAM; // Use TCP stream sockets
  hints.ai_protocol = 0;           // let getaddrinfo() pick the protocol
  hints.ai_flags = AI_PASSIVE;     // fill in my IP (be interface independent)

  r = getaddrinfo(NULL, port.c_str(), &hints, &res);
  if (r < 0) {
    throw std::runtime_error("getaddrinfo() failed");
  }

  sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock_fd < 0) {
    throw std::runtime_error("socket() failed");
  }

  /* bind to port (SO_REUSEADDR?) */
  r = bind(sock_fd, res->ai_addr, res->ai_addrlen);
  if (r < 0) {
    throw std::runtime_error("bind() failed");
  }

  /* listen */
  r = listen(sock_fd, BACKLOG);
  if (r < 0) {
    throw std::runtime_error("listen() failed");
  }
}

Server::~Server()
{
  /* TODO: error checking on close()... who cares? */
  close(dir_fd);
  close(sock_fd);
}

void Server::start_server()
{
  while (true) {
    int client_fd = accept(sock_fd, NULL, NULL);
    if (client_fd < 0) {
      /* TODO: this error probably shouldn't bring down our whole server */
      throw std::runtime_error("accept() failed");
    }
    send(client_fd, "wheeee\n", 8, 0);
    close(client_fd);
  }
}

int main() {
  try {
    Server s = Server("4000", "filedir");
    s.start_server();
  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
