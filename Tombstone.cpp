#include <ctype.h>

#include "Global.h"
#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Hero.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif


/* calculation of score:

1 point per XP earned
500 points per level visited besides level 1
10000 for finding the bizarro orgasmatron

 */

void
shHero::tallyScore ()
{

}


static int 
copystr (char *dest, char *src, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        dest[i] = src[i];
        if (0 == src[i]) {
            return -1;
        }
    }
    while (i > 0) {
        --i;
        if (isspace (src[i])) {
            dest[i] = 0;
            return i+1;
        }
    }
    dest[i] = 0;
    return len;
}


const char *
shMapLevel::getDescription ()
{
    switch (Level->mType) {
    case shMapLevel::kTown: return "in Robot Town";
    case shMapLevel::kRadiationCave: return "in the Gamma Caves";
    case shMapLevel::kMainframe: return "in the Mainframe";
    case shMapLevel::kRabbit: return "in the Rabbit's Hole";
    case shMapLevel::kSewer: return "in the sewer";
    case shMapLevel::kSewerPlant: 
        return "in the waste treatment plant";
    default:
        return "in the space base";
    }
}


void
shHero::epitaph (char *buf, int len, shCauseOfDeath how, const char *killer)
{

    switch (how) {
    case kSlain:
    case kKilled:
        if (killer && !strcasecmp (killer, "a dalek")) 
            snprintf (buf, len, "Exterminated by %s", killer);
        else if (killer && (!strcasecmp (killer, "a low ping bastard") ||
                            !strcasecmp (killer, "a high ping bastard")))
            snprintf (buf, len, "Pwned by %s", killer);
        else 
            snprintf (buf, len, "Killed by %s", killer ? killer : "a monster");
        break;
    case kAnnihilated:
        snprintf (buf, len, "Annihilated by %s", 
                  killer ? killer : "a monster");
        break;
    case kEmbarassment:
        snprintf (buf, len, "Died of embarassment");
        break;
    case kSuddenDecompression:
        snprintf (buf, len, "Died of vaccuum exposure");
        break;
    case kTransporterAccident:
        snprintf (buf, len, "Died in a transporter accident");
        break;
    case kSuicide:
        snprintf (buf, len, "Committed suicide");
        break;
    case kDrowned:
        snprintf (buf, len, "Drowned %s", killer);
        break;
    case kBrainJarred:
        snprintf (buf, len, "Brain-napped by %s", killer);
        break;
    case kMisc:
        snprintf (buf, len, "%s", killer);
        break;
    case kQuitGame:
        snprintf (buf, len, "Quit");
        break;
    case kWonGame:
        snprintf (buf, len, "Activated the Bizarro Orgasmatron");
        break;
    }

    snprintf (&buf[strlen (buf)], len, " %s at depth %d.", 
              Level->getDescription (), Level->mDLevel);
}



void
shHero::tomb (char *message)
{
    char buf1[80];
    char buf2[80];
    int n, l;
    int row;
    WINDOW *win;
    PANEL *panel;

    win = newwin (25, 60, 0, 0);
    if (!win) {
        I->p ("Couldn't create tombstone.");
        return;
    }
    panel = new_panel (win);

    wattrset (win, ColorMap[kBrightCyan]);

    row = 3;

    mvwaddstr (win, row++, 6,  "            _________            ");
    mvwaddstr (win, row++, 6,  "           /         \\           ");
    mvwaddstr (win, row++, 6,  "          /  _     _  \\          ");
    mvwaddstr (win, row++, 6,  "         /  | \\ | | \\  \\         ");
    mvwaddstr (win, row++, 6,  "        /   |_/ | |_/   \\        ");
    mvwaddstr (win, row++, 6,  "       /    | \\ | |      \\       ");
    mvwaddstr (win, row++, 6,  "      |                   |      ");
    mvwaddstr (win, row++, 6,  "      |                   |      ");
    mvwaddstr (win, row++, 6,  "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 6, "      |                   |      ");
    mvwaddstr (win, row++, 2, 
               "----------+-------------------+--------------------------");

    row -= 10;

    wattrset (win, ColorMap[kWhite]);
    snprintf (buf1, 20, "%s", mName);
    mvwaddstr (win, row, 22 - strlen (buf1) / 2, buf1);

    snprintf (buf1, 18, "%s", mProfession->mTitles[mCLevel/3]);
    wattrset (win, ColorMap[kGray]);
    mvwaddstr (win, ++row, 22 - strlen (buf1) / 2, buf1);

    row += 2;
    n = 0;
    do {
        l = copystr (buf2, &message[n], 18);
        n += l;
        mvwaddstr (win, row, 22 - strlen (buf2) / 2, buf2);
        row++;
    } while (l >= 0);

    wmove (win, 0, 0);

    update_panels ();
    doupdate();
    I->getChar ();
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
}


struct HighScoreEntry
{
    int mScore;
    int mUid;
    char mName[30];
    char mMessage[200];
};


static void
displayScoreEntry (HighScoreEntry *e, int rank, int yours, WINDOW *win, int y)
{
    char buf[200];
    char buf2[78];
    int l, n;

    if (yours) {
        wattrset (win, ColorMap[kBrightRed]);
    } else {
        wattrset (win, ColorMap[kGray]);
    }
    if (rank >= 0) {
        snprintf (buf, 200, "%d.", rank + 1);
        mvwaddstr (win, y, 2, buf);
    }
    
    snprintf (buf, 200, "%8d", e->mScore);
    mvwaddstr (win, y, 6, buf);
    snprintf (buf, 200, "%s, %s", e->mName, e->mMessage);

    n = 0;
    do {
        l = copystr (buf2, &buf[n], 60);
        n += l;
        mvwaddstr (win, y, 15, buf2);
        y++;
    } while (l >= 0);

}


void
shHero::logGame (char *message)
{
    char logname[ZAPM_PATH_LENGTH];
    int fd;
    time_t clocktime;
    struct tm *when;

    time (&clocktime);
    when = localtime (&clocktime);

    snprintf (logname, sizeof(logname)-1, "%s/logfile.txt", DataDir);
    
    fd = open (logname, O_WRONLY | O_APPEND | O_CREAT, 
#ifdef _WIN32
               S_IREAD | S_IWRITE);
#else
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif

    if (-1 == fd) {
        I->p ("Couldn't open logfile.");
        I->pause ();
    } else {
        char logstr[1024];
        snprintf (logstr, sizeof(logstr), 
                  "%s %04d-%02d-%02d %10d %2d %6d %s the %s %s%s\n", 
                  ZAPM_VERSION, 
                  when->tm_year + 1900, when->tm_mon + 1, when->tm_mday, 
                  mScore, mCLevel, mXP, mName, 
                  mProfession->mTitles[mCLevel/3], 
                  GodMode? "(godmode) " : "", message);
        write (fd, logstr, strlen (logstr));
        close (fd);
    }

    /* check high score file */

    if (GodMode) {
        I->p ("As god, you are ineligible for the highscore list.");
        return;
    }

#define NUMHIGHSCORES 10
    HighScoreEntry scores[NUMHIGHSCORES+1];
    int nscores = 0;
    int entry = 0;
    int i;

    snprintf (logname, sizeof(logname)-1, "%s/highscores.dat", DataDir);
    fd = open (logname, O_RDONLY | O_BINARY, 
#ifdef _WIN32
        S_IREAD | S_IWRITE);
#else
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif

    if (-1 != fd) {
        int numread;
        while (numread = read (fd, &scores[nscores], sizeof (HighScoreEntry)),
               numread == sizeof (HighScoreEntry))
        {
            if (mScore < scores[nscores].mScore) {
                entry = nscores + 1;
            }
            nscores++;
        }
        if (numread != -1) {
            I->p ("The high score table is corrupt, attempting to repair...");
        }
        close (fd);
    } else {
        /* create a new high score table */
        nscores = 0;
        entry = 0;
    }

    if (NUMHIGHSCORES == entry) {
        /* didn't make a high score */
        scores[entry].mScore = mScore;
#ifdef _WIN32
        scores[entry].mUid = 0;
#else
        scores[entry].mUid = getuid ();
#endif
        strncpy (scores[entry].mName, mName, 30);
        strncpy (scores[entry].mMessage, message, 200);
    } else {
        if (++nscores > NUMHIGHSCORES) {
            nscores = NUMHIGHSCORES;
        }
        for (i = nscores - 1; i > entry; i--) {
            memcpy (&scores[i], &scores[i-1], sizeof (HighScoreEntry));
        }
        scores[entry].mScore = mScore;
#ifndef _WIN32
        scores[entry].mUid = getuid ();
#else
        scores[entry].mUid = 0;
#endif
        strncpy (scores[entry].mName, mName, 30);
        strncpy (scores[entry].mMessage, message, 200);

        fd = open (logname, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 
#ifdef _WIN32
                   S_IREAD | S_IWRITE);
#else
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
        if (-1 == fd) {
            I->p ("Couldn't write high score file.");
            return;
        }
        for (i = 0; i < nscores; i++) {
            if (sizeof (HighScoreEntry) != 
                write (fd, &scores[i], sizeof (HighScoreEntry)))
            {
                I->p ("error writing high score file");
                close (fd);
                return;
            }
        }
        close (fd);
    }
    
    WINDOW *win;
    PANEL *panel;
    int row = 1;

    win = newwin (25, 80, 0, 0);
    if (!win) {
        I->p ("Couldn't display high scores.");
        return;
    }
    panel = new_panel (win);
    
    if (NUMHIGHSCORES == entry) {
        mvwaddstr (win, row, 1, "You didn't make the high score list.");
        row += 2;
        displayScoreEntry (&scores[NUMHIGHSCORES], -1, 1, win, row);    
    } else {
        mvwaddstr (win, row, 1, "You made the high score list!");
    }
    for (i = 0; i < nscores; i++) {
        row+=2;
        displayScoreEntry (&scores[i], i, entry == i, win, row);
    }
    wattrset (win, ColorMap[kGray]);
    mvwaddstr (win, row + 2, 1, "--End--");
    
    update_panels ();
    doupdate();
    I->getChar (win);
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
    
}


