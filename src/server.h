#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10

typedef enum { GET, POST, OPTIONS } RequestType;

typedef struct {
  char *key;
  char *value;
} Header;

typedef struct {
  RequestType type;

  char *path;

  Header *headers;
  int header_count;

  char *body;
  int body_length;

} Request;

typedef struct {
  RequestType type;

  Header *headers;
  int header_count;

  char *body;
  int body_length;

} Response;

int run_server(char *port);

#endif
