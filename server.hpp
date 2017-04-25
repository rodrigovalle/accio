#ifndef SERVER_HPP
#define SERVER_HPP

#include "socket.hpp"
#include "file.hpp"

#include <string>
#include <thread>
#include <vector>

#define BACKLOG 10
#define THREADS 10
#define RECVSIZ BUFSIZ

class Server {
 public:
  Server(const std::string& port, const std::string& file_directory);
  Server(const Server& that) = delete; /* server's threads cannot be copied! */
  /* TODO: perhaps declare a move constructor & move assignment */
  ~Server();

  void start();
  void recv_file(ConnectedSocket client, int client_id);

 private:
  FileDescriptor dir;
  ListeningSocket sock;
  int n_conn;

  // TODO: poor man's threadpool :(
  std::vector<std::thread> threads;
};

#endif // SERVER_HPP
