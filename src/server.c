#include "server.h"
#include <stdio.h>

int run_server(char *port);

int setup_socket(const char *port);

char *parse_response(Response *res);
void parse_request(char buf[512], int len, Request *req);
void handle_requests(int sockfd);
void create_response(Response *res, Request *req);

void create_get_response(Response *res, Request *req);
void create_post_response(Response *res, Request *req);
void create_options_response(Response *res);

void add_header(Header **headers, int *header_count, char *key, char *value);
void free_header(Header *headers, int *header_count);

void read_file_to(char **body, char **body_len, char *file_name);
void sigchld_handler();
void setup_sigaction();
void *get_in_addr(struct sockaddr *sa);
char *get_current_time(int offset);

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

char *parse_response(Response *res) {
  size_t message_size =
      strlen(res->type) + ((res->body) ? strlen(res->body) + 3 : 2);

  for (int i = 0; i < res->header_count; i++) {
    message_size += strlen(res->headers[i].key) +
                    strlen(res->headers[i].value) + 4; //+4 for the ": " "\n\t"
  }

  char *final_message = malloc(message_size);
  final_message[0] = '\0';

  strcpy(final_message, res->type);

  for (int i = 0; i < res->header_count; i++) {
    strcat(final_message, res->headers[i].key);
    strcat(final_message, ": ");
    strcat(final_message, res->headers[i].value);
    strcat(final_message, "\r\n");
  }

  if (res->body) {
    strcat(final_message, "\r\n");
    strcat(final_message, res->body);
  } else {
    strcat(final_message, "\0");
  }

  printf("final_message:\n\n%s\n\n", final_message);

  return final_message;
}

void parse_request(char buf[512], int len, Request *req) {
  // printf("\nreq:\n%s\n\n", buf);

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
      add_header(&req->headers, &req->header_count, key, value);
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
  req->path = malloc(loop_count);
  strncpy(req->path, path, loop_count);

  // set request method
  if (strcmp(method, "GET") == 0) {
    req->type = GET;
  } else if (strcmp(method, "POST") == 0) {
    req->type = POST;
  } else if (strcmp(method, "OPTIONS") == 0) {
    req->type = OPTIONS;
  }

  if (req->header_count > 0 && req->type == POST) {
    req->body = malloc(strlen(req->headers[req->header_count - 1].value) + 1);
    strcpy(req->body, req->headers[req->header_count - 1].value);
  }
  // printf("\n\n\treq->\npath: %s\ntype: %u\n body: %s\n "
  //        "header_count: %i\n headers\n",
  //        req->path, req->type, req->body, req->header_count);
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

    if (!fork()) {   // create fork and check if I'm the child process, if yes
                     // then execute
      close(sockfd); // close listener, fork doesn't need it
      Request request = {.type = UNKNOWN,
                         .path = NULL,
                         .body = NULL,
                         .headers = NULL,
                         .header_count = 0};

      Response response = {
          .type = NULL, .body = NULL, .headers = NULL, .header_count = 0};

      char buf[512];
      int byte_count;

      byte_count = recv(new_fd, buf, sizeof(buf), 0);

      parse_request(buf, byte_count, &request);

      // printf("\n\n\treq:\npath: %s\ntype: %u\n body: %s\n "
      //        "header_count: %i\n",
      //        request.path, request.type, request.body, request.header_count);

      create_response(&response, &request);

      char *final_message = parse_response(&response);

      if (send(new_fd, final_message, strlen(final_message), 0) == -1)
        perror("send");
      close(new_fd);

      // Clean up allocated memory
      free(final_message);
      if (request.path)
        free(request.path);
      if (request.body)
        free(request.body);
      free_header(request.headers, &request.header_count);

      if (response.type)
        free(response.type);
      if (response.body)
        free(response.body);
      free_header(response.headers, &response.header_count);

      exit(0);
    }
    close(new_fd);
  }
}

void create_response(Response *res, Request *req) {
  switch (req->type) {
  case GET:
    create_get_response(res, req);
    break;
  case POST:
    create_post_response(res, req);
  case OPTIONS:
    create_options_response(res);
    return;
    break;
  default:
    perror("Unsupported request type\n");
    exit(1);
    break;
  }
}

void create_get_response(Response *res, Request *req) {
  if (strcmp(req->path, "/data") == 0) {
    char type[] = "HTTP/1.1 200 OK\r\n";
    char *slen = malloc(64);

    res->type = malloc(sizeof type);
    if (res->type == NULL) {
        perror("Error opening file");
        return;
    }

    strcpy(res->type, type);

    read_file_to(&res->body, &slen, (char*){"./frontend/data.json"});


    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Origin", "*");
    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Headers", "Content-Type");
    add_header(&res->headers, &res->header_count, "Age", "249970");
    add_header(&res->headers, &res->header_count, "Cache-Control", "max-age=604800");
    add_header(&res->headers, &res->header_count, "Content-Type", "text/html; charset=UTF-8");
    add_header(&res->headers, &res->header_count, "Date", get_current_time(0));
    add_header(&res->headers, &res->header_count, "Expires", get_current_time(10000));
    add_header(&res->headers, &res->header_count, "Last-Modified", get_current_time(-10000));
    add_header(&res->headers, &res->header_count, "Server", "ECAcc (nyd/D13E)");
    add_header(&res->headers, &res->header_count, "Vary", "Accept-Encoding");
    add_header(&res->headers, &res->header_count, "X-Cache", "404-HIT");
    add_header(&res->headers, &res->header_count, "Content-Length", slen);


  } else {
    char type[] = "HTTP/1.1 404 Not Found\r\n";
    res->type = malloc(sizeof type);
    strcpy(res->type, type);

    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Origin", "*");
    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    add_header(&res->headers, &res->header_count, "Access-Control-Allow-Headers", "Content-Type");
    add_header(&res->headers, &res->header_count, "Age", "249970");
    add_header(&res->headers, &res->header_count, "Cache-Control", "max-age=604800");
    add_header(&res->headers, &res->header_count, "Content-Type", "text/html; charset=UTF-8");
    add_header(&res->headers, &res->header_count, "Date", get_current_time(0));
    add_header(&res->headers, &res->header_count, "Expires", get_current_time(10000));
    add_header(&res->headers, &res->header_count, "Last-Modified", get_current_time(-10000));
    add_header(&res->headers, &res->header_count, "Server", "ECAcc (nyd/D13E)");
    add_header(&res->headers, &res->header_count, "Vary", "Accept-Encoding");
    add_header(&res->headers, &res->header_count, "X-Cache", "404-HIT");
    add_header(&res->headers, &res->header_count, "Content-Length", "0");
  }
}

void create_post_response(Response *res, Request *req) {}

void create_options_response(Response *res) {
  res->type = malloc(26);
  strcpy(res->type, "HTTP/1.1 204 No Content\r\n");

  add_header(&res->headers, &res->header_count, "Allow", "GET, POST, OPTIONS");
  add_header(&res->headers, &res->header_count, "Access-Control-Allow-Origin", "*");
  add_header(&res->headers, &res->header_count, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  add_header(&res->headers, &res->header_count, "Access-Control-Allow-Headers", "Content-Type");
  add_header(&res->headers, &res->header_count, "Cache-Control", "max-age=604800");
  add_header(&res->headers, &res->header_count, "Date", get_current_time(0));
  add_header(&res->headers, &res->header_count, "Server", "EOS (lax004/2813)");

  // res.headers = "Allow: OPTIONS, GET, HEAD, POST\n\t"
  //   "Cache-Control: max-age=604800\n\t"
  //   "Date: Thu, 13 Oct 2016 11:45:00 GMT\n\t"
  //   "Server: EOS (lax004/2813)\n\t";
}

void add_header(Header **headers, int *header_count, char *key, char *value) {
  Header *temp = realloc(*headers, (*header_count + 1) * sizeof(Header));
  if (temp == NULL) {
    perror("Failed to reallocate headers");
    return;
  }
  *headers = temp;
  (*headers)[*header_count].key = strdup(key);
  (*headers)[*header_count].value = strdup(value);
  *header_count += 1;
}

void free_header(Header *headers, int *header_count) {
  if (*header_count > 0) {
    for (int i = 0; i < *header_count; i++) {
      free(headers[i].key);
      free(headers[i].value);
    }
    free(headers);
    headers = NULL;
    *header_count = 0;
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

char *get_current_time(int offset) {
  static char buffer[50];
  time_t rawtime;
  struct tm *gmt;

  time(&rawtime);
  rawtime += offset;
  gmt = gmtime(&rawtime);

  strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  return buffer;
}

void read_file_to(char **body, char **body_len, char *file_name) {
    FILE *file;
    char slen[64];

    file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer for entire file
    char *buf = malloc(file_size + 1);
    if (buf == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // Read entire file
    size_t bytes_read = fread(buf, 1, file_size, file);
    if (bytes_read < file_size) {
        perror("Error reading file");
        free(buf);
        fclose(file);
        return;
    }
    buf[file_size] = '\0';

    fclose(file);
    
    *body = malloc(file_size + 1);
    if (*body == NULL) {
        perror("Memory allocation failed");
        free(buf);
        return;
    }

    snprintf(slen, sizeof(slen), "%li", file_size);

    strcpy(*body, buf);
    strcpy(*body_len, slen);
    free(buf);
}
