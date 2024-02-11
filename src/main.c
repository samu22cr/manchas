#include "log.h"
#include "netu.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT "8080"
#define TCP_BACKLOG 10 // max # of parallel tcp connections to be queued

/*
ISSUES/BUGS:
    - Not displaying correct server socket info
    at startup
        - check endianness of printed data
CURRENT:
    - implementing basic http func
FEATURES:
    - logging
        - add formatted logging (time, pid, etc...)
        - log to spec file or stdout???
        - make it so logging calls only get actually
        printed when selected log level is matches func
    - read config
        - from cli params &&|| conf file (toml, yaml, json, other???)
        - merge config from all sources above
        - set following params
            - log level (info, debug, warning, error)
            - tcp port
            - logging params (file, level, etc...)
            - ip version
                - v4
                - v6
                - any

*/

/**
 * @brief Logs lastest errno code in human readable
 * form to stdout
 * @param label text to prepend before error msg
 */
void log_latest_errno_and_exit(const char *const label) {
    char *errmsg = strerror(errno);
    log_error("%s: %s", label, errmsg);
    exit(EXIT_FAILURE);
}

void *to_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void) {
    struct addrinfo hints, *serv_addr;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status_addrinfo = getaddrinfo(NULL, MY_PORT, &hints, &serv_addr);
    if (status_addrinfo != 0) {
        log_error("getaddrinfo error: %s\n", gai_strerror(status_addrinfo));
        exit(EXIT_FAILURE);
    }

    int serv_sockfd = socket(
        serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol
    );
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
        bind(serv_sockfd, serv_addr->ai_addr, serv_addr->ai_addrlen);
    if (status_bind == -1) {
        log_latest_errno_and_exit("bind");
    }

    int status_listen = listen(serv_sockfd, TCP_BACKLOG);
    if (status_listen == -1) {
        log_latest_errno_and_exit("listen");
    }

    char serv_addrbuff[INET6_ADDRSTRLEN];
    inet_ntop(
        serv_addr->ai_family, to_in_addr((struct sockaddr *)&serv_addr),
        serv_addrbuff, sizeof serv_addrbuff
    );
    uint16_t serv_port = netu_ntopp((struct sockaddr *)&serv_addr);
    log_info("Listening for requests at %s:%d\n", serv_addrbuff, serv_port);
    freeaddrinfo(serv_addr); // !!!

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    while (1) {
        int client_sockfd = accept(
            serv_sockfd, (struct sockaddr *)&client_addr, &client_addr_size
        );
        if (client_sockfd == -1) {
            char *errmsg = strerror(errno);
            log_warning("%s: %s", "accept", errmsg);
            continue;
        }

        char client_addrbuff[INET6_ADDRSTRLEN];
        inet_ntop(
            client_addr.ss_family, to_in_addr((struct sockaddr *)&client_addr),
            client_addrbuff, sizeof client_addrbuff
        );
        uint16_t client_port = netu_ntopp((struct sockaddr *)&client_addr);
        log_info("got connection from: %s:%d\n", client_addrbuff, client_port);

        // do whatever with connection

#define RECV_BUFF_SIZE 128
        char recv_buff[RECV_BUFF_SIZE];
        int bytes_read = recv(client_sockfd, recv_buff, sizeof(recv_buff), 0);
        log_info("received http req: %s\n", recv_buff);
        close(client_sockfd);
    }

    // freeaddrinfo(servinfo); FREE much before this cause this linked list
    //                         or simply substract dup selected node from
    //                         addresses linked list and dealloc/free
    //                         all other nodes...
    // REMEMBER: servinfo above contains lots of .next allocated references
    // freeaddrinfo(serv_addr); // here DEALLOCATING possible server addresses!!!!
    close(serv_sockfd);
}
