#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

const char message_part_1[] =
    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ";
const char message_part_2[] = "\r\n\r\n{\"message\":\"";

void sigchld_handler(int s) {
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int setup_socket(const char *port) {
  int sockfd; // listen on sock_fd, new connection on new_fd
  int rv;

  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // use local address
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  return sockfd;
}

char *build_response() {
  const char *payload = "something";
  char content_length[8];
  int payload_len = strlen(payload) + 14; // 14 is the full message
  snprintf(content_length, sizeof(content_length), "%d", payload_len);

  // Calculate total message size and allocate buffer
  size_t total_size = strlen(message_part_1) + strlen(content_length) +
                      strlen(message_part_2) + strlen(payload) +
                      3; // +2 for the closing "}" and null terminator

  char *final_message = malloc(total_size);
  if (final_message == NULL) {
    perror("malloc failed");
    exit(1);
  }

  // Build the complete message
  strcpy(final_message, message_part_1);
  strcat(final_message, content_length);
  strcat(final_message, message_part_2);
  strcat(final_message, payload);
  strcat(final_message, "\"}");
  printf("%s", final_message);

  return final_message;
}

void setup_sigaction() {
  struct sigaction sa;

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
}

// we assume get request for now
void parse_request(char buf[512], int len, char method[15], char path[4]) {
  int word_count = 0, char_index = 0;
  char words[2][15] = {{0}};

  for (size_t i = 0; i < len && word_count < 2; i++) {
    if (buf[i] == ' ') {
      words[word_count][char_index] = '\0';
      word_count++;
      char_index = 0;
    } else if (char_index < 14) {
      words[word_count][char_index] = buf[i];
      char_index++;
    }
  }

  words[word_count][char_index] = '\0';

  strcpy(method, words[0]);
  strcpy(path, words[1]);
}

void handle_requests(int sockfd) {
  int new_fd;
  struct sockaddr_storage their_addr; // connector's address information
  char s[INET6_ADDRSTRLEN];

  while (1) {
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
                    &(socklen_t){sizeof their_addr});
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (!fork()) {   // create fork and check if I'm the child process, if yes
      char buf[512]; // then execute
      int byte_count;
      char path[4];
      char method[15];

      close(sockfd); // close listener, fork doesn't need it

      byte_count = recv(new_fd, buf, sizeof(buf), 0);
      parse_request(buf, byte_count, method, path);
      printf("\n\tpath: %s\n", path);
      char *final_message = build_response();

      if (send(new_fd, final_message, strlen(final_message), 0) == -1)
        perror("send");
      close(new_fd);
      exit(0);
      free(final_message);
    }
    close(new_fd);
  }
}

int run_server(char *port) {
  int sockfd = setup_socket(port);

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  handle_requests(sockfd);

  return 0;
}
