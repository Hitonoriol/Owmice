extern char nullterm;
extern char linebreak;

extern char* strchr(const char *s, int c);
extern int atoi(char* str);
extern void swap(char *x, char *y);
extern char* itoa(int value, char* buffer, int base);
extern size_t strlen(const char* str);
extern char *strcat(char *dest, const char *src);
extern char *clearchar(char *str);
extern int cmp(const char* s1, const char* s2);
extern bool streq(const char* s1, const char* s2);
extern char *strcpy(char* destination, char* source);
extern char *substr(char *dest, char *src, int position, int length);
extern char *strshl (char *str, size_t amt);
extern char *strtok(char *dest, char *src, char *delim);
