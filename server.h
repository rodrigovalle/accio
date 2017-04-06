#include <string>

class Server {
public:
  Server(const std::string& port, const std::string& file_directory);
  ~Server();
  void start_server();

private:
  int dir_fd;
  int sock_fd;
};
