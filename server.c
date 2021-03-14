#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <arpa/inet.h>

#include "utils.h"
#include "tcp_utils.h"
#include "request_parser.h"
#include "response_builder.h"

#define MAX_RESOURCE_STR_LEN PATH_MAX

int is_request_not_full(char *a, char *b, char *c) {
    return (a[0] == '\0' || b[0] == '\0' || c[0] == '\0');
}

int main(int argc, char **args) {
    if(argc != 3) {
        fprintf(stderr, "Invalid arguements!\nPlease use: [port] [working directory]\n");
        exit(EXIT_FAILURE);
    }

    if(chdir(args[2]) != 0) {
        fprintf(stderr, "Couldn't start server in specifid directory!\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        fprintf(stderr, "Couldn't create network socket!\n");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed\n");

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(args[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;


    if(bind(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        fprintf(stderr, "Error while binding socket to address.\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, 0) != 0) {
        fprintf(stderr, "Couldn't listen to incoming connection!\n");
        exit(EXIT_FAILURE);
    }

    // Główny socket oczekuje nieskończenie długo na klientów
    struct pollfd poll_sockfds[1];
    poll_sockfds[0].fd = sockfd;
    poll_sockfds[0].events = POLLIN;

    while(1) {
        int retval;
        if((retval = poll(poll_sockfds, 1, -1)) == -1) {
            printf("Select on main socket error!\n");
            exit(EXIT_FAILURE);
        }
        else if(retval == 0) printf("impossible!\n");
        else if(retval) {
            if(poll_sockfds[0].revents & POLLIN) {
                int socket_fd;
                while((socket_fd = accept(sockfd, NULL, NULL)) != -1) { // nowy klient
                    printf("New connection!\n");

                    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

                    struct pollfd client_fds[1];
                    client_fds[0].fd = socket_fd;
                    client_fds[0].events = POLLIN | POLLHUP;

                    const int buffers_alloc_size = 4096;
                    char *buffer = NULL, *response_header = NULL, *resource_buffer = NULL;

                    int is_new = 1; // nowe połączenie, z którego nie odczytaliśmy nic jeszcze

                    uint8_t keep_alive = 0;
                    char host[MAX_HOST_STR_LEN];
                    char vhost[MAX_HOST_STR_LEN];
                    char resource[MAX_RESOURCE_STR_LEN];

                    host[0] = '\0';
                    vhost[0] = '\0';
                    resource[0] = '\0';
                    while(1) {
                        int retval;
                        // klient ma 500ms zanim zamkniemy połączenie
                        if((retval = poll(client_fds, 1, 1000)) == -1) {
                            fprintf(stderr, "Couldn't wait for requestes!\n");
                            exit(EXIT_FAILURE);
                        }
                        else if(retval) {
                            printf("New data received!\n");

                            int size = 0;
                            if((buffer = (char*)malloc(buffers_alloc_size * sizeof(char))) == NULL) {
                                fprintf(stderr, "Error allocating memory!\n");
                                exit(EXIT_FAILURE);
                            }

                            // zakładamy, że odczytamy całe linie w żądaniu http
                            if((size = read(socket_fd, buffer, buffers_alloc_size)) == 0) {
                                printf("Client closed connection!\n");
                                break;
                            } else if(size < 0) {
                                fprintf(stderr, "Couldn't read from socket!\n");
                                break;
                            } else {
                                printf("Read something!\n");

                                int resourcefd;

                                int parse_result = parse_request(buffer, &keep_alive, vhost, host, resource, is_new);
                                int is_not_finished = is_request_not_full(host, vhost, resource);

                                if(parse_result == -1) {
                                    // błąd czytania żądania, więc stwórzmy nagłówek z kodem 501
                                    response_header = create_response_header(NULL, NULL, NULL, &resourcefd);
                                } else if(parse_result == 0) {
                                    // odczytaliśmy coś ale potrzebujemy hosta
                                    printf("Not all request data received! Waiting for more...\n");
                                    is_new = 0;
                                    continue;
                                } else {
                                    printf("All data received!\n");

                                    if(is_not_finished) {
                                        printf("End of request reached but not all parameters found!\n");
                                        response_header = create_response_header(NULL, NULL, NULL, &resourcefd);

                                    } else {
                                        response_header = create_response_header(host, vhost, resource, &resourcefd);
                                    }
                                }

                                free(buffer);

                                if(response_header == NULL) {
                                    fprintf(stderr, "Couldn't prepare response!\n");
                                    break;
                                }

                                printf("Response header:\n%s\n", response_header);

                                if(resourcefd != -1 && 
                                    (resource_buffer = (char*)malloc(buffers_alloc_size)) == NULL) {
                                    fprintf(stderr, "Couldn't allocate memory for requested resource!\n");
                                    break;
                                }

                                if(send_response(socket_fd, response_header, resourcefd, resource_buffer, buffers_alloc_size) == -1) {
                                    fprintf(stderr, "Couldn't send response!\n");
                                    break;
                                }

                                if(!keep_alive) break;
                                is_new = 1;
                            }
                        }
                        else {
                            printf("Timeout!\n");
                            break;
                        }
                    }
                    printf("Closing connection with client!\n");

                    close(socket_fd);
                    if(response_header != NULL) {
                        free(response_header);
                    }
                    if(resource_buffer != NULL) {
                        free(resource_buffer);
                    }
                }
            }
        } 
    } 

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    printf("Bye!\n");
    exit(EXIT_SUCCESS);
}
