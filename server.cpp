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
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdexcept>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <cerrno>
#include <cstring>
#include <cstdlib>

Server::Server(const std::string& port, const std::string& file_directory)
    : sock(port), n_conn(1) {
  /* number connections starting from 1 or we'll fail a bunch of test cases.
   * isn't this a CS class though I mean let's be real here,
   * they should be zero indexed */

  dir = FileDescriptor::opendir(file_directory);

  /* check your privilege
   * NOTE: man pages warn that access() has a race condition, but we're not
   *       using it to give permissions, just perform a sanity check */
  if (access(file_directory.c_str(), W_OK) == -1) {
    throw std::runtime_error{"no write permissions in " + file_directory};
  }
}

Server::~Server() {
  /* When the Server is destroyed, we have a few options:
   *
   *  1. wait for all threads to finish with their clients
   *     (and hope this happens in a timely manner/at all)
   *  2. signal all threads with a condition variable that the
   *     server is exiting and join() each thread
   *  3. kill everything and leave a mess of unfinished files on disk
   *
   *  I chose option 3 because I want the server to go down immediately and
   *  the spec doesn't say anything about what the server's behavior should be
   *  when told to exit.
   *
   *  TODO: the safe option is number 2, so all threads would get the chance
   *        to destruct their ConnectedSocket objects */
}

void Server::start() {
  while (true) {
    ConnectedSocket conn = sock.accept();

    /* create & detach a new thread to handle the connection and continue
     * waiting for new connections.
     *
     * NOTE: the thread is detached because I don't care if it ever finishes;
     *       if the server goes down and the threads aren't done, then what
     *       happens to the client connections is undefined */
    std::thread(&Server::recv_file, this, std::move(conn), n_conn).detach();
    n_conn++;
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

  } catch (socket_timeout_error& e) {
    outfile.clear();
    outfile.write_all("ERROR: socket timed out");
    /* TODO: if these methods throw std::runtime_error the thread will call
     *       std::terminate() */
    return;

  } catch (socket_closed_exception& e) {
    return;
  }
}

/* main code block */

/* Blocks SIGQUIT and SIGTERM signals in current thread (main); any threads
 * spawned by main will inherit this signal mask.
 * Replaces block_mask with the set of signals that have been blocked */
static void block_signals(sigset_t *block_mask) {
  sigemptyset(block_mask);
  sigaddset(block_mask, SIGINT);
  sigaddset(block_mask, SIGQUIT);
  sigaddset(block_mask, SIGTERM);

  if (pthread_sigmask(SIG_BLOCK, block_mask, NULL) == -1) {
    throw std::runtime_error{"pthread_sigmask(): " +
                             std::string{strerror(errno)}};
  }
}

/* A thread routine that unblocks and handles SIGQUIT and SIGTERM signals.
 * 'arg' is a pointer to a sigset_t which specifies signals to wait for */
static void handle_signals(sigset_t* sigset) {
  int sig_caught;

  if (sigwait(sigset, &sig_caught) == -1) {
    throw std::runtime_error{"sigwait(): " + std::string{strerror(errno)}};
    // XXX: will call std::terminate()
  }

  switch (sig_caught) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
      /* _exit() will kill the process immediately and won't clean up our
       * server object appropriately :(
       * There's not much we can do about this because we can't propogate
       * exceptions out of threads without keeping the main thread waiting.
       *
       * Potential workaround:
       * Send a signal to the main thread which handles it by using setjmp? */
      _exit(EXIT_SUCCESS);
      break;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <PORT> <FILE-DIR>" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    sigset_t blocked;
    block_signals(&blocked);
    std::thread{handle_signals, &blocked}.detach();
    Server s{argv[1], argv[2]};
    s.start();

  } catch (std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
