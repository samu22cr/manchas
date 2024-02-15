#include "http.h"
#include "log.h"
#include <sys/socket.h>

#define RECV_BUFF_SIZE 128 // tmp solution for a fixed read of bytes

void http_parse_req(struct HttpRequest *req, int client_sockfd) {

    char recv_buff[RECV_BUFF_SIZE];
    int bytes_read = recv(client_sockfd, recv_buff, sizeof(recv_buff), 0);
    log_info("received http req:\n%s", recv_buff);
}
