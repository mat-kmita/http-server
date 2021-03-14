#include "tcp_utils.h"

#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int close_connection(int sockfd) {
    if(sockfd < 0) return -1;
    if(shutdown(sockfd, SHUT_RDWR) == -1) return -1;
    if(close(sockfd) == -1) return -1;

    return 1;
}

int send_header(int sockfd, char *header) {
    int header_len = strlen(header);
    int left_to_send = header_len;
    char *p = header;

    while(left_to_send > 0) {
        int sent = send(sockfd, p, left_to_send, 0);
        if(sent < 0) return -1;

        left_to_send -= sent;
        p += sent;
    }

    return 0;
}

int send_body(int sockfd, int resourcefd, char *buffer, int buffer_size) {
    if(resourcefd == -1) return -1;

    int read_size;
    while((read_size = read(resourcefd, buffer, buffer_size)) > 0) {
        int sum_sent = 0;
        int left_to_send = read_size;
        char *send_pointer = buffer;

        while(sum_sent != read_size) {
            int sent_bytes = send(sockfd, send_pointer, left_to_send, 0);
            if(sent_bytes < 0) return -1;

            sum_sent += sent_bytes;
            left_to_send -= sent_bytes;
            send_pointer += sent_bytes;
        }
    }

    return 0;
}

int send_response(int sockfd, char *header, int resourcefd, char *buffer, int buffer_size) {
    if( send_header(sockfd, header) == -1)
        return -1;
    
    if(resourcefd == -1) return 1;
    return send_body(sockfd, resourcefd, buffer, buffer_size);
}
