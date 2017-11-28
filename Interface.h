struct shInterface;

#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdio.h>
#include <curses.h>
#ifndef _PANEL_H
extern "C" {
#include <panel.h>
}
#endif
#include <stdarg.h>
#include "Global.h"
#include "Map.h"
#include "ObjectType.h"

extern int ColorMap[22];

extern int GodMode;

void
initGlyph (shGlyph *g, char sym, shColor color, int extra);


struct shInterface
{
    friend class shMapLevel;
    friend class shMenu;

    enum Command {
        kNoCommand,
        kEscape,
        kSpace,

        kMoveN,
        kMoveNE,
        kMoveE,
        kMoveSE,
        kMoveS,
        kMoveSW,
        kMoveW,
        kMoveNW,
        kMoveDown,
        kMoveUp,

        kGlideN,
        kGlideNE,
        kGlideE,
        kGlideSE,
        kGlideS,
        kGlideSW,
        kGlideW,
        kGlideNW,
        kGlideDown,
        kGlideUp,

        kFireN,
        kFireNE,
        kFireE,
        kFireSE,
        kFireS,
        kFireSW,
        kFireW,
        kFireNW,
        kFireDown,
        kFireUp,

        kAdjust,
        kClose,
        kDrop,
        kDropMany,
        kEditOptions,
        kEditSkills,
        kExecute,
        kFireWeapon,
        kGlide,
        kHelp,
        kHistory,
        kInstall,
        kKick,
        kListInventory,
        kLookHere,
        kLookThere,
        kMutantPower,
        kName,
        kOpen,
        kPay,
        kPickup,
        kQuaff,
        kRest,
        kSaveGame,
        kSearch,
        kShowArmor,
        kShowImplants,
        kShowWeapons,
        kTakeOff,
        kToggleAutopickup,
        kThrow,
        kUninstall,
        kUse,
        kVersion,
        kWear,
        kWield,
        kZapRayGun,
        kQuit,
        
/* debug commands: */
        
        kGodMode,
        
        kMaxCommand
    };

    FILE *mDbgFile;

    static shDirection 
    moveToDirection (Command cmd)
    {
        if (cmd < kMoveN || cmd > kMoveUp) {
            return kNoDirection;
        } else {
            return (shDirection) (cmd - kMoveN);
        }
    }

    shInterface ();
    ~shInterface ();
    
    int getChar (WINDOW *window = NULL);
    int getStr (char *buf, int len, const char *prompt, 
                const char *dflt = NULL);
    int getSquare (const char *prompt, int *x, int *y, int maxradius, 
                   int instant = 0);
    Command getCommand ();
    shDirection getDirection (int *x = NULL, int *y = NULL, int *z = NULL, 
                              int silent = 0);
    shObject *getObjFromInventory ();

    void cursorOnHero ();
    void cursorOnXY (int x, int y);
    void pauseXY (int x, int y, int ms = 0);
    void drawSideWin ();
    void drawScreen ();
    void drawLog ();
    void refreshScreen ();
    
    void dirty ();

    int yn (const char *prompt, ...);
    int p (const char *format, ...);
    int vp (const char *format, va_list ap);
    int diag (const char *format, ...);
    void nevermind () { p ("Never mind."); }
    void pause ();
    void smallPause ();
    void pageLog ();
    void showHistory ();
    void showHelp ();
    void showVersion ();
    void editOptions ();
    void doMorePrompt ();

    void setColor (shColor c) { mColor = c; };

    inline int
    debug (const char *format, ...)
    {
#ifdef SH_DEBUG
        int n;
        va_list ap;
        va_start (ap, format);
        //return vp (format, ap);
        n = vfprintf (mDbgFile, format, ap);
        fputs ("\n", mDbgFile);
        return n + 1;
        va_end (ap);
#else
        return 0;
#endif
    }

    void crazyIvan (int on);

    WINDOW *logWin() { return mLogWin; }

 private:
    //representation:

    int mX0;
    int mY0;
    int mXMax;
    int mYMax;

    WINDOW *mMainWin;
    WINDOW *mSideWin;
    WINDOW *mLogWin;
    WINDOW *mDiagWin; //diagnostics

    int mLogRow;      //first empty row in LogWin
    int mLogSize;     //num rows in LogWin
    int mLogSCount;   //num rows output since last keyboard input

    chtype mSqGlyphs[kMaxTerrainType + 1];
    Command mKey2Cmd[KEY_MAX];
    const char *mCommandHelp[kMaxCommand];
#define HISTORY_ROWS 20
    char mLogHistory[HISTORY_ROWS * 80];
    int mHistoryIdx;
    int mHistoryWrapped;
    int mPause;
    int mDirty;
    int mNoNewline;

    shColor mColor;

    //helper functions:
    void initializeGlyphs ();
    void initializeCommands ();
    
};


struct shMenuChoice;

class shMenu
{
 private:
    char mPrompt[80];
    int mHeight;     /* viewable rows */
    int mWidth;      /* viewable cols */
    int mOffset;     /* first choice */
    shVector <shMenuChoice *> mChoices;
    int mFlags;
    int mDone;
    int mResultIterator;
    PANEL *mPanel;
    shObjectType mObjTypeHack;

    void accumulateResults ();
    void select (int i1, int i2, int action, shObjectType t = kUninitialized);

 public:

    enum MenuFlags {
        kNoPick = 0x1,      /* display only */
        kMultiPick = 0x2, 
        kCountAllowed = 0x4,
        kAnythingAllowed = 0x8,
        kNothingAllowed = 0x10,
        kCategorizeObjects = 0x20, /* hack */
    };

    shMenu (const char *prompt, int flags);
    ~shMenu ();

    void addItem (char letter, const char *text, const void *value, 
                  int count = 1, int selected = 0);
    void addHeader (const char *text);
    int getResult (const void **value, int *count = NULL);
    char getResult ();
    void finish () {
        assert (kNoPick & mFlags);
        accumulateResults ();
    }
};



extern shInterface *I;


#endif
