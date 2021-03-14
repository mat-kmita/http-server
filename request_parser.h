#ifndef request_parser_h
#define request_parser_h

#include <stdint.h>
#include <linux/limits.h>

#define MAX_HOST_STR_LEN 256
#define MAX_RESOURCE_STR_LEN PATH_MAX

int parse_request(char *input, uint8_t *keep_alive,char * vhost, char *host, char *resource, int first);

#endif