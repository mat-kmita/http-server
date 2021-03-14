#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

int find_extension(char *filepath) {
    if(filepath == NULL) return -1;
    
    size_t filepath_len = strlen(filepath);
    if(filepath_len < 5) return -1;

    char *extension_start = filepath + filepath_len - 4;
    int extenstion_len = 5;

    if(*extension_start == '.') {
        extension_start += 1;
        extenstion_len = 4;
    }

    if(strncmp_lower("txt", extension_start, extenstion_len)) {
        return TXT;
    } else if(strncmp_lower("html", extension_start, extenstion_len)) {
        return HTML;
    } else if(strncmp_lower("css", extension_start, extenstion_len)) {
        return CSS;
    } else if(strncmp_lower("jpg", extension_start, extenstion_len)) {
        return JPG;
    } else if(strncmp_lower("jpeg", extension_start, extenstion_len)) {
        return JPEG;
    } else if(strncmp_lower("png", extension_start, extenstion_len)) {
        return PNG;
    } else if(strncmp_lower("pdf", extension_start, extenstion_len)) {
        return PDF;
    } else {
        return OCTET;
    }
}

char *strmime(int extension_code) {
    char *result;

    switch(extension_code) {
        case TXT:
            result = "text/plain;charset=utf-8\n";
            break;
        case HTML:
            result = "text/html;charset=utf-8\n";
            break;
        case CSS:
            result = "text/css;charset=utf-8\n";
            break;
        case JPG:
            result = "image/jpg\n";
            break;
        case JPEG:
            result = "image/jpeg\n";
            break;
        case PNG:
            result = "image/png\n";
            break;
        case PDF:
            result = "application/pdf\n";
            break;
        default:
            result = "application/octet-stream\n";
            break;
    }

    char *p;
    size_t result_len = strlen(result);
    if((p = (char*)malloc(result_len+1)) == NULL) return NULL;

    sprintf(p, "%s", result);

    return p;
}

int strncmp_lower(char *str1, char *str2, int n) {
    if(str1 == NULL || str2 == NULL)
        return 0;

    char str1_c, str2_c;
    for(int i=0; n>0; i++, n--) {
        if(str1[i] == '\0' || str2[i] == '\0')
            return str1[i] == '\0' && str2[i] == '\0';

        str1_c = str1[i];
        str2_c = str2[i];

        if(str1_c > 64 && str1_c < 91) str1_c += 32;
        if(str1_c > 64 && str2_c < 91) str2_c += 32;

        if(str1_c != str2_c)
            return 0;
    }
    return 1;
}

char *ltrim(char *str, const char *seps)
{
    size_t totrim;
    if (seps == NULL) {
        seps = "\t\n\v\f\r ";
    }
    totrim = strspn(str, seps);
    if (totrim > 0) {
        size_t len = strlen(str);
        if (totrim == len) {
            str[0] = '\0';
        }
        else {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

char *rtrim(char *str, const char *seps)
{
    int i;
    if (seps == NULL) {
        seps = "\t\n\v\f\r ";
    }
    i = strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL) {
        str[i] = '\0';
        i--;
    }
    return str;
}

char *trim(char *str, const char *seps)
{
    return ltrim(rtrim(str, seps), seps);
}