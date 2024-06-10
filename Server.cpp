#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#define MYPORT "3420"
#define Backlog 10

int main() {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd, new_fd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, MYPORT, &hints, &res);

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  std::cout << "Socket created...\n";
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  std::cout << "Socket bound..\n";
  listen(sockfd, Backlog);
  std::cout << "Listening...\n";
  addr_size = sizeof their_addr;
  while (true) {
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd < 0) {
      std::cout << "error accepting connection restarting loop...\n";
      continue;
    }
    std::cout << "connection accepted...\n";
    std::string buffer;
    char temp_buffer[256];
    int len = 255;
    std::cout << "Recieving message....\n";
    recv(new_fd, temp_buffer, len, 0);
    std::cout << "Message Recieved!\n";
    buffer = std::string(temp_buffer);
    std::cout << "Message: " << buffer << std::endl;
    int close(new_fd);
  }
  return 0;
}
