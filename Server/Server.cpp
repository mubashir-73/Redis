#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#define MYPORT "3420"
#define Backlog 10

// Implementing parser here:

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = recv(fd, buf, n, 0);
    if (rv <= 0) {
      return -1;
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

static int32_t write_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = send(fd, buf, n, 0);
    if (rv <= 0) {
      return -1;
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

const size_t k_max_msg = 4096;
static int32_t one_request(int new_fd) {
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(new_fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      std::cout << "EOF reached~";
    } else {
      std::cout << "read() Error";
    }

    return err;
  }
  u_int32_t len = 0;
  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    std::cout << "Message too long";
    return -1;
  }
  err = read_full(new_fd, &rbuf[4], len);
  if (err) {
    std::cout << "read() error\n";
    return err;
  }
  rbuf[4 + len] = '\0';
  std::cout << "Client says: " << &rbuf[4] << "\n";
  const char Reply[] = "world";
  char wbuf[4 + sizeof(Reply)];
  len = (uint32_t)strlen(Reply);
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], Reply, len);
  return write_full(new_fd, wbuf, 4 + len);
}

// Response here:

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
    while (true) {
      int32_t err = one_request(new_fd);
      if (err) {
        break;
      }
    }
    int close(new_fd);
  }
  return 0;
}
