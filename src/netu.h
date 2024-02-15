/**
 * @file
 * @brief netu - network utils
 */

#include <netinet/in.h>
#include <stdint.h>

#ifndef NETU_H
#define NETU_H

// max str len of a printable socket
#define NETU_PRINTSOCK_STRLEN                                                  \
    (INET6_ADDRSTRLEN + sizeof(":") + sizeof(uint16_t))
// Could have different macros for ipv6 and ipv4 in order to
// save a couple bytes (by not creating buffers bigger than
// necessary) but that would requiere callers of funcs like
// netu_ntops to have more overhead just to get desired values

/*
    Returns port nuber from socket address in Host Byte Order
*/
uint16_t netu_ntopp(struct sockaddr *sa);

/*
    Writes printable address to given buffer despite address family of
    specified socket address
*/
char *netu_ntop(const struct sockaddr *sa, char *s, size_t len);

/*
 * Converts a socket to an internet socket, either an ipv4 socket
 * or an ipv6 one
 */
void *netu_stoin(struct sockaddr *sa);

/*
 * Writes a printable version of specfified socket to buffer in the
 * format of "ip_address:port"
 */
char *netu_ntops(struct sockaddr *sa, char *buf, int len);

#endif // !NETU_H
