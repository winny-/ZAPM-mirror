#ifndef _PANEL_H
extern "C" {
#include <panel.h>
}
#define _PANEL_H
#endif
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Hero.h"

#define CTRL(_k) (0x1f & (_k))
#define META(_k) (0x80 | (_k))

int ColorMap[22];

//constructor:
shInterface::shInterface ()
{
    int i;
    WINDOW *win;

    win = initscr ();

    if (!win) {
        fprintf (stderr, "Sorry, curses support is required.\n");
        exitZapm (-1);
    }

    if (!has_colors ()) {
        endwin ();
        fprintf (stderr, "Sorry, color support is required.\n");
        exitZapm (-1);
    }
    start_color ();

    for (i = 1; i < COLOR_PAIRS; i++) {
        init_pair (i, i %8, i/8);
    }


#ifdef _WIN32
    curs_set (2); /* use block cursor */
    ColorMap[kBlack] = COLOR_BLACK;
    ColorMap[kRed] = COLOR_PAIR (4) | A_BOLD;
    ColorMap[kGreen] = COLOR_PAIR (2) | A_BOLD;
    ColorMap[kYellow] = COLOR_PAIR (6) | A_BOLD;
    ColorMap[kBlue] = COLOR_PAIR (1) | A_BOLD;
    ColorMap[kMagenta] = COLOR_PAIR (5)| A_BOLD;
    ColorMap[kCyan] = COLOR_PAIR (3) | A_BOLD;
    ColorMap[kGray] = COLOR_PAIR (7);
    ColorMap[kBrightRed] = COLOR_PAIR (4) | A_BOLD;
    ColorMap[kBrightGreen] = COLOR_PAIR (2) | A_BOLD;
    ColorMap[kBrightYellow] = COLOR_PAIR (6) | A_BOLD;
    ColorMap[kBrightBlue] = COLOR_PAIR (1)| A_BOLD;
    ColorMap[kBrightMagenta] = COLOR_PAIR (5) | A_BOLD;
    ColorMap[kBrightCyan] = COLOR_PAIR (3) | A_BOLD;
    ColorMap[kWhite] = COLOR_PAIR (7) | A_BOLD;
    
    ColorMap[kDarkRed] = COLOR_PAIR (4) | A_DIM;
    ColorMap[kDarkGreen] = COLOR_PAIR (2) | A_DIM;
    ColorMap[kBrown] = COLOR_PAIR (6);
    ColorMap[kDarkBlue] = COLOR_PAIR (1)| A_DIM;
    ColorMap[kDarkMagenta] = COLOR_PAIR (5) | A_DIM;
    ColorMap[kDarkCyan] = COLOR_PAIR (3) | A_DIM;
    ColorMap[kDarkGray] = COLOR_PAIR (7) | A_DIM;

#else
    ColorMap[kBlack] = COLOR_BLACK;
    ColorMap[kRed] = COLOR_PAIR (1);
    ColorMap[kGreen] = COLOR_PAIR (2);
    ColorMap[kYellow] = COLOR_PAIR (3);
    ColorMap[kBlue] = COLOR_PAIR (4);
    ColorMap[kMagenta] = COLOR_PAIR (5);
    ColorMap[kCyan] = COLOR_PAIR (6);
    ColorMap[kGray] = COLOR_PAIR (7);
    ColorMap[kBrightRed] = COLOR_PAIR (1) | A_BOLD;
    ColorMap[kBrightGreen] = COLOR_PAIR (2) | A_BOLD;
    ColorMap[kBrightYellow] = COLOR_PAIR (3) | A_BOLD;
    ColorMap[kBrightBlue] = COLOR_PAIR (4)| A_BOLD;
    ColorMap[kBrightMagenta] = COLOR_PAIR (5) | A_BOLD;
    ColorMap[kBrightCyan] = COLOR_PAIR (6) | A_BOLD;
    ColorMap[kWhite] = COLOR_PAIR (7) | A_BOLD;

    /* A_DIM doesn't seem to do much, but... */
    ColorMap[kDarkRed] = COLOR_PAIR (1) | A_DIM;
    ColorMap[kDarkGreen] = COLOR_PAIR (2) | A_DIM;
    ColorMap[kBrown] = COLOR_PAIR (3) | A_DIM;
    ColorMap[kDarkBlue] = COLOR_PAIR (4)| A_DIM;
    ColorMap[kDarkMagenta] = COLOR_PAIR (5) | A_DIM;
    ColorMap[kDarkCyan] = COLOR_PAIR (6) | A_DIM;
    ColorMap[kDarkGray] = COLOR_PAIR (7) | A_DIM;

    






#endif

    mColor = kGray;

//    cbreak ();
    raw ();
    noecho ();
#ifdef DJGPP
    nl ();
#else
    nonl ();
#endif

    mXMax = 64;
    mYMax = 20;

    mMainWin = newwin (20, mXMax, 0, 0);
    if (!mMainWin) goto toosmall;
    new_panel (mMainWin);
    notimeout (mMainWin, TRUE);
    mSideWin = newwin (20, 80 - mXMax, 0, mXMax);
    if (!mSideWin) goto toosmall;
    new_panel (mSideWin);
    mLogWin = newwin (5, 80, 20, 0);
    if (!mLogWin) goto toosmall;
    mDiagWin = NULL;
    if (GodMode) {
        mDiagWin = newwin (10, 80, 25, 0);
        scrollok (mDiagWin, TRUE);
    }
    notimeout (mLogWin, TRUE);
    mLogSize = 5;
    mLogRow = 0;
    mHistoryIdx = 0;
    mHistoryWrapped = 0;
    mDirty = 0;
    mNoNewline = 0;
    mLogSCount = 0;
    mPause = 0;

    mX0 = 0;
    mY0 = 0;

    intrflush (mMainWin, FALSE);
//    keypad (mMainWin, TRUE);
    intrflush (mLogWin, FALSE);
//    keypad (mLogWin, TRUE);
    scrollok (mLogWin, TRUE);

    initializeCommands ();
    initializeGlyphs ();

#ifdef SH_DEBUG
    char dbgfilename[40];
    snprintf (dbgfilename, 40, "%s/dbg.%d.txt", DataDir, getuid ());
    mDbgFile = fopen (dbgfilename, "w");
    if (!mDbgFile) {
        endwin ();
        fprintf (stderr, "Sorry, couldn't open %s\n", dbgfilename);
        exitZapm (-1);
    }
    setlinebuf (mDbgFile);
#endif

    debug ("COLORS: %d", COLORS);
    debug ("COLOR_PAIRS: %d", COLOR_PAIRS);
    debug ("can change: %d", can_change_color());

    return;

toosmall:
    endwin ();
    fprintf (stderr, "Sorry, but a terminal with dimensions of at least 80x25"
             " is required.\n");
    exitZapm (-1);
}

shInterface::~shInterface ()
{
    endwin ();
#ifdef SH_DEBUG
    fclose (mDbgFile);
#endif
};


#define MAPKEY(_k,_c) mKey2Cmd[(_k)] = (_c); 


/* remaps direction keys: if on is true, then Crazy Ivan mode, o/w normal  */
void
shInterface::crazyIvan (int on)
{
    if (on) {
        MAPKEY ('3', kMoveNW);
        MAPKEY ('2', kMoveN);
        MAPKEY ('1', kMoveNE);
        MAPKEY ('4', kMoveE);
        MAPKEY ('7', kMoveSE);
        MAPKEY ('8', kMoveS);
        MAPKEY ('9', kMoveSW);
        MAPKEY ('6', kMoveW);

#ifdef CTRLFIRE
        MAPKEY (CTRL('3'), kFireNW);
        MAPKEY (CTRL('2'), kFireN);
        MAPKEY (CTRL('1'), kFireNE);
        MAPKEY (CTRL('4'), kFireE);
        MAPKEY (CTRL('7'), kFireSE);
        MAPKEY (CTRL('8'), kFireS);
        MAPKEY (CTRL('9'), kFireSW);
        MAPKEY (CTRL('6'), kFireW);
#endif

        if (Flags.mVIKeys) {
            MAPKEY ('n', kMoveNW);
            MAPKEY ('j', kMoveN);
            MAPKEY ('b', kMoveNE);
            MAPKEY ('h', kMoveE);
            MAPKEY ('y', kMoveSE);
            MAPKEY ('k', kMoveS);
            MAPKEY ('u', kMoveSW);
            MAPKEY ('l', kMoveW);

            MAPKEY ('N', kGlideNW);
            MAPKEY ('J', kGlideN);
            MAPKEY ('B', kGlideNE);
            MAPKEY ('H', kGlideE);
            MAPKEY ('Y', kGlideSE);
            MAPKEY ('K', kGlideS);
            MAPKEY ('U', kGlideSW);
            MAPKEY ('L', kGlideW);

#ifdef CTRLFIRE
            MAPKEY (CTRL('n'), kFireNW);
            MAPKEY (CTRL('j'), kFireN);
            MAPKEY (CTRL('b'), kFireNE);
            MAPKEY (CTRL('h'), kFireE);
            MAPKEY (CTRL('y'), kFireSE);
            MAPKEY (CTRL('k'), kFireS);
            MAPKEY (CTRL('u'), kFireSW);
            MAPKEY (CTRL('l'), kFireW);
#else
            MAPKEY (CTRL('N'), kName);
#endif
        } else {
            MAPKEY ('n', kNoCommand);
            MAPKEY ('j', kNoCommand);
            MAPKEY ('b', kNoCommand);
            MAPKEY ('h', kNoCommand);
            MAPKEY ('y', kNoCommand);
            MAPKEY ('u', kNoCommand);
            MAPKEY ('k', kKick);
            MAPKEY ('l', kNoCommand);

            MAPKEY ('N', kName);
            MAPKEY ('J', kNoCommand);
            MAPKEY ('B', kNoCommand);
            MAPKEY ('H', kHistory);
            MAPKEY ('Y', kNoCommand);
            MAPKEY ('K', kNoCommand);
            MAPKEY ('U', kUninstall);
            MAPKEY ('L', kNoCommand);



#ifdef CTRLFIRE
            MAPKEY (CTRL('n'), kNoCommand);
            MAPKEY (CTRL('j'), kNoCommand);
            MAPKEY (CTRL('b'), kNoCommand);
            MAPKEY (CTRL('h'), kNoCommand);
            MAPKEY (CTRL('y'), kNoCommand);
            MAPKEY (CTRL('u'), kNoCommand);
            MAPKEY (CTRL('k'), kNoCommand);
            MAPKEY (CTRL('l'), kNoCommand);
#else
            MAPKEY (CTRL('N'), kName);
#endif
        }
        MAPKEY ('<', kMoveDown);
        MAPKEY ('>', kMoveUp);
    } else {
        MAPKEY ('7', kMoveNW);
        MAPKEY ('8', kMoveN);
        MAPKEY ('9', kMoveNE);
        MAPKEY ('6', kMoveE);
        MAPKEY ('3', kMoveSE);
        MAPKEY ('2', kMoveS);
        MAPKEY ('1', kMoveSW);
        MAPKEY ('4', kMoveW);

#ifdef CTRLFIRE
        MAPKEY (CTRL('7'), kFireNW);
        MAPKEY (CTRL('8'), kFireN);
        MAPKEY (CTRL('9'), kFireNE);
        MAPKEY (CTRL('6'), kFireE);
        MAPKEY (CTRL('3'), kFireSE);
        MAPKEY (CTRL('2'), kFireS);
        MAPKEY (CTRL('1'), kFireSW);
        MAPKEY (CTRL('4'), kFireW);
#endif
        if (Flags.mVIKeys) {
            MAPKEY ('y', kMoveNW);
            MAPKEY ('k', kMoveN);
            MAPKEY ('u', kMoveNE);
            MAPKEY ('l', kMoveE);
            MAPKEY ('n', kMoveSE);
            MAPKEY ('j', kMoveS);
            MAPKEY ('b', kMoveSW);
            MAPKEY ('h', kMoveW);

            MAPKEY ('Y', kGlideNW);
            MAPKEY ('K', kGlideN);
            MAPKEY ('U', kGlideNE);
            MAPKEY ('L', kGlideE);
            MAPKEY ('N', kGlideSE);
            MAPKEY ('J', kGlideS);
            MAPKEY ('B', kGlideSW);
            MAPKEY ('H', kGlideW);

#ifdef CTRLFIRE
            MAPKEY (CTRL('y'), kFireNW);
            MAPKEY (CTRL('k'), kFireN);
            MAPKEY (CTRL('u'), kFireNE);
            MAPKEY (CTRL('l'), kFireE);
            MAPKEY (CTRL('n'), kFireSE);
            MAPKEY (CTRL('j'), kFireS);
            MAPKEY (CTRL('b'), kFireSW);
            MAPKEY (CTRL('h'), kFireW);
#endif
        } else {
            MAPKEY ('y', kNoCommand);
            MAPKEY ('k', kKick);
            MAPKEY ('u', kNoCommand);
            MAPKEY ('l', kNoCommand);
            MAPKEY ('n', kNoCommand);
            MAPKEY ('j', kNoCommand);
            MAPKEY ('b', kNoCommand);
            MAPKEY ('h', kNoCommand);

            MAPKEY ('Y', kNoCommand);
            MAPKEY ('K', kNoCommand);
            MAPKEY ('U', kUninstall);
            MAPKEY ('L', kNoCommand);
            MAPKEY ('N', kName);
            MAPKEY ('J', kNoCommand);
            MAPKEY ('B', kNoCommand);
            MAPKEY ('H', kHistory);

#ifdef CTRLFIRE
            MAPKEY (CTRL('y'), kNoCommand);
            MAPKEY (CTRL('k'), kKick);
            MAPKEY (CTRL('u'), kNoCommand);
            MAPKEY (CTRL('l'), kNoCommand);
            MAPKEY (CTRL('n'), kNoCommand);
            MAPKEY (CTRL('j'), kNoCommand);
            MAPKEY (CTRL('b'), kNoCommand);
            MAPKEY (CTRL('h'), kNoCommand);
#endif
        }
        MAPKEY ('>', kMoveDown);
        MAPKEY ('<', kMoveUp);
    }
}


void
shInterface::initializeCommands () 
{
    // TODO: customizable command config file


    int i;
    for (i = 0; i < KEY_MAX; i++) {
        MAPKEY (i, kNoCommand);
    }

    MAPKEY ('?', kHelp);
    MAPKEY ('.', kRest);
    MAPKEY (27, kEscape);
    MAPKEY (' ', kSpace);

    MAPKEY ('5', kGlide);
    crazyIvan (0);
#if 0
    if (!(has_key (KEY_UP) &&
          has_key (KEY_RIGHT) &&
          has_key (KEY_DOWN) &&
          has_key (KEY_LEFT) 
          /*&&
                  has_key (KEY_A1) &&
          has_key (KEY_A3) &&
          has_key (KEY_C1) &&
          has_key (KEY_C3))
          */))
    {
        endwin ();
        fprintf (stderr, 
                 "Sorry, I can't find movement keys in your terminal!\n");
        exitZapm (-1);
    }

    MAPKEY (KEY_UP, kMoveN);
    MAPKEY (KEY_RIGHT, kMoveE);
    MAPKEY (KEY_DOWN, kMoveS);
    MAPKEY (KEY_LEFT, kMoveW);

    MAPKEY (KEY_A1, kMoveNW);
    MAPKEY (KEY_A3, kMoveNE);
    MAPKEY (KEY_C1, kMoveSW);
    MAPKEY (KEY_C3, kMoveSE);
#endif

    MAPKEY ('@', kToggleAutopickup);
    MAPKEY (',', kPickup);
    MAPKEY (':', kLookHere);
    MAPKEY ('/', kLookThere);


    MAPKEY ('[', kShowArmor);
    MAPKEY ('+', kShowImplants);
    MAPKEY (')', kShowWeapons);
    

/* It's best to follow the familiar key bindings of Nethack and other 
   roguelikes and to map frequently used key to the left side of the 
   keyboard.  For example, this is why mutant powers are bound to the 
   'Z' key instead of the 'm' key. 
*/

    MAPKEY ('a', kUse);
    MAPKEY ('c', kClose);
    MAPKEY ('d', kDrop);
    MAPKEY ('f', kFireWeapon);
    MAPKEY ('g', kGlide);

    MAPKEY ('i', kListInventory);

    MAPKEY ('m', kMutantPower);
    MAPKEY ('o', kOpen);
    MAPKEY ('p', kPay);
    MAPKEY ('q', kQuaff);
    MAPKEY ('s', kSearch);
    MAPKEY ('t', kThrow);
    MAPKEY ('w', kWield);
    MAPKEY ('x', kExecute);
    MAPKEY ('z', kZapRayGun);

    MAPKEY ('A', kAdjust) 
    MAPKEY ('D', kDropMany);
    MAPKEY ('E', kEditSkills);
    //MAPKEY ('H', kHistory);
    MAPKEY ('I', kInstall);
    MAPKEY ('K', kKick);
    MAPKEY ('N', kName);
    MAPKEY ('O', kEditOptions);
    MAPKEY ('R', kUninstall);
    MAPKEY ('S', kSaveGame);
    MAPKEY ('T', kTakeOff);
    MAPKEY ('U', kUninstall);
    MAPKEY ('V', kVersion);
    MAPKEY ('W', kWear);
    MAPKEY ('Z', kMutantPower);

    MAPKEY (CTRL('N'), kName);
    MAPKEY (CTRL('P'), kHistory);
    MAPKEY (CTRL('Q'), kQuit);
    MAPKEY (CTRL('D'), kKick);

    /*debug commands */
    MAPKEY ('`', kGodMode);

    for (i = 0; i < kMaxCommand; i++)
        mCommandHelp[i] = "";

    mCommandHelp[kAdjust] = "Adjust inventory letters";
    mCommandHelp[kClose] = "Close a door";
    mCommandHelp[kDrop] = "Drop an item";
    mCommandHelp[kDropMany] = "Drop several items";
    mCommandHelp[kEditOptions] = "Set options";
    mCommandHelp[kEditSkills] = "Edit skills";
    mCommandHelp[kExecute] = "Execute a floppy disk program";
    mCommandHelp[kFireWeapon] = "Fire your weapon";
    mCommandHelp[kGlide] = "Move until something interesting is found";
    mCommandHelp[kHelp] = "Help menu";
    mCommandHelp[kHistory] = "Show console message history";
    mCommandHelp[kInstall] = "Install a bionic implant";
    mCommandHelp[kKick] = "Kick a door, monster, or object";
    mCommandHelp[kListInventory] = "Inventory List";
    mCommandHelp[kLookHere] = "Look at what is here";
    mCommandHelp[kLookThere] = "Look at a feature or monster";

    mCommandHelp[kMoveDown] = "Climb down stairs";
    mCommandHelp[kMoveUp] = "Climb up stairs";
    mCommandHelp[kMutantPower] = "Zap a mutant power";
    mCommandHelp[kName] = "Name an object or class of objects";
    mCommandHelp[kOpen] = "Open a door";
    mCommandHelp[kPay] = "Pay for items in a store";
    mCommandHelp[kPickup] = "Pick up items from the floor";
    mCommandHelp[kQuaff] = "Quaff from a canister or vat";
    mCommandHelp[kRest] = "Rest for one second";
    mCommandHelp[kSaveGame] = "Save your game (and exit Zapm)";
    mCommandHelp[kSearch] = "Search for traps and secret doors";
    mCommandHelp[kShowArmor] = "List worn armor";
    mCommandHelp[kShowImplants] = "List installed bionic implants";
    mCommandHelp[kShowWeapons] = "Show wielded weapon";
    mCommandHelp[kTakeOff] = "Take off armor";
    mCommandHelp[kThrow] = "Throw an item";
    mCommandHelp[kToggleAutopickup] = "Toggle Autopickup option";
    mCommandHelp[kUninstall] = "Remove / Uninstall a bionic implant";
    mCommandHelp[kUse] = "Activate or apply a tool";
    mCommandHelp[kVersion] = "Version and Copyright information";
    mCommandHelp[kWear] = "Wear armor";
    mCommandHelp[kWield] = "Wield a weapon";
    mCommandHelp[kQuit] = "Quit the game (without saving)";
    mCommandHelp[kZapRayGun] = "Zap a ray gun";
        
    mCommandHelp[kGodMode] = "God Mode Commands";
}

#undef MAPKEY



shInterface::Command
shInterface::getCommand ()
{
    int res;
    
    mLogSCount = 0;
    cursorOnHero ();
    res = wgetch (mMainWin);
    pageLog ();
    return mKey2Cmd[res];
}


void
shInterface::pauseXY (int x, int y, int ms /* = 0 */)
{
    int ch;
#ifndef DJGPP
    const 
#endif
    char more[] = "  --More--";
        
    if (ms) {
        drawScreen ();
    } else {
        char morebuf[16];
        strncpy (morebuf, more, sizeof(morebuf));
		waddstr (mLogWin, morebuf);
        drawLog ();
    }
    cursorOnXY (x, y);
    if (ms) {
#ifdef _WIN32
        Sleep (ms);
#else
        usleep (ms * 1000);
#endif
    } else {
        do {
            ch = getChar ();
        } while (' ' != ch && 13 != ch && 27 != ch);
    }
    Level->drawSq (x, y);
    if (!ms) {
        if (Flags.mFadeLog) {
            int x, y;
            getyx (mLogWin, y, x);
            mvwaddstr (mLogWin, y, x - strlen (more), "          ");
            wmove (mLogWin, y, x);
//        drawLog ();
        }
        mLogSCount = 0;
        pageLog ();
    }
}


/*maxradius is specified in squares
  returns: 1 on success, 0 on abort
*/
int
shInterface::getSquare (const char *prompt, int *x, int *y, int maxradius, 
                        int instant)
{
    int key;
    int tx, ty;
    int step = 1;

    if (maxradius <= 0) maxradius = 99;
    if (prompt) p (prompt);
    
    if (-1 == *x || -1 == *y) {
        *x = Hero.mX;
        *y = Hero.mY;
    }
    while (1) {
        wmove (mMainWin, *y, *x);
        wrefresh (mMainWin);
        tx = *x; ty = *y;
        key = wgetch (mMainWin);
        switch (mKey2Cmd[key]) {
        case kMoveSW: tx = *x - step; ty = *y + step; break;
        case kMoveS:                  ty = *y + step; break;
        case kMoveSE: tx = *x + step; ty = *y + step; break;
        case kMoveW:  tx = *x - step;                 break;
        case kMoveE:  tx = *x + step;                 break;
        case kMoveNW: tx = *x - step; ty = *y - step; break;
        case kMoveN:                  ty = *y - step; break;
        case kMoveNE: tx = *x + step; ty = *y - step; break;
        case kGlide: case kRest: 
            return 1;
        case kEscape:
        case kSpace:
            return 0;
        default:
            I->p ("Use the movement keys to position the cursor.");
            I->p ("Select the location with the \".\" key.  "
                  "Press Escape to abort.");
            continue;
        }
        if (Level->isInBounds (tx, ty) && 
            distance (&Hero, tx, ty) <= 5 * maxradius)
        {
            *x = tx; 
            *y = ty;
        }       
        if (instant)
            return 1;
    }
}


shDirection
shInterface::getDirection (int *x, int *y, int *z, int silent)
{
    int res;
    if (!silent) p ("In what direction? ");
    drawLog ();
    cursorOnHero ();
    res = wgetch (mMainWin);
    if (z) *z = 0;
    switch (mKey2Cmd[res]) {
    case kMoveSW: if (x) --*x; if (y) ++*y; pageLog (); return kSouthWest;
    case kMoveS:  if (y) ++*y; return kSouth;
    case kMoveSE: if (x) ++*x; if (y) ++*y; pageLog (); return kSouthEast;
    case kMoveW:  if (x) --*x; return kWest;
    case kRest: case kGlide: pageLog (); return kOrigin;
    case kMoveE:  if (x) ++*x; pageLog (); return kEast;
    case kMoveNW: if (x) --*x; if (y) --*y; pageLog (); return kNorthWest;
    case kMoveN:  if (y) --*y; pageLog (); return kNorth;
    case kMoveNE: if (x) ++*x; if (y) --*y; pageLog (); return kNorthEast;
    case kMoveUp: if (z) --*z; pageLog (); return kUp;
    case kMoveDown: if (z) ++*z; pageLog (); return kDown;
    default:
        nevermind ();
        return kNoDirection;
    }

}


int
shInterface::getChar (WINDOW *window /* = NULL */)
{
    if (!window) {
        window = mMainWin;
    }
    return  wgetch (window);
}


/* MODIFIES: reads a string from the keyboard 
   RETURNS: number of characters read, 0 on error */

int
shInterface::getStr (char *buf, int len,
                     const char *prompt,
                     const char *dflt /* = NULL */ )
{
    /* can't use wgetnstr b/c it won't pass us ^C */
    
    char msg[80];
    int pos = 0;
    int savehistidx = mHistoryIdx;

    snprintf (msg, 80, "%s ", prompt);    
    msg[79] = 0;
    p (msg);
    buf[0] = 0;
    //mNoNewline = 1;

    if (dflt) {
        strncpy (buf, dflt, len);
        buf[len-1] = 0;
        waddstr (mLogWin, buf);
        pos = strlen (buf);
    }

    wtimeout (mLogWin, 100);

    while (1) {
        drawLog ();
        int c = getChar (mLogWin);
        if (isprint (c)) {
            if (pos >= len - 1) {
                continue;
            } 
            buf[pos++] = c;
            waddch (mLogWin, c);
        } else if ('\n' == c || '\r' == c) {
            break;
        } else if (8 == c && pos) {
            pos--;
            waddch (mLogWin, 8);
#ifdef _WIN32
            waddch (mLogWin, ' ');
            waddch (mLogWin, 8);
#endif
        } else if (127 == c && pos) {
        backspace:
            pos--;
            waddch (mLogWin, 8);
            waddch (mLogWin, ' ');
            waddch (mLogWin, 8);
        } else if (27 == c) {
            /* FIXME */
            
            debug ("escape sequence?");
            int c2 = getChar (mLogWin);
            
            if ('[' != c2) {
                debug ("just plain escape, bail out");
                pos = 0;
                break;
            }
            
            c2 = getChar (mLogWin);
            if ('D' == c2 && pos) {
                goto backspace;
            }
            
        } else {
            debug ("unhandled char %d", c);
        }
    }
    
    wtimeout (mLogWin, -1);
    //waddch (mLogWin,'\n');

    buf[pos] = 0;
    snprintf (msg, 80, "%s %s", prompt, buf);
    msg[79] = 0;
    strcpy (&mLogHistory[savehistidx*80], msg);

    return pos;
}


int
shInterface::yn (const char *prompt, ...)
{
    char msg[80];
    int res;

    snprintf (msg, 80, "%s [y/n]", prompt);    
    msg[79] = 0;

    va_list ap;
    va_start (ap, prompt);
    vp (msg, ap);
    va_end (ap);
    
    while (1) {
        res = getChar (mLogWin);
        if ('y' == res || 'Y' == res) {
            return 1;
        } else if ('n' == res || 'N' == res) {
            return 0;
        }
    }
}


void
shInterface::dirty ()
{
    mDirty = 1;
}


void
shInterface::pause ()
{
    doMorePrompt ();
    //mPause = 1;
}


void
shInterface::smallPause ()
{
    mPause = 2;
}


int
shInterface::p (const char *format, ...)
{
    int res;
    va_list ap;
    va_start (ap, format);
    res = vp (format, ap);
    va_end (ap);
    return res;
}
 

void
shInterface::doMorePrompt ()
{
    int ch;
#ifndef _WIN32
    const 
#endif
    char more[] = "  --More--";
        
    waddstr (mLogWin, more);
    drawLog ();
    do {
        ch = getChar (mLogWin);
    } while (' ' != ch && 13 != ch && 27 != ch);

    if (2 != mPause) {
        mLogSCount = 0;
        pageLog ();
    }
    mPause = 0;
    if (Flags.mFadeLog) {
        int x, y;
        getyx (mLogWin, y, x);
        mvwaddstr (mLogWin, y, x - strlen (more), "          ");
        drawLog ();
    }
    return;
}


int
shInterface::vp (const char *format, va_list ap)
{
    const int buflen = 80;
    char strbuf[buflen];
    int res;

    if (6 == ++mLogSCount || mPause) {
        doMorePrompt ();
        mLogSCount = 1;
    }
   
    res = vsnprintf (&strbuf[0], buflen, format, ap);
    if (' ' != strbuf[1] || '-' != strbuf[2]) {
        strbuf[0] = toupper (strbuf[0]);
    }
    if ('"' == strbuf[0]) {
        strbuf[1] = toupper (strbuf[1]);
    }
    strbuf[buflen-1] = 0;
    strcpy (&mLogHistory[mHistoryIdx*80], strbuf);
    debug ("%s", strbuf);
    mHistoryIdx = (mHistoryIdx + 1) % HISTORY_ROWS;
    if (0 == mHistoryIdx) {
        mHistoryWrapped = 1;
    }
    if (mLogRow != 0 && !mNoNewline) {
        waddch (mLogWin, '\n');
    }
    mNoNewline = 0;
    if (mLogRow < mLogSize) {
        mLogRow++;
    }

    wattrset (mLogWin, ColorMap[mColor]);
    waddstr (mLogWin, strbuf);
    wattrset (mLogWin, A_NORMAL);
    if (mDirty) { 
        drawScreen ();
    } else {
        drawLog ();
    }
    return res;
};


int
shInterface::diag (const char *format, ...)
{
    int res;
    va_list ap;

    mColor = kMagenta;
    va_start (ap, format);
#if 0
    res = vp (format, ap);
#else
    char dbgbuf[100];
    res = vsnprintf (dbgbuf, 100, format, ap);
    if (mDiagWin) {
        wattrset (mDiagWin, ColorMap[kCyan]);
        waddstr (mDiagWin, dbgbuf);
        waddch (mDiagWin, '\n');
        touchwin (mDiagWin);
        wnoutrefresh (mDiagWin);
        doupdate ();
    }
    debug ("%s", dbgbuf);
#endif
    va_end (ap);
    mColor = kGray;
    return res;



}


void
shInterface::pageLog () 
{
    if (Flags.mFadeLog) {
        int i;
        int offs;
        int x, y;
        getyx (mLogWin, y, x);

        wattrset (mLogWin, ColorMap[kBlue]);

        for (i = 0; i < mLogRow; i++) {
            offs = (HISTORY_ROWS + mHistoryIdx - (mLogRow-i)) % HISTORY_ROWS;
            mvwaddstr (mLogWin, i, 0, &mLogHistory[offs*80]);
        }

        wattrset (mLogWin, A_NORMAL);
        wmove (mLogWin, y, x);
        drawLog ();
    } else {
        werase (mLogWin);
        wmove (mLogWin, 0, 0);
        mLogRow = 0;
        mLogSCount = 0;
        drawLog ();
    }
}


void
shInterface::showHistory ()
{
    WINDOW *win;
    PANEL *panel;
    int i = 0;
    int n = 0;

    win = newwin (25, 80, 0, 0);
    if (!win) {
        I->p ("Couldn't open history window");
        return;
    }
    panel = new_panel (win);

    mvwaddstr (win, 0, 0, "Console history:");
    if (mHistoryWrapped) {
        i = (HISTORY_ROWS + mHistoryIdx - 
             (HISTORY_ROWS < 23 ? HISTORY_ROWS : 23)) % HISTORY_ROWS;
    } 
    do {
        mvwaddstr (win, n + 1, 0, &mLogHistory[i*80]);
        i = (i + 1) % HISTORY_ROWS;
        ++n;
    } while (i != mHistoryIdx);
    mvwaddstr (win, n + 1, 0, "--End--");

    update_panels ();
    doupdate ();
    I->getChar (win);
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
}


void
initGlyph (shGlyph *g, char sym, shColor color, int extra)
{
    g->mChar = sym | ColorMap[color];
    g->mAttr = 0;
    g->mForeground = color;
}



struct shMenuChoice
{
    char mLetter;
    int mCount;           /* count available. -1 indicates this is a header */
    int mSelected;        /* count currently selected */
    char mText[256];
    const void *mValue;   /* value of the object returned later if this choice
                             is selected. */
    
    shMenuChoice (char letter, const char *text, const void *value, int count, 
                  int selected = 0)
    {
        mLetter = letter;
        strncpy (mText, text, 255); mText[255] = 0;
        mValue = value;
        mSelected = selected;
        mCount = count;
    }   
};


shMenu::shMenu (const char *prompt, int flags)
    : mChoices ()
{
    strncpy (mPrompt, prompt, 79); mPrompt[79] = 0;
    mFlags = flags;
    //mFirstReady = 0;
    mResultIterator = 0;
    mPanel = NULL;
    mHeight = 0;
    mOffset = 0;
    mDone = 0;
    getmaxyx(stdscr, mHeight, mWidth);

    if (!(mFlags & kNoPick)) {
        /* KLUDGE: avoid repeating a letter on the same page */
        mHeight = mini (52, mHeight); 
    }
    mObjTypeHack = kMaxObjectType;
}


shMenu::~shMenu ()
{
    for (int i = 0; i < mChoices.count (); i++) {
        delete mChoices.get (i);
    }
}


void
shMenu::addItem (char letter, const char *text, const void *value, 
                 int count /* = 1 */, int selected /* = 0 */)
{
    shMenuChoice *c;
    
    if (0 == letter) {
        letter = ' ';
    }
    if (mFlags & kCategorizeObjects && value) {
        shObjectType t = ((shObject *) value) -> mIlk->mType;
        if (t != mObjTypeHack) {
            switch (t) {
            case kMoney: addHeader ("Money"); break;
            case kImplant: addHeader ("Bionic Implants"); break;
            case kFloppyDisk: addHeader ("Floppy Disks"); break;
            case kCanister: addHeader ("Canisters"); break;
            case kTool: addHeader ("Tools"); break;
            case kArmor: addHeader ("Armor"); break;
            case kWeapon: addHeader ("Weapons"); break;
            case kRayGun: addHeader ("Ray Guns"); break;
            case kProjectile: addHeader ("Ammunition"); break;
            case kFood: addHeader ("Comestibles"); break;
            case kDevice: addHeader ("Devices"); break;
            case kEnergyCell: addHeader ("Energy Cells"); break;
            default: addHeader ("------");
            }
            mObjTypeHack = t;
        }
    }

    c = new shMenuChoice (letter, text, value, count, selected);
    mChoices.add (c);
}


void
shMenu::addHeader (const char *text)
{
    /* prints out the header */
    addItem (-1, text, NULL, -1);
}


/* call this repeatedly to store the selected results into value and count. 
  RETURNS: 0 if there are no (more) results, 1 o/w 
*/

int
shMenu::getResult (const void **value, int *count /* = NULL */ )
{
    shMenuChoice *choice;
    int n;

    if (!mDone)
        accumulateResults ();

    while (mResultIterator < mChoices.count ()) {
        choice = mChoices.get (mResultIterator++);
        if ((n = choice->mSelected)) {
            *value = choice->mValue;
            if (NULL != count) {
                *count = n;
            }
            return 1;
        }
    } 
    *value = NULL;
    return 0;
};


void
shMenu::select (int i1, int i2,   /* mChoices[i1..i2) */ 
                int action,       /* 0 unselect, 1 select, 2 toggle */
                shObjectType t)   /* = kUninitialized */
{
    int i;
    if (!(mFlags & kMultiPick)) {
        return;
    }

    for (i = i1; i < i2; i++) {
        shMenuChoice *choice = mChoices.get (i);
        if (choice->mCount < 0) {
            continue;
        }
        if (t && 
            mFlags & kCategorizeObjects && 
            choice->mValue &&
            t != ((shObject *) choice->mValue) -> mIlk->mType) 
        {
            continue;
        }
        if (0 == action) {
            choice->mSelected = 0;
        } else if (1 == action) {
            choice->mSelected = choice->mCount;
        } else if (2 == action) {
            if (!choice->mSelected)
                choice->mSelected = choice->mCount;
            else 
                choice->mSelected = 0;
        }
    }
}


void
shMenu::accumulateResults ()
{
    int key;
    int i;
    int nummode = 0;
    int num = 0; /* stays zero unless nummode is set */
    const int maxcount = 9999999;
    WINDOW *win;
    PANEL *panel;

    while (1) {
        /* Display a menu page.  First, calculate the width needed:  */
        int width = maxi (10, strlen (mPrompt) + 2);
        int n = mini (mOffset + mHeight - 2, mChoices.count ());
        for (i = mOffset; i < n; i++) {
            width = maxi (width, strlen (mChoices.get (i) ->mText));
        }
        width += 10;
        width = mini (mWidth, width);
        win = newwin (mHeight, width, 0, maxi (0, I->mXMax - width));
        if (!win) {
            I->debug ("Unable to create window (%d, %d, %d, %d)", 
                      mHeight, mWidth, 0, maxi (0, I->mXMax - width));
            I->p ("Uh oh!  Couldn't create window!!");
            mDone = 1;
            return;
            // endwin ();
            // fprintf (stderr, "Unable to create window.\n");
            // exitZapm (-1);
        }
        panel = new_panel (win);
        wattrset (win, A_BOLD);
        mvwaddnstr (win, 0, 1, mPrompt, width);
        wattrset (win, A_NORMAL);
  //    mvwaddstr (win, mHeight + 2, 0, mFlags & kMultiPick ? "multipick" : "singlepick");

        for (i = mOffset; i < n; i++) {
            char buf[100];
            shMenuChoice *item = mChoices.get (i);

            if (-1 == item->mLetter) { /* this is a header entry */
                wattrset (win, A_REVERSE);
                snprintf (buf, 100, " %s ", item->mText);
            } else if (mFlags & kNoPick) {
                if (' ' == item->mLetter) {
                    snprintf (buf, 100, "%s", item->mText);
                } else {
                    snprintf (buf, 100, "%c - %s", item->mLetter, item->mText);
                }
            } else {
                if (' ' == item->mLetter) {
                    snprintf (buf, 100, "        %s", item->mText);
                } else {
                    snprintf (buf, 100, "(%c) %c - %s",
                              0 == item->mSelected ? ' ' :
                              item->mCount == item->mSelected ? 'X' : '#',
                              item->mLetter, item->mText);
                }
            }
            mvwaddnstr (win, 1 + i - mOffset, 1, buf, width);
            wattrset (win, A_NORMAL);
        }
        if (i == mChoices.count ()) {
            mvwaddnstr (win, 1 + i - mOffset, 1, "--End--", width);
        } else {
            mvwaddnstr (win, 1 + i - mOffset, 1, "--More--", width);
        }
        update_panels (); 
        doupdate();

        while (1) {
            key = I->getChar (win);

            if (' ' == key || 13 == key) {
                /* page through */
                mOffset = n;
                break;
            } else if (27 == key) {
                /* done */
                mOffset = mChoices.count ();
                break;
            } else if ('>' == key) {
                /* page down */
                if (n < mChoices.count ()) {
                    mOffset = n;
                }
                goto nextpage;
            } else if ('<' == key) {
                /* previous page */
                mOffset -= (mHeight - 2);
                mOffset = maxi (0, mOffset);
                goto nextpage;
            } else if ('^' == key) {
                mOffset = 0;
                goto nextpage;
            } else if (mFlags & kNoPick) {
                continue;
            } else if ('@' == key) { 
                select (0, mChoices.count(), 2); /* toggle all */
                goto nextpage;
            } else if ('-' == key) {
                select (0, mChoices.count(), 0); /* deselect all */
                goto nextpage;
            } else if ('.' == key) {
                select (0, mChoices.count(), 1); /* select all */
                goto nextpage;
            } else if ('~' == key) {
                select (mOffset, n, 2); /* toggle page */
                goto nextpage;
            } else if ('\\' == key) {
                select (mOffset, n, 0); /* deselect page */
                goto nextpage;
            } else if (',' == key) {
                select (mOffset, n, 1); /* select page */
                goto nextpage;
            } else if ('$' == key) { 
                select (0, mChoices.count(), 2, kMoney); /* toggle all gold */
                goto nextpage;
            } else if ('+' == key) { 
                select (0, mChoices.count(), 2, kImplant); /* toggle all implants */
                goto nextpage;
            } else if ('?' == key) { 
                select (0, mChoices.count(), 2, kFloppyDisk); /* toggle all floppies */
                goto nextpage;
            } else if ('!' == key) { 
                select (0, mChoices.count(), 2, kCanister); /* toggle all cans */
                goto nextpage;
            } else if ('(' == key) { 
                select (0, mChoices.count(), 2, kTool); /* toggle all tools */
                goto nextpage;
            } else if ('[' == key) { 
                select (0, mChoices.count(), 2, kArmor); /* toggle all armor */
                goto nextpage;
            } else if (')' == key) { 
                select (0, mChoices.count(), 2, kWeapon); /* toggle all weapons */
                goto nextpage;
            } else if ('=' == key) { 
                select (0, mChoices.count(), 2, kProjectile); /* toggle all ammo */
                goto nextpage;
            } else if ('/' == key) { 
                select (0, mChoices.count(), 2, kRayGun); /* toggle all ray guns */
                goto nextpage;
            } else if (key >= '0' && key <= '9') {
                if (mFlags & kCountAllowed) {
                    nummode = 1;
                    num = num * 10 + key - '0';
                    if (num >= maxcount) { 
                        num = maxcount;
                    }
                }
                else {
                    /* TODO: tell user count not allowed for this command. */
                }
            } else for (i = mOffset; i < n; i++) {
                shMenuChoice *item = mChoices.get (i);
                if (item->mLetter == key) {
                    if (nummode) { /* set exact number picked */
                        if (num > item->mCount) {
                            /* TODO: warn user not that many items are available */
                            num = item->mCount;
                        }
                    } else { /* toggle all or none */
                        num = item->mSelected ? 0 : item->mCount;
                    }
                    item->mSelected = num;
                    nummode = 0;
                    num = 0;
                    
                    mvwaddch (win, 1 + i - mOffset, 2, 
                              0 == item->mSelected ? ' ' :
                              item->mCount == item->mSelected ? 'X' : '#'); 
                    wmove (win, 1 + i - mOffset, 2);
                    update_panels ();
                    doupdate();
                    
                    if (!(mFlags & kMultiPick)) {
                        mOffset = -1;
                        goto nextpage;
                    }
                }
            }
        }
nextpage:
        hide_panel (panel);
        del_panel (panel);
        delwin (win);
        update_panels (); 
        I->drawScreen ();
        if (mOffset < 0 || mOffset >= mChoices.count ()) {
            break;
        }
    }
    mDone = 1;
}



void
shInterface::showVersion ()
{

    WINDOW *win;
    PANEL *panel;

    win = newwin (15, 60, 4, 10);
    if (!win) {
        return;
    }
    panel = new_panel (win);


    wattrset (win, ColorMap[kYellow]);
    mvwaddstr (win, 2, 15, "######  ####  #####   ######");
    mvwaddstr (win, 3, 15, "   ##  ##  ## ##  ## ## ## ##");
    mvwaddstr (win, 4, 15, "  ##   ###### #####  ## ## ##");
    mvwaddstr (win, 5, 15, " ##    ##  ## ##     ## ## ##");
    mvwaddstr (win, 6, 15, "###### ##  ## ##     ## ## ##");

    wattrset (win, ColorMap[kBrightRed]);

    mvwaddstr (win, 8, 12, "ZAPM version " ZAPM_VERSION " ");
    wattrset (win, A_NORMAL);
    mvwaddstr (win, 10, 12,
               "Copyright (C) 2002-2010 Cyrus Dolph.");
    mvwaddstr (win, 11, 12, "All rights reserved.");
    mvwaddstr (win, 12, 12, "http://www.zapm.org");

    mvwaddstr (win, 14, 12, "--End--");

    update_panels ();
    doupdate();
    I->getChar (win);
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
//    I->drawScreen ();   
}
