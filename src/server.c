#include "server.h"

int run_server(char *port);

int setup_socket(const char *port);

char *parse_response(Response res);
void parse_request(char buf[512], int len, Request req);
void handle_requests(int sockfd);
void add_header(void *p, char *key, char *value);
void free_header(void *p);
void create_response(Response res, Request req);
void create_options_response(Response res);

void sigchld_handler();
void setup_sigaction();
void *get_in_addr(struct sockaddr *sa);

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

char *parse_response(Response res) {
  char *final_message = malloc(6);
  strcpy(final_message, "hello\0");
  // char h_p1[32];
  // snprintf(h_p1, sizeof(h_p1), "HTTP/1.1 %d", status);
  //
  // const char h_p2[] = "\r\nContent-Type: application/json";
  // const char h_p3[] = "\r\nContent-Length: ";
  // const char h_p4[] = "\r\n\r\n{\"message\":\"";
  // const char h_p5[] = "\"}\0";
  //
  // char content_length[8];
  // snprintf(content_length, sizeof(content_length), "%d",
  //          (int){strlen(payload) + 14});
  //
  // // Calculate total message size and allocate buffer
  // size_t total_size = strlen(h_p1) + strlen(h_p2) + strlen(h_p3) +
  //                     strlen(h_p4) + strlen(h_p5) + strlen(payload) +
  //                     strlen(content_length) + 1;
  //
  // char *final_message = malloc(total_size);
  // if (final_message == NULL) {
  //   perror("malloc failed");
  //   exit(1);
  // }
  //
  // // Build the complete message
  // strcpy(final_message, h_p1);
  // strcat(final_message, h_p2);
  // strcat(final_message, h_p3);
  // strcat(final_message, content_length);
  // strcat(final_message, h_p4);
  // strcat(final_message, payload);
  // strcat(final_message, h_p5);

  // "HTTP/1.1 200\r\nContent-Type: application/json\r\nContent-Length:
  // xyz\r\n\r\n{\"message\":\"message\"}"
  // printf("%s", final_message);

  return final_message;
}

void parse_request(char buf[512], int len, Request req) {
  printf("\nreq:\n%s\n\n", buf);

  int loop_count = 0, char_index = 0;

  char method[16];
  char path[1024];
  char *key, *value, *line_split;

  char *request_line = strtok(buf, "\r\n");
  char *line = strtok(NULL, "\r\n");

  while (line) {
    key = strtok_r(line, ":", &line_split);
    value = strtok_r(NULL, "", &line_split);

    if (key && value) {
      while (*value == ' ')
        value++; // Trim leading spaces
      add_header(&req, key, value);
    }

    line = strtok(NULL, "\r\n");
  }

  // get method
  char_index = 0;
  for (; request_line[char_index] != ' ' && request_line[char_index] != '\0';
       char_index++) {
    method[char_index] = request_line[char_index];
  }
  method[char_index] = '\0';

  // get path(starting after method)
  char_index++;
  loop_count = 0;
  for (; request_line[char_index] != ' ' && request_line[char_index] != '\0';
  char_index++) {
    path[loop_count] = request_line[char_index];
    loop_count++;
  }

  path[loop_count] = '\0';
  req.path = malloc(loop_count);
  strncpy(req.path, path, loop_count);

  // set request method
  if (strcmp(method, "GET") == 0) {
    req.type = GET;
  } else if (strcmp(method, "POST") == 0) {
    req.type = POST;
  } else if (strcmp(method, "OPTIONS") == 0) {
    req.type = OPTIONS;
  }


  if (req.header_count > 0 && req.type == POST ) {
    req.body = malloc(strlen(req.headers[req.header_count - 1].value) + 1);
    strcpy(req.body, req.headers[req.header_count - 1].value);
  }
  printf("\n\n\treq:\npath: %s\ntype: %u\n body: %s\n "
         "header_count: %i\n headers\n",
         req.path, req.type, req.body, req.header_count);
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

    if (!fork()) { // create fork and check if I'm the child process, if yes
                   // then execute
      close(sockfd); // close listener, fork doesn't need it
      Request request = {.type = UNKNOWN,
                         .path = NULL,
                         .body = NULL,
                         .headers = NULL,
                         .header_count = 0};

      Response response = {.type = NULL,
                         .body = NULL,
                         .headers = NULL,
                         .header_count = 0};

      char buf[512];
      int byte_count;

      byte_count = recv(new_fd, buf, sizeof(buf), 0);

      parse_request(buf, byte_count, request);

      create_response(response, request);

      char *final_message = parse_response(response);

      if (send(new_fd, final_message, strlen(final_message), 0) == -1)
        perror("send");
      close(new_fd);

      // Clean up allocated memory
      free(final_message);
      if (request.path)
        free(request.path);
      if (request.body)
        free(request.body);
      free_header(&request);

      if (response.type)
        free(response.type);
      if (response.body)
        free(response.body);
      free_header(&response);

      exit(0);
    }
    close(new_fd);
  }
}

void create_response(Response res, Request req) {
  switch (req.type) {
    case GET:
      res.type = "POST";
      break;
    case POST:
      res.type = "POST";
    case OPTIONS:
      create_options_response(res);
      return;
      break;
    default:
     perror("Unsupported request type");
     exit(1);
    break;
  }
}

void create_options_response(Response res) {
  char type[] = "HTTP/1.1 204 No Content\t\n";
  res.type = malloc(sizeof type);
  strcpy(res.type, type);

  add_header(&res, "Allow", "OPTIONS, GET, HEAD, POST");
  add_header(&res, "Cache-Control", "max-age=604800");
  add_header(&res, "Date", get_current_time());
  add_header(&res, "Server", "EOS (lax004/2813)");

  // res.headers = "Allow: OPTIONS, GET, HEAD, POST\n\t"
  //   "Cache-Control: max-age=604800\n\t"
  //   "Date: Thu, 13 Oct 2016 11:45:00 GMT\n\t"
  //   "Server: EOS (lax004/2813)\n\t";

}

void add_header(void *p, char *key, char *value) {
  Request req = *((Request *)p);
  req.headers =
      realloc(req.headers, (req.header_count + 1) * sizeof(Header));
  req.headers[req.header_count].key = strdup(key);
  req.headers[req.header_count].value = strdup(value);
  req.header_count++;
}

void free_header(void *p) {
  Request req = *((Request *)p);
  if (req.header_count > 0) {
    for (int i = 0; i < req.header_count; i++) {
      free(req.headers[i].key);
      free(req.headers[i].value);
    }
    free(req.headers);
    req.header_count = 0;
  }
}

void sigchld_handler() {
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

char* get_current_time() {
  char *buffer = malloc(sizeof(char) * 40);
  time_t rawtime;
  struct tm *gmt;

  time(&rawtime);
  gmt = gmtime(&rawtime);

  strftime(buffer, sizeof buffer, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  return buffer;
}
