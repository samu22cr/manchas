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
CURRENT:
    - implementing basic http func
        - parse/read incoming request
            - insert headers into a map/haspmap
            - read/parse url
            - get method
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

typedef struct {
    struct addrinfo *serv_info;
    int serv_sockfd;
} ServerContext;

void bind_server(ServerContext *ctx) {

    struct addrinfo hints, *tmp_serv_info, *serv_info;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int status_addrinfo = getaddrinfo(NULL, MY_PORT, &hints, &serv_info);

    if (status_addrinfo != 0) {
        log_error("getaddrinfo error: %s\n", gai_strerror(status_addrinfo));
        exit(EXIT_FAILURE);
    }

    for (tmp_serv_info = serv_info; tmp_serv_info != NULL;
         tmp_serv_info = tmp_serv_info->ai_next) {

        char serv_addrbuff[INET6_ADDRSTRLEN]; 
        uint16_t serv_port = netu_ntopp(tmp_serv_info->ai_addr);
        inet_ntop(
            tmp_serv_info->ai_family,
            to_in_addr(tmp_serv_info->ai_addr), serv_addrbuff,
            sizeof serv_addrbuff
        );
        log_debug("Trying to link socket [%s:%d]", serv_addrbuff, serv_port);
        ctx->serv_sockfd = socket(
            serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol
        );
        if (ctx->serv_sockfd == -1) {
            char *errmsg = strerror(errno);
            log_debug(
                "Couldn't link socket [%s], trying again...",
                "some-ip:some-port"
            );
            log_debug("%s: %s", "socket", errmsg);
            continue;
        }

        log_debug(
            "Setting SO_REUSEADDR option to socket [%s]", "some-ip:some-port"
        );
        int yes = 1;
        int status_setsockopt = setsockopt(
            ctx->serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes
        );
        if (status_setsockopt == -1) {
            char *errmsg = strerror(errno);
            log_debug("Couldn't set SO_REUSEADDR option to socket [%s], trying "
                      "again..."
                      "some-ip:some-port");
            log_debug("%s: %s", "socket", errmsg);
            close(ctx->serv_sockfd);
            continue;
        }

        log_debug("Binding socket [%s]", "some-ip:some-port");
        int status_bind =
            bind(ctx->serv_sockfd, serv_info->ai_addr, serv_info->ai_addrlen);
        if (status_bind == -1) {
            char *errmsg = strerror(errno);
            log_debug(
                "Couldn't bind socket [%s], trying again... ",
                "some-ip:some-port"
            );
            log_debug("%s: %s", "socket", errmsg);
            close(ctx->serv_sockfd);
            continue;
        }
        break;
    }
    // freeaddrinfo(serv_info); // fuck, can't dealloc WHOLE linked list yet...
    // ??? unless deep copy tmp_serv_info members except .ai_next
    // (malloc/calloc) into ctx->serv_info ????
    if (tmp_serv_info == NULL) {
        log_error("server: failed to bind socket");
        exit(EXIT_FAILURE);
    }
    ctx->serv_info = tmp_serv_info;
}

int main(void) {

    ServerContext serv_ctx = {
        .serv_info = NULL,
        .serv_sockfd = -1,
    };
    bind_server(&serv_ctx);
    int serv_sockfd = serv_ctx.serv_sockfd;
    int status_listen = listen(serv_sockfd, TCP_BACKLOG);
    if (status_listen == -1) {
        log_latest_errno_and_exit("listen");
    }
    char serv_addrbuff[INET6_ADDRSTRLEN];
    uint16_t serv_port = netu_ntopp(serv_ctx.serv_info->ai_addr);
    inet_ntop(
        serv_ctx.serv_info->ai_family, to_in_addr(serv_ctx.serv_info->ai_addr),
        serv_addrbuff, sizeof serv_addrbuff
    );
    log_info("Listening for requests at %s:%d", serv_addrbuff, serv_port);
    freeaddrinfo(serv_ctx.serv_info); // allocated in bind_server()
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
        log_info("got connection from: %s:%d", client_addrbuff, client_port);

        // do whatever with connection
#define RECV_BUFF_SIZE 128
        char recv_buff[RECV_BUFF_SIZE];
        int bytes_read = recv(client_sockfd, recv_buff, sizeof(recv_buff), 0);
        log_info("received http req:\n%s", recv_buff);

        close(client_sockfd);
    }

    close(serv_sockfd);
}
