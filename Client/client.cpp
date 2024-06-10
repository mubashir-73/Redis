#include <asm-generic/socket.h>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#define MYPORT "3420"
int main() {

  struct addrinfo hints, *res;
  int sockfd;
  char buffer[256];
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, MYPORT, &hints, &res);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cout << "Error opening the socket!\n";
  }
  int val = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
    std::cout << "Error connecting socket!\n";
    std::cout << errno;
  }
  std::cout << "Enter the message:\n";
  bzero(buffer, 256);
  fgets(buffer, 255, stdin);
  send(sockfd, buffer, 255, 0);
  int close(sockfd);
  return 0;
}
