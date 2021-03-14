#include "request_parser.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

int save_host_line(char*, char*, char*);
int save_connection_line(char *, uint8_t *);

/*
    parse_request parsuje żądanie input i wpisuje:
    - w vhost wpisuje hosta po usunięciu ewentualnego portu
    - w host wpisywany jest taki host jaki był w żadaniu
    - w resource wpisywany jest zasób z pierwszej linjki
    - w keep_alive wpisywane jest 1 dla keep-alive i 0 dla close
    Argument first powinien informować, czy ma się input zaczynać od pierwszej linijki żądania.

    Zwraca -1 w przypadku błędu, 0 jeśli odczytano poprawnie dane, ale nie znaleziono
    wszystkich wymaganych i 1 jeśli znaleziono wszystkie wymagane dane.
*/
int parse_request(char *input, uint8_t *keep_alive, char *vhost, char *host, char *resource, int first) {
    if(input == NULL || keep_alive == NULL || vhost == NULL || host == NULL || resource == NULL) return -1;

    char method_str[] = "get ";
    int method_str_len = strlen(method_str);

    char *p = NULL;
    // Skanujemy z linijką z metodą HTTP na początku
    if(first) {
        // jeśli nie znajdziemy metody get to błąd
        if(!strncmp_lower(method_str, input, method_str_len)) return -1; 
        // jeśli znajdziemy get to pobieramy zasób i przeskakujemy do końca linii
        p = strtok(input + 4, " "); // input+4 ponieważ pomijamy GET
        if(p == NULL) return -1; 
        if(strncpy(resource, p, MAX_RESOURCE_STR_LEN) == NULL) return -1;
        p = strtok(NULL, "\n");
    }
    
    // tutaj skanujemy linijki postaci:
    // field-name ":" OWS field-value OWS
    // interesują nas field-name connection, host
    // usuwamy też białe znaki z wartości
    if(first && p == NULL) return 0;
    else if(!first) p = strtok(input, ":");
    else {
        p = strtok(NULL, ":");
    }

    // int found_host = 0;

    while(p != NULL) {
        if(strncmp("\r\n", p, 2) == 0) return 1;

        if(strncmp_lower("host", p, strlen("host"))) {
            if((p = strtok(NULL, "\n")) == NULL) return -1;
            if((p = trim(p, NULL)) == NULL) return -1;
            if(save_host_line(p, vhost, host) == -1) return -1;
        } else if(strncmp_lower("connection", p, strlen("connection"))) {
            if((p = strtok(NULL, "\n")) == NULL) return -1;
            if(save_connection_line(p, keep_alive) == -1) return -1;
        } else {
            // znalezlismy coś czego nie obsługujemy
            if((p = strtok(NULL, "\n")) == NULL) return 0;
        }

        p = strtok(NULL, ":");
    }

    return 0;
}

int save_host_line(char *start, char *vhost, char *host) {
    if(start == NULL || host == NULL) return -1;

    start = trim(start, NULL);
    strncpy(host, start, MAX_HOST_STR_LEN);

    char *p = strchr(start, ':');
    if(p != NULL) *p = '\0';

    strncpy(vhost, start, MAX_HOST_STR_LEN);

    return 0;
}

int save_connection_line(char *start, uint8_t *keep_alive) {
    if(start == NULL || keep_alive == NULL) return -1;

    start = trim(start, NULL);

    *keep_alive = strncmp_lower("keep-alive", start, strlen("keep-alive"));
    return 0;
}