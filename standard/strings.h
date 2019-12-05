#ifndef _STANDARD_STRINGS_H
#define _STANDARD_STRINGS_H
char nullterm = '\0';
char linebreak = '\n';

char *strchr(const char *s, int c) {
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

int abs(int num) {
	if (num < 0)
		return num * (-1);
	else
		return num;
}

int atoi(char* str) {
	int res = 0;
	int sign = 1;
	int i = 0;
	if (str[0] == '-') { 
		sign = -1; 
		i++;
	}
	for (; str[i] != '\0'; ++i)
		res = res * 10 + str[i] - '0';
	return sign * res; 
}

void swap(char *x, char *y) {
	char t = *x;
	*x = *y;
	*y = t;
}

char *reverse(char *buffer, int i, int j) {
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}

char *itoa(int value, char* buffer, int base) {
	if (base < 2 || base > 32)
		return buffer;
	int n = abs(value);
	int i = 0;
	while (n) {
		int r = n % base;
		if (r >= 10) 
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}
	if (i == 0)
		buffer[i++] = '0';
	if (value < 0 && base == 10)
		buffer[i++] = '-';
	buffer[i] = '\0';
	return reverse(buffer, 0, i - 1);
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

char *strcat(char *dest, const char *src) {
    size_t i,j;
    for (i = 0; dest[i] != nullterm; i++)
        ;
    for (j = 0; src[j] != nullterm; j++)
        dest[i+j] = src[j];
    dest[i+j] = nullterm;
    return dest;
}

extern void *memset(void* bufptr, int value, size_t size);
char *clearchar(char *str) {
	memset(str, 0, strlen(str));
	return str;
}

char *justincase (char *s) {
	strcat(s, &nullterm);
	return s;
}

int cmp(const char* s1, const char* s2) {
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

bool streq(const char* s1, const char* s2) {
	if (cmp(s1, s2) == 0)
		return true;
	else
		return false;
}

char *strcpy(char* destination, char* source) {
	if (destination == NULL)
		return NULL;
	char *ptr = destination;
	while (*source != nullterm) {
		*destination = *source;
		destination++;
		source++;
	}
	*destination = nullterm;
	return ptr;
}

char *substr(char *dest, char *src, int position, int length) {
	while(length > 0) {
		*dest = *(src+position);
		dest++;
		src++;
		length--;
	}
	*dest = nullterm;
	return dest;
} 

extern void owmice_writestring(char*);
char *strshl (char *str, size_t amt) {
	size_t i, size = strlen(str);
	if (amt >= size) {
    		memset(str, nullterm, size);
    		return str;
	}
	for (i = 0; i < size-amt; i++) {
		if (i + amt > size)
			break;
    		str[i] = str[i + amt];
    		str[i + amt] = nullterm;
	}
	if (strlen(str) > size - amt)
		substr(str, str, 0, size - amt);
	return str;
}

char *strtok(char *dest, char *src, char *delim) {
	if (src[0] == nullterm)
		return NULL;
	clearchar(dest);
	size_t i, di;
	for (di = 0; delim[di] != nullterm; di++) {
		for (i = 0; (src[i] != delim[di]) && (src[i] != nullterm); i++){
			dest[i] = src[i];
		}
		src = strshl(src, i+1);
	}
	if (dest[0] == nullterm)
		return NULL;
	return dest;
}

bool isspace(char c) {
	if (	c == ' ' || c == '\n' ||
		c == '\t' || c == '\v' ||
		c == '\f' || c == '\r')
		return true;
	else
		return false;
}

char *ltrim(char *s) {
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s)); 
}
#endif
