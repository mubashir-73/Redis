#include <asm-generic/socket.h>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#define MYPORT "3420"

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d]%s\n", err, msg);
  abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = recv(fd, buf, n, 0);
    if (rv <= 0) {
      return -1; // error, or unexpected EOF
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = send(fd, buf, n, 0);
    if (rv <= 0) {
      return -1; // error
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

const size_t k_max_msg = 4096;

static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    return -1;
  }

  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4); // assume little endian
  memcpy(&wbuf[4], text, len);
  if (int32_t err = write_all(fd, wbuf, 4 + len)) {
    return err;
  }

  // 4 bytes header
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  // do something
  rbuf[4 + len] = '\0';
  printf("server says: %s\n", &rbuf[4]);
  return 0;
}

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
  int rv = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (rv) {
    die("connect");
  }
  int32_t err = query(sockfd, "hello1");
  if (err) {
    goto L_DONE;
  }
  err = query(sockfd, "hello2");
  if (err) {
    goto L_DONE;
  }
  err = query(sockfd, "hello3");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  int close(sockfd);
  return 0;
}
