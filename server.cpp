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
#include <iostream>
#include <thread>
#include <string>

#define EXIT_FAILURE 1

int main() {
  try {
    // TODO
  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
