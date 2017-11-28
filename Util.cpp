#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "Util.h"


int
RNG (int max)
{
  return random () % max;
}

int
RNG (int min, int max)
{
    return min + random () % (max + 1 - min);
}

int
NDX (int n, int x)
{
    int result = 0;
    while (n--) {
        result += RNG (1, x);
    }
    return result;
}


int 
mini (int x, int y) 
{
    return x < y ? x : y;
}


int 
maxi (int x, int y) 
{
    return x > y ? x : y;
}


char IsVowelLookupTable[256];


void
utilInitialize ()
{
    srandom ((unsigned) time (NULL));
    
    IsVowelLookupTable['a'] = 1;
    IsVowelLookupTable['e'] = 1;
    IsVowelLookupTable['i'] = 1;
    IsVowelLookupTable['o'] = 1;
    IsVowelLookupTable['u'] = 1;
}


void
shuffle (void *array, int nmemb, int size) {
    int i;
    char tmp[50];

    assert (size <= 50);

    for (i = 0; i < nmemb; ++i) {
        int x = i + RNG (nmemb - i);
        memcpy (tmp, (void *) ((unsigned long) array + x * size), size);
        memcpy ((void *) ((unsigned long) array + x * size),
                (void *) ((unsigned long) array + i * size), size);
        memcpy ((void *) ((unsigned long) array + i * size), tmp, size);
    }
}


void
insertionsort (void *base, size_t nmemb, size_t size, 
                int (*compar) (const void *, const void *))
{
    int i;
    unsigned int j;
    char tmp[20];
    
    assert (size < 20);

    for (j = 1; j < nmemb; j++) {
        memcpy (tmp, (void *) ((unsigned long) base + j * size), size);
        i = j - 1; 
        while (i >= 0 && 
               compar ((void *) ((unsigned long) base + i * size), tmp) > 0)
        {
            memmove ((void *) ((unsigned long) base + (i + 1) * size), 
                     (void *) ((unsigned long) base + i * size), size);
            --i;
        }
        memcpy ((void *) ((unsigned long) base + (i + 1) * size), tmp, size);
    }
}
