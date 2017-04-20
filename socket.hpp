#include <string>

#define BACKLOG 10
#define RECVBUF BUFSIZ

class ConnectedSocket;
class ListeningSocket {
 public:
	ListeningSocket(const std::string& port);
	~ListeningSocket();
	ConnectedSocket accept();

 private:
	int sockfd;
};

class ConnectedSocket {
 public:
	ConnectedSocket(const std::string& hostname, const std::string& port);
	~ConnectedSocket();
	std::string& recv();

 private:
	ConnectedSocket(int fd);
	int sockfd;
	char buf[RECVBUF];
};
