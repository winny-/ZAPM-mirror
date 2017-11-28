#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include <direct.h>
#include <io.h>
#define getcwd _getcwd
#define access _access
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "Global.h"
#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Monster.h"
#include "Hero.h"
#include "Game.h"

shInterface *I;
shMapLevel *Level;
char DataDir[ZAPM_PATH_LENGTH];
int GodMode = 0;



static void 
usage ()
{
    fprintf (stdout, "usage: zapm [-u username]\n"
                     "       A science fiction themed \"roguelike\" game\n"
                     "       http://zapm.org\n");
}


int
main (int argc, char **argv)
{
    int rlimitresult = 0;
#ifndef _WIN32
	struct rlimit coredumpsize;

    coredumpsize.rlim_cur = RLIM_INFINITY;
    coredumpsize.rlim_max = RLIM_INFINITY;
    rlimitresult = setrlimit (RLIMIT_CORE, &coredumpsize);
#endif
    snprintf (DataDir, ZAPM_PATH_LENGTH, "%s", DATADIR);

    const char *name = NULL;
    char namebuf[HERO_NAME_LENGTH+2];
    int c;
    int g = 0, o = 0, d = 0;

    //FIXME: should probably stat() it and make sure it's a directory...
    //FIXME: access() doesn't work for setuid install
    if (0 && access (DataDir, O_RDWR)) {
        fprintf (stderr, "Couldn't open \"%s\" directory: %s.\n", DataDir, strerror (errno));
        fprintf (stderr, "ZAPM needs this directory for saved games and other data.\n"
                 "Perhaps you were running it from a strange directory?\n");
        exitZapm (-1);
    }

#ifndef _WIN32
    while (-1 != (c = getopt (argc, argv, "u:dgo"))) {
        switch (c) {
        case 'd': d++; break;
        case 'g': g++; break;
        case 'o': o++; break;
        case 'u': 
            name = strdup (optarg); 
            if (!nameOK (name)) {
                fprintf (stderr, "Sorry, can't play a game as \"%s\".\n", 
                         name);
                return -1;
            }
            break;
        case '?':
        default:
            usage (); 
            return 0;
        }
    }
#else
    //windows doesn't have getopt()
    if (2 == argc && !strcmp("-god", argv[1]))
        d = g = o = 1;
#endif
    if (d && g && o)
        GodMode = 1;

    utilInitialize ();

    I = new shInterface ();    
    if (rlimitresult) {
        I->debug ("Couldn't adjust coredumpsize.");
    }

    initializeOptions ();
    initializeProfessions ();
    initializeMonsters ();

    I->showVersion ();

    if (!name) {
        int tries = 5;

        name = getenv ("USER");
        if (!name)
            name = getenv ("USERNAME");

        do {
            if (!tries--) {
                I->p ("Too many tries!");
                I->pause ();
                I->p ("");
                goto done;
            }
            I->p ("Welcome to ZAPM!");
            I->getStr (namebuf, HERO_NAME_LENGTH+1, 
                       "What is your name?", name);
            I->pageLog ();
        } while (!nameOK (namebuf));
        name = namebuf;
    }

    if (0 == loadGame (name) ||
        0 == newGame (name)) 
    {
        I->p ("Welcome to \"ZapM\".  Press '?' for help.");
        I->p ("Please submit bug reports at http://zapm.org/bb/");
        if (GodMode) {
            I->p ("God Mode is on.");
        }
        gameLoop ();
    }

done:
    delete I;

    fprintf (stdout, "Thanks for playing!\n");
    fprintf (stdout, "Please submit bug reports at http://zapm.org/bb/\n");
    exitZapm (0);
}
