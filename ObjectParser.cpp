#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Global.h"
#include "Object.h"
#include "Interface.h"



static char *
consumeChance (char *buf, int *result)
{
    char *start = buf;
    int n = 0;
    *result = 100;
    while (isspace (*buf)) ++buf;
    while (isdigit (*buf)) {
        n = 10 * n + *buf - '0';
        ++buf;
    }
    if ('%' != *buf++) {
        return start;
    }
    if (n) {
        *result = n;
        if (!(0 == *buf || isspace (*buf))) {
            return start;
        }
    } 
    while (isspace (*buf)) ++buf;
    return buf;
}


static char *
consumeArticle (char *buf)
{
    while (isspace (*buf)) ++buf;

    if (0 == strncmp ("the ", buf, 4)) {
        buf += 4;
    } else if (0 == strncmp ("a ", buf, 2)) {
        buf += 2;
    } else if (0 == strncmp ("an ", buf, 3)) {
        buf += 3;
    }
    while (isspace (*buf)) ++buf;
    return buf;
}


static char *
consumeCount (char *buf, int *result)
{
    int n = 0;
    int d = 0;
    while (isspace (*buf)) ++buf;
    while (isdigit (*buf)) {
        n = 10 * n + *buf - '0';
        ++buf;
    }
    if (n) {
        if ('d' == *buf) {
            ++buf;
            while (isdigit (*buf)) {
                d = 10 * d + *buf - '0';
                ++buf;
            }
            if (0 == d) {
                return NULL;
            }
            n = NDX (n, d);
        }
        if (!(0 == *buf || isspace (*buf))) {
            return NULL;
        }
        *result = n;
    } else {
        buf = consumeArticle (buf);
    }
    while (isspace (*buf)) ++buf;
    return buf;
}


static char *
consumeBugginess (char *buf, int *result)
{
    while (isspace (*buf)) ++buf;
    if (0 == strncmp ("buggy ", buf, 6)) {
        *result = -1;
        buf += 6;
    } else if (0 == strncmp ("debugged ", buf, 9)) {
        *result = 0;
        buf += 9;
    } else if (0 == strncmp ("optimized ", buf, 10)) {
        *result = 1;
        buf += 10;
    } else {
        *result = -2;
    }
    while (isspace (*buf)) ++buf;
    return buf;
}


static char *
consumeEnhancement (char *buf, int *result)
{
    int n = 0;
    while (isspace (*buf)) ++buf;
    if ('+' == *buf) {
        while (isdigit (*++buf)) {
            n = 10 * n + *buf - '0';
        }
        *result = n;
    } else if ('-' == *buf) {
        while (isdigit (*++buf)) {
            n = 10 * n + *buf - '0';
        }
        *result = -n;
    } else {
        *result = -22;
        return buf;
    }
    if (!(0 == *buf || isspace (*buf))) {
        return NULL;
    } else {
        return buf;
    }
}



static char *
consumeCracked (char *buf, int *result)
{
    *result = 0;
    while (isspace (*buf)) ++buf;
    if (0 == strncmp ("cracked ", buf, 7)) {
        *result = 1;
        buf += 7;
    } 
    while (isspace (*buf)) ++buf;
    return buf;
}


/* test for compound noun such as "canisters of superglue" */
const static char *compoundnouns[] = {
    " of ",
    " labeled ",
    " called ",
    " named ",
    NULL
};


void
makePlural (char *buf, int len)
{
    char *mid;
    const char *t;
    int i;
    int blen = strlen (buf);

    if (blen + 1 >= len) {
        return;
    }

    for (i = 0, mid = NULL;
         (t = compoundnouns[i]);
         ++i)
    {
        mid = strstr (buf + 1, t);
        if (mid) { /* add an 's' */
            memmove (mid + 1 , mid, strlen (mid) + 1);
            *mid = 's';
            return;
        }
    }
    
    buf[blen] = 's';
    buf[blen+1] = 0;
}


/* MODIFIES: writes the singular form of the noun in buf into result */

char *
makeSingular (char *buf, 
              char *result, int len) /* result: a buffer of length len */
{
    char *mid;
    const char *t;
    int i;
    int blen;
    char save;

    while (isspace (*buf)) ++buf;
        
    for (i = 0, mid = NULL;
         (t = compoundnouns[i]);
         ++i)
    {
        mid = strstr (buf + 1, t);
        if (mid) { /* overwrite the 's' */
            I->debug ("found compound %s", t);
            --mid;
            save = *mid;
            if ('s' == save) {
                *mid = 0;
                snprintf (result, len, "%s%s", buf, mid + 1);
                *mid = save;
            } else {
                snprintf (result, len, "%s", buf);
            }
            return result;
        }
    }

/* assume the straightforward case of a noun ending in 's' */
    blen = strlen (buf) - 1;
    while (blen && isspace(buf[blen])) {
        --blen;
    }
    if ('s' == buf[blen]) {
        snprintf (result, len, "%s", buf);
        result[blen] = 0;
    } else { /* assume it's singular already; copy verbatim */
        snprintf (result, len, "%s", buf);
    }
    return result;
}


shObject *
createObject (const char *desc, int flags)
{
    int count = -22;
    int bugginess = -22;
    int enhancement = -22;
    int cracked = -22;
    int charges = -22;
    char ilkdesc[50];
    shObject *obj;
    char buffer[256];
    char *str;
    int chance;
    int pass;

    ilkdesc[0] = 0;

    /* make a copy of the string */

    strncpy (buffer, desc, 255); buffer[255] = 0;

    for (str = buffer; *str; str++)
        *str = tolower (*str);
    str = buffer;

    I->debug ("parsing %s", str);
    
    str = consumeChance (str, &chance);
    I->debug ("Chance %d, %s", chance, str);
    if (RNG(100) > chance || NULL == str) {
        return NULL;
    }
    str = consumeCount (str, &count);
    I->debug ("Count %d, %s", count, str);
    if (NULL == str) return NULL;
    str = consumeBugginess (str, &bugginess);
    I->debug ("Buggy %d, %s", bugginess, str);
    if (NULL == str) return NULL;
    str = consumeEnhancement (str, &enhancement);
    I->debug ("Enhancement %d, %s", enhancement, str);
    if (NULL == str) return NULL;
    str = consumeCracked (str, &cracked);
    I->debug ("Cracked %d, %s", cracked, str);
    if (NULL == str) return NULL;
  
    str = makeSingular (str, ilkdesc, 50);
    I->debug ("ilk %s, desc", ilkdesc, str);

    for (pass = 0; pass < 2; pass++) {
        if (0 == strcmp (ilkdesc, "buckazoid")) {
            return createMoney (count);
        }
        if (0 == strcmp (ilkdesc, "energy cell")) {
            return createEnergyCell (count);
        }
        obj = createWeapon (ilkdesc, count, bugginess, enhancement, charges);
        if (obj) return obj;
        obj = createRayGun (ilkdesc, bugginess, charges);
        if (obj) return obj;
        obj = createArmor (ilkdesc, count, bugginess, enhancement, charges);
        if (obj) return obj;
        obj = createTool (ilkdesc, count, bugginess, enhancement, charges);
        if (obj) return obj;
        obj = createCanister (ilkdesc, count, bugginess, enhancement, charges);
        if (obj) return obj;
        obj = createFloppyDisk (ilkdesc, count, bugginess, enhancement,
                                charges);
        if (obj) {
            if (cracked) 
                obj->setCracked ();
            return obj;
        }
        obj = createImplant (ilkdesc, count, bugginess, enhancement, charges);
        if (obj) return obj;
        
    }

    return NULL;
}
