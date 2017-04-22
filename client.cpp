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
#include "file.hpp"
#include "socket.hpp"

#include <iostream>
#include <string>

static std::string usage = " <HOSTNAME-OR-IP> <PORT> <FILENAME>";

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << usage << std::endl;
    return EXIT_FAILURE;
  }

  try {
    ConnectedSocket sock{argv[1], argv[2]};
    File::open_r(argv[3]).sendfile(sock);
  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
