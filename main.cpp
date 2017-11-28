#include "Global.h"
#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Monster.h"
#include "Hero.h"
#include "Game.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

shInterface *I;
shMapLevel *Level;
char DataDir[256];
int GodMode = 0;


int
main (int argc, char **argv)
{
    struct rlimit coredumpsize;
    int rlimitresult;

    coredumpsize.rlim_cur = RLIM_INFINITY;
    coredumpsize.rlim_max = RLIM_INFINITY;
    rlimitresult = setrlimit (RLIMIT_CORE, &coredumpsize);

    snprintf (DataDir, 255, "%s", DATADIR);
    if (argc > 1) {
        if (0 == strcmp (argv[1], "-god")) {
            GodMode = 1;
        }
    }

    utilInitialize ();

    I = new shInterface ();    
    if (rlimitresult) {
        I->debug ("Couldn't adjust coredumpsize.");
    }

    initializeOptions ();
    initializeProfessions ();
    initializeMonsters ();

    I->showVersion ();

    if (0 == loadGame () ||
        0 == newGame ()) 
    {
        I->p ("Welcome to \"ZapM\".  Press '?' for help.");
        I->p ("Thanks for playtesting my game!");
        I->p ("Please submit bug reports at http://zapm.org/bb/");
        if (GodMode) {
            I->p ("God Mode is on.");
        }
        gameLoop ();
    }
    delete I;

    fprintf (stdout, "Thanks for playing!\n");
    fprintf (stdout, "Please submit bug reports at http://zapm.org/bb/\n");
}
