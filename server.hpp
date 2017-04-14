#include <string>
#include <thread>
#include <vector>

#define BACKLOG 10
#define THREADS 10
#define RECV_BUF 1048576 /* 1 MiB */

class Server {
 public:
  Server(const std::string& port, const std::string& file_directory);
  Server(const Server& that) = delete; /* server's threads cannot be copied! */
  /* TODO: perhaps declare a move constructor */
  ~Server();

  void start_server();
  void recv_file(int client_fd, int client_id);

 private:
  int dir_fd;
  int sock_fd;
  int client_count;
  char buf[RECV_BUF];

  std::vector<std::thread> threads;
};
