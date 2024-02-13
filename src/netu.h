/**
 * @file
 * @brief netu - network utils
 */

#include <netinet/in.h>
#include <stdint.h>

#ifndef NETU_H
#define NETU_H

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

#endif // !NETU_H
