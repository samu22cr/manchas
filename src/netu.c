
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * @brief Gets port from socket address regardless of sa's
 * address family in Host Byte Order (ntopp means "network to presentation
 * port")
 * @param sa to subctract from
 * @return port address of socket
 */
uint16_t netu_ntopp(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in *)sa)->sin_port);
    }
    return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

/**
 * @brief Writes printable address to 's' despite address family of
 * specified socket address
 * @param sa address to read from
 * @param s buffer to write to
 * @param len writes up to this number of chars
 * @return pointer to s or NULL if unkown address family
 */
char *netu_ntop(const struct sockaddr *sa, char *s, size_t len) {
    switch (sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, len);
            break;
        case AF_INET6:
            inet_ntop(
                AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, len
            );
            break;
        default:
            strncpy(s, "Unknown AF", len);
            return NULL;
    }
    return s;
}

char *netu_ntops(struct sockaddr *sa, char *buf, int len) {

    uint16_t serv_port = netu_ntopp(sa);
    char serv_addr_buf[INET6_ADDRSTRLEN]; 
    snprintf(buf, len, "%s:%d\n", serv_addr_buf, serv_port);

    return buf;
}

