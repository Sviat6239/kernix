#include "string.hpp"
#include "../syscalls/syscalls.hpp"
#include "stdint.h"

int strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b))
    {
        a++;
        b++;
    }

    return *(const unsigned char *)a - *(const unsigned char *)b;
}

char *find_space(char *str)
{
    while (*str)
    {
        if (*str == ' ')
            return str;
        str++;
    }
    return nullptr;
}

unsigned int strlen(const char *str)
{
    unsigned int len = 0;
    while (*str)
    {
        len++;
        str++;
    }
    return len;
}

void ksys_print(const char *str)
{
    if (!str)
        return;

    uint32_t len = 0;
    const char *p = str;
    while (*p)
    {
        len++;
        p++;
    }

    ksys_write(str, len);
}