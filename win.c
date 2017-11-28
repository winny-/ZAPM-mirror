/*

Windoes is teh suxxks!!

*/

#include <stdarg.h>
#include <stdio.h>


static char bigbuff[4096];


int
vsnprintf (char *str, size_t size, const char *format, va_list ap)
{
    int result;

    result = vsprintf (bigbuff, format, ap);
    if (result >= size) result = size - 1;
    memcpy (str, bigbuff, result);
    str[result] = 0;
    return result;
}



int
snprintf (char *str, size_t size, const char * format, ...)
{
    int result;
    va_list ap;

    va_start (ap, format);
    result = vsnprintf (str, size, format, ap);
    va_end (ap);
    return result;
}
