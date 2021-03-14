#ifndef utils_h
#define utils_h

#define TXT 0
#define HTML 1
#define CSS 2
#define JPG 3
#define JPEG 4
#define PNG 5
#define PDF 6
#define OCTET 7

int find_extension(char *filepath);
int strncmp_lower(char *str1, char *str2, int n);
char *trim(char *str, const char *seps);
char *strmime(int extension_code);

#endif