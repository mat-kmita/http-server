#ifndef tcp_utils_h
#define tcp__utils_h

int close_connection(int sockfd);
int send_response(int sockfd, char *header, int resourcefd, char *buffer, int buffer_size);

#endif