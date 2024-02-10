
#include "log.h"
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MY_PORT "8080"
#define TCP_BACKLOG 10 // max # of parallel tcp connections to be queued

/*
TODOs:
    - Make it so logging calls only get actually
    printed when selected log level is matches func
    - Read config (log level, port, etc..):
        - from cli params && conf file
        - merge config from all sources above

*/

void log_latest_errno_and_exit(const char *const label) {
    char *errmsg = strerror(errno);
    log_error("%s: %s", label, errmsg);
    exit(EXIT_FAILURE);
}

int main(void) {

    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status_addrinfo = getaddrinfo(NULL, MY_PORT, &hints, &servinfo);
    if (status_addrinfo != 0) {
        log_error("getaddrinfo error: %s\n", gai_strerror(status_addrinfo));
        exit(EXIT_FAILURE);
    }
    int serv_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                             servinfo->ai_protocol);
    if (serv_sockfd == -1) {
        log_latest_errno_and_exit("socket");
    }

    // so no "Address already in use" error msg...
    int yes = 1;
    int status_setsockopt =
        setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (status_setsockopt == -1) {
        log_latest_errno_and_exit("setsockopt");
    }

    int status_bind =
        bind(serv_sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status_bind == -1) {
        log_latest_errno_and_exit("bind");
    }

    freeaddrinfo(servinfo);
    int status_listen = listen(serv_sockfd, TCP_BACKLOG);
    if (status_listen == -1) {
        log_latest_errno_and_exit("listen");
    }

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    while (1) {
        int client_sockfd = accept(serv_sockfd, (struct sockaddr *)&client_addr,
                                   &client_addr_size);
        if (client_sockfd == -1) {
            // handle errno value
        }
    }
}
