#ifndef HTTP_H
#define HTTP_H

enum HttpMethod {
	GET,
	POST,
	PUT,
	DELETE
};

struct HttpRequest {
    enum HttpMethod method;
};

void http_parse_req(struct HttpRequest *req, int client_sockfd);

#endif // !HTTP_H
