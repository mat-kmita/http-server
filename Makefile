CC = gcc -g -fno-sanitize=all
CFLAGS = -Og -Wall -Werror -std=gnu99

OBJS = server.o request_parser.o response_builder.o utils.o tcp_utils.o

FILES = server.c request_parser.h request_parser.c response_builder.h response_builder.c utils.h utils.c tcp_utils.h tcp_utils.c

all: $(OBJS) 
	$(CC) $(CFLAGS) -o server $(OBJS)

server: server.c
	$(CC) $(CFLAGS) -c server.c

utils: utils.h utils.c
	$(CC) $(CFLAGS) -c utils.h utils.c

request_parser: request_parser.h request_parser.c	
	$(CC) $(CFLAGS) -c request_parser.h request_parser.c

response_builder: response_builder.h response_builder.c
	$(CC) $(CFLAGS) -c response_builder.h response_builder.c

tcp_utils: tcp_utils.h tcp_utils.c
	$(CC) $(CFLAGS) -c tcp_utils.h tcp_utils.c

clean: 
	rm -f *.o *.gch

distclean: 
	rm -f *.o *.gch server