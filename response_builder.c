#include "response_builder.h"
#include "utils.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define HEAD_200 "HTTP/1.1 200 OK\r\n"
#define HEAD_301 "HTTP/1.1 301 Moved Permanently\r\nLocation: "
#define HEAD_403 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nConent-Length: "
#define HEAD_404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nConent-Length: "
#define HEAD_501 "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\nConent-Length: "

#define HEAD_TYPE "Content-Type: "
#define HEAD_LENGTH "Content-Length: "

#define BODY_403 "<html><head><title>403 - Forbidden</title><body><h1> 403 Forbidden</h1></body></html>"
#define BODY_404 "<html><head><title>404 - Not Found</title><body><h1> 404 Not Found</h1></body></html>"
#define BODY_501 "<html><head><title>501 - Not Implemented</title><body><h1> 501 Not Implemented</h1></body></html>"

int create_location_link(char *dest, char *host, char *resource);

char *build_200_response(int fd, int ext_code) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) == -1) {
        return NULL;
    }

    int size = statbuf.st_size;
    char len[21];
    sprintf(len, "%d", size);
    size_t len_len = strlen(len);
    
    char *mime;
    int mime_len;
    if((mime = strmime(ext_code)) == NULL) return NULL;
    mime_len = strlen(mime);

    size_t head_len = strlen(HEAD_200);
    size_t type_len = strlen(HEAD_TYPE);
    size_t length_len = strlen(HEAD_LENGTH);

    size_t header_len = head_len + type_len + mime_len + length_len + len_len + 4;

    char *buffer;
    if((buffer = (char*)malloc(header_len + 1)) == NULL) {
        return NULL;
    }

    if(snprintf(buffer, header_len, "%s%s%s%s%s\r\n\n", HEAD_200, HEAD_TYPE, mime, HEAD_LENGTH, len) < 0)
        return NULL;

    return buffer;
}

char *build_301_response(char *new_location, size_t new_location_len) {
    if(new_location == NULL) {
        return NULL;
    }

    size_t head_len = strlen(HEAD_301);
    size_t header_len = head_len + new_location_len + 4;

    char *buffer;
    if((buffer = (char*)malloc(header_len + 1)) == NULL) {
        return NULL;
    }

    if(snprintf(buffer, header_len, "%s%s\r\n\n", HEAD_301, new_location) < 0) return NULL;

    return buffer;
}


char *build_simple_response(char *header, size_t header_len, int code) {
    char *buffer, *body;

    if(code == 403) body = BODY_403;
    else if(code == 404) body = BODY_404;
    else if(code == 501) body = BODY_501;
    else return NULL;

    char *separator = "\r\n\n";
    size_t separator_len = strlen(separator);

    size_t body_length = strlen(body);
    char len[21];
    sprintf(len, "%ld", body_length);

    size_t len_length = strlen(len);
    size_t length = header_len + len_length + body_length + 2 * separator_len;

    if((buffer = (char*)malloc(length + 1)) == NULL) {
        return NULL;
    }

    if(snprintf(buffer, length, "%s%s%s%s%s", header, len, separator, body, separator) < 0) return NULL;

    return buffer;
}

char *build_403_response() {
    return build_simple_response(HEAD_403, strlen(HEAD_403), 403);
}

char *build_404_response() {
    return build_simple_response(HEAD_404, strlen(HEAD_404), 404);
}

char *build_501_response() {
    return build_simple_response(HEAD_501, strlen(HEAD_501), 501);
}

char *create_response_header(char *host, char *vhost, char *resource, int *fd) {
    if(fd == NULL)
        return NULL;

    *fd = -1;

    if(host == NULL || vhost == NULL || resource == NULL) {
        return build_501_response();
    }

    int host_dir, file;
    if((host_dir = open(vhost, O_RDONLY | O_DIRECTORY | O_NOFOLLOW)) == -1) {
        return build_404_response();
    }

    if(strstr(resource, "../") != NULL) 
        return build_403_response();

    resource = trim(resource, "/");

    size_t prefix = strspn(resource, "./");
    resource += prefix;
    if(*resource == '\0') {
        file = host_dir;
    } else {
        if((file = openat(host_dir, resource, O_RDONLY | O_NOFOLLOW)) == -1)
            return build_404_response();
    }

    struct stat buf;
    if (fstat(file, &buf) != 0) {
        return NULL;
    }
    if (S_ISDIR(buf.st_mode)) {
        int dir_file = file;
        if((file = openat(dir_file, "index.html", O_RDONLY | O_NOFOLLOW)) == -1)
            return build_404_response();

        char *p;
        if((p = (char*)malloc(PATH_MAX)) == NULL) {
            return NULL;
        }

        int location_len;
        if((location_len = create_location_link(p, host, resource)) == -1) return NULL;
        
        char *res = build_301_response(p, location_len);
        free(p);
        return res;
    }

    *fd = file;
    int extension_code = find_extension(resource);

    char *retval = build_200_response(*fd, extension_code);
    return retval;
}
 
int create_location_link(char *dest, char *host, char *resource){
    if(dest == NULL || host == NULL || resource == NULL) return -1;

    char *head = "http://";
    char *tail = "index.html";
    size_t head_len = strlen(head);
    size_t tail_len = strlen(tail);
    size_t location_len;

    int result;
    if(resource[0] == '\0') {
        location_len = head_len + strlen(host) + tail_len + 2;
        if((result = snprintf(dest, location_len, "%s%s/%s", head, host, tail)) < 0) 
            return -1;
    } else {
        location_len = head_len + strlen(host) + strlen(resource) + tail_len + 3;
        if((result = snprintf(dest, location_len, "%s%s/%s/%s", head, host, resource, tail)) < 0)
            return -1;
    }
    return result;
}