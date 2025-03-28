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
#include <time.h>
#include <unistd.h>

#define BACKLOG 10

typedef enum { GET, POST, OPTIONS, UNKNOWN, OPTIONS_RESPONSE } RequestType;

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
} Request;

typedef struct {
  char *type;

  Header *headers;
  int header_count;

  char *body;
} Response;

int run_server(char *port);

#endif
