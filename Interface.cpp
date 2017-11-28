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
#include <unistd.h>


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

    initscr ();

    if (!has_colors ()) {
        endwin ();
        fprintf (stderr, "Sorry, color support is required.\n");
        exit (-1);
    }
    start_color ();

    for (i = 1; i < COLOR_PAIRS; i++) {
        init_pair (i, i, COLOR_BLACK);
    }


#ifdef DJGPP
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
    notimeout (mLogWin, TRUE);
    mLogSize = 5;
    mLogRow = 0;
    mHistoryIdx = 0;
    mHistoryWrapped = 0;
    mDirty = 0;
    mNoNewline = 0;

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
        exit (-1);
    }
    setlinebuf (mDbgFile);
#endif

    debug ("%d color pairs are available.", COLOR_PAIRS);
    return;

toosmall:
    endwin ();
    fprintf (stderr, "Sorry, but a terminal with dimensions of at least 80x25"
             " is required.\n");
    exit (-1);
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

#ifdef CTRLFIRE
            MAPKEY (CTRL('n'), kFireNW);
            MAPKEY (CTRL('j'), kFireN);
            MAPKEY (CTRL('b'), kFireNE);
            MAPKEY (CTRL('h'), kFireE);
            MAPKEY (CTRL('y'), kFireSE);
            MAPKEY (CTRL('k'), kFireS);
            MAPKEY (CTRL('u'), kFireSW);
            MAPKEY (CTRL('l'), kFireW);
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

#ifdef CTRLFIRE
            MAPKEY (CTRL('n'), kNoCommand);
            MAPKEY (CTRL('j'), kNoCommand);
            MAPKEY (CTRL('b'), kNoCommand);
            MAPKEY (CTRL('h'), kNoCommand);
            MAPKEY (CTRL('y'), kNoCommand);
            MAPKEY (CTRL('u'), kNoCommand);
            MAPKEY (CTRL('k'), kNoCommand);
            MAPKEY (CTRL('l'), kNoCommand);
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
        exit (-1);
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

    MAPKEY (',', kPickup);
    MAPKEY (':', kLookHere);
    MAPKEY ('/', kLookThere);

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
    MAPKEY ('H', kHistory);
    MAPKEY ('I', kInstall);
    MAPKEY ('K', kKick);
    MAPKEY ('N', kName);
    MAPKEY ('O', kEditOptions);
    MAPKEY ('S', kSaveGame);
    MAPKEY ('T', kTakeOff);
    MAPKEY ('U', kUninstall);
    MAPKEY ('V', kVersion);
    MAPKEY ('W', kWear);
    MAPKEY ('Z', kMutantPower);


    MAPKEY (CTRL('P'), kHistory);
    MAPKEY (CTRL('Q'), kQuit);
    MAPKEY (CTRL('D'), kKick);

    /*debug commands */
    MAPKEY ('`', kGodMode);

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
/*
    mCommandHelp[kMoveN] = "";
    mCommandHelp[kMoveNE] = "";
    mCommandHelp[kMoveE] = "";
    mCommandHelp[kMoveSE] = "";
    mCommandHelp[kMoveS] = "";
    mCommandHelp[kMoveSW] = "";
    mCommandHelp[kMoveW] = "";
    mCommandHelp[kMoveNW] = "";
*/

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
    mCommandHelp[kTakeOff] = "Take off armor";
    mCommandHelp[kThrow] = "Throw an item";
    mCommandHelp[kUninstall] = "Uninstall a bionic implant";
    mCommandHelp[kUse] = "Activate or apply a tool";
    mCommandHelp[kVersion] = "Version and Copyright information";
    mCommandHelp[kWear] = "Wear armor";
    mCommandHelp[kWield] = "Wield a weapon";
    mCommandHelp[kQuit] = "Quit the game";
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
shInterface::pauseXY (int x, int y)
{
    int ch;
#ifndef DJGPP
    const 
#endif
    char more[] = "  --More--";
        
    waddstr (mLogWin, more);
    drawLog ();
    cursorOnXY (x, y);
    do {
        ch = getChar ();
    } while (' ' != ch && 13 != ch && 27 != ch);
    Level->drawSq (x, y);
    pageLog ();
}


/*maxradius is specified in squares
  returns: 1 on success, 0 on abort
*/
int
shInterface::getSquare (char *prompt, int *x, int *y, int maxradius)
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
            I->p ("Select the location with the \".\" key.  Press Escape to abort.");
        }
        if (Level->isInBounds (tx, ty) && 
            distance (&Hero, tx, ty) <= 5 * maxradius)
        {
            *x = tx; 
            *y = ty;
        }       
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
shInterface::getChar ()
{
    int c = wgetch (mMainWin);
//    debug ("wgetch() -> %d", c);
    return c;
}


/* MODIFIES: reads a string from the keyboard 
   RETURNS: number of characters read, 0 on error */

int
shInterface::getStr (char *buf, int len,
                     const char *prompt, ...)
{
    /* can't use wgetnstr b/c it won't pass us ^C */
    
    char msg[80];
    int pos = 0;

    snprintf (msg, 80, "%s ", prompt);    
    msg[79] = 0;
    p (msg);
    buf[0] = 0;
    mNoNewline = 1;

    wtimeout (mLogWin, 100);

    while (1) {
        drawLog ();
        int c = getChar ();
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
        } else if (127 == c && pos) {
        backspace:
            pos--;
            waddch (mLogWin, 8);
            waddch (mLogWin, ' ');
            waddch (mLogWin, 8);
        } else if (27 == c) {
            /* FIXME */
            
            debug ("escape sequence?");
            int c2 = getChar ();
            
            if ('[' != c2) {
                debug ("just plain escape, bail out");
                pos = 0;
                break;
            }
            
            c2 = getChar ();
            if ('D' == c2 && pos) {
                goto backspace;
            }
            
        } else {
            debug ("unhandled char %d", c);
        }
    }
    
    wtimeout (mLogWin, -1);

    buf[pos] = 0;
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
        res = getChar ();
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
    mPause = 1;
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
#ifndef DJGPP
    const 
#endif
    char more[] = "  --More--";
        
    waddstr (mLogWin, more);
    drawLog ();
    do {
        ch = getChar ();
    } while (' ' != ch && 13 != ch && 27 != ch);
    if (2 != mPause) {
        mLogSCount = 0;
        pageLog ();
    }
    mPause = 0;
#if 0 /* this makes it hard to tell which lines came before the --More-- */
    int x, y;
    getyx (mLogWin, y, x);
    wmove (mLogWin, y, x - strlen (more));
#endif
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
    if (' ' != strbuf[1] && '-' != strbuf[2]) {
        strbuf[0] = toupper (strbuf[0]);
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
    debug ("%s", dbgbuf);
#endif
    va_end (ap);
    mColor = kGray;
    return res;



}


void
shInterface::pageLog () 
{
    werase (mLogWin);
    wmove (mLogWin, 0, 0);
    mLogRow = 0;
    mLogSCount = 0;
    drawLog ();
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
    I->getChar ();
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
    int mCount;     /* count available. -1 indicates this is a header */
    int mSelected;  /* count currently selected */
    char mText[70];
    void *mValue;   /* value of the object returned later if this choice
                       is selected. */
    
    shMenuChoice (char letter, char *text, void *value, int count)
    {
        mLetter = letter;
        strncpy (mText, text, 69); mText[69] = 0;
        mValue = value;
        mSelected = 0;
        mCount = count;
    }   
};


shMenu::shMenu (char *prompt, int flags)
    : mChoices ()
{
    strncpy (mPrompt, prompt, 79); mPrompt[79] = 0;
    mFlags = flags;
    mFirstReady = 0;
    mResultIterator = 0;
    mPanel = NULL;
    mWidth = 20;
    mHeight = 0;
    mObjTypeHack = kMaxObjectType;
}


shMenu::~shMenu ()
{

}


void
shMenu::addItem (char letter, char *text, void *value, int count /* = 1 */)
{
    shMenuChoice *c;
    int w;
    
    if (++mHeight > 22) {
        accumulateResults (1);
        mWidth = 20;
        mHeight = 1;
    }    
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
            default: addHeader ("------");
            }
            mObjTypeHack = t;
        }
    }

    c = new shMenuChoice (letter, text, value, count);
    mChoices.add (c);
    w = strlen (text);
    if (w > 70) {
        w = 70;
    }
    if (w > mWidth) {
        mWidth = w;
    }
}


void
shMenu::addHeader (char *text)
{
    /* prints out the header */
    addItem (-1, text, NULL, -1);
}


/* call this repeatedly to store the selected results into value and count. 
  RETURNS: 0 if there are no (more) results, 1 o/w 
*/

int
shMenu::getResult (void **value, int *count /* = NULL */ )
{
    shMenuChoice *choice;
    int n;

    if (0 == mResultIterator) {
        accumulateResults (0);
    }

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



/* called when we've filled up a whole screen full of results, or by the
   done() method.  waits for user to input selection, and records the
   results
*/

void
shMenu::accumulateResults (int more /* = 0 */ )
{
    int key;
    int i;
    int nummode = 0;
    int num = 0; /* stays zero unless nummode is set */
    const int maxcount = 9999999;
    WINDOW *win;
    PANEL *panel;

    if (I->mPause) {
        I->doMorePrompt ();
    }

    if (-1 == mFirstReady) {
        /* our single pick has already been made */
        return;
    }
    
    mWidth += 10;
    mWidth = mini (79, mWidth); 
    win = newwin (mHeight + 2, mWidth, 0, maxi (0, I->mXMax - mWidth));
    if (!win) {
        //TODO: recover gracefully somehow?
        endwin ();
        fprintf (stderr, "Unable to create window.\n");
        exit (-1);
    }
    panel = new_panel (win);
    wattrset (win, A_BOLD);
    mvwaddstr (win, 0, 1, mPrompt);
    wattrset (win, A_NORMAL);
//    mvwaddstr (win, mHeight + 2, 0, mFlags & kMultiPick ? "multipick" : "singlepick");
    for (i = mFirstReady; i < mChoices.count (); i++) {
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
                snprintf (buf, 100, "( ) %c - %s", 
                          item->mLetter, item->mText);
            }
        }
        mvwaddstr (win, 1 + i - mFirstReady, 1, buf);
        wattrset (win, A_NORMAL);
    }
#ifdef DJGPP
    if (more) {
        mvwaddstr (win, 1 + i - mFirstReady, 1, "--More--");
    } else {
        mvwaddstr (win, 1 + i - mFirstReady, 1, "--End--");
    }
#else
    mvwaddstr (win, 1 + i - mFirstReady, 1, more ? "--More--" : "--End--");
#endif
    update_panels (); 
    doupdate();

    if (mFlags & kNoPick) {
        do {
            key = I->getChar ();
        } while (' ' != key && 13 != key && 27 != key);
        mFirstReady = i;
        goto done;
    }

    while (1) {
        key = I->getChar ();
        if (' ' == key || 13 == key || 27 == key) {
            break;
        }
        if (key >= '0' && key <= '9') {
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
        }
        for (i = mFirstReady; i < mChoices.count (); i++) {
            shMenuChoice *item = mChoices.get (i);
            if (item->mLetter == key) {
                if (nummode) { /* set exact number picked */
                    if (num > item->mCount) {
                        /* TODO: warn user not that many items are available */
                        num = item->mCount;
                    }
                }
                else { /* toggle all or none */
                    num = item->mSelected ? 0 : item->mCount;
                }
                item->mSelected = num;
                nummode = 0;
                num = 0;

                mvwaddch (win, 1 + i - mFirstReady, 2, 
                          0 == item->mSelected ? ' ' :
                          item->mCount == item->mSelected ? 'X' : '#'); 
                wmove (win, 1 + i - mFirstReady, 2);
                update_panels ();
                doupdate();

                if (!(mFlags & kMultiPick)) {
                    mFirstReady = -1;
                    goto done;
                }
            }
        }
    }
    mFirstReady = i; /* next time around, these choices won't be available */
 done:
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
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
               "Copyright (C) 2002-2004 Cyrus Dolph.");
    mvwaddstr (win, 11, 12, "All rights reserved.");
    mvwaddstr (win, 12, 12, "http://www.zapm.org");

    mvwaddstr (win, 14, 12, "--End--");

    update_panels ();
    doupdate();
    I->getChar ();
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
//    I->drawScreen ();   
}
