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
#include "server.hpp"
#include "socket.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

Server::Server(const std::string& port, const std::string& file_directory)
    : sock(port), client_count(0), threads(THREADS) {

  /* check your privilege
   * NOTE: man pages warn that access() has a race condition, but we're not
   *       using it to give permissions, just perform a sanity check */
  if (access(file_directory.c_str(), W_OK) == -1) {
    throw std::runtime_error("no write permissions in " + file_directory);
  }

  dir = FileDescriptor::opendir(file_directory);
}

Server::~Server() {
  /* TODO:
   *  - error checking on close()
   *  - exit all running threads
   *  - close all open files      */

  /* wait for all threads to finish */
  for (std::thread& t : threads) {
    t.join();
  }
}

void Server::start() {
  while (true) {
    ConnectedSocket client_conn = sock.accept();

    /* create a new thread to handle the connection and continue waiting for
     * new connections */
    threads.emplace_back(&Server::recv_file, this, std::move(client_conn), client_count);
    client_count++;
  }
}

void Server::recv_file(ConnectedSocket client, int client_id) {
  std::string fname = std::to_string(client_id) + ".file";
  FileDescriptor outfile = FileDescriptor::openat_cw(dir, fname);

  try {
    while (1) {
      std::string chunk = client.recv();
      outfile.write_all(chunk);
    }
  } catch (socket_closed_exception& e) {
    return;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <PORT> <FILE-DIR>" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    Server s{argv[1], argv[2]};
    s.start();
  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
