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

#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Hero.h"

void
shInterface::showHelp ()
{
    WINDOW *win;
    PANEL *panel;
    int i;
    int row;

    int height, width;
    getmaxyx(stdscr, height, width);

    win = newwin (height, 80, 0, 0);
    if (!win) {
        I->p ("Couldn't create help window.");
        return;
    }
    panel = new_panel (win);
    mvwaddstr (win, 0, 0, "ZapM Command Help");
   
    wattrset (win, Flags.mVIKeys ? ColorMap[kMagenta] : ColorMap[kBrightMagenta]);
    mvwaddstr (win, 2, 2, "7 8 9");
    mvwaddstr (win, 4, 2, "4   6");
    mvwaddstr (win, 6, 2, "1 2 3");

    wattrset (win, A_NORMAL);
    mvwaddstr (win, 3, 2, " \\|/");
    mvwaddstr (win, 4, 3,  "-@-");
    mvwaddstr (win, 5, 2, " /|\\");

    if (Flags.mVIKeys) {
        mvwaddstr (win, 2, 9, "(Numeric keypad is disabled.)");
    } else {
        mvwaddstr (win, 2, 9, "Use the numeric keypad for movement.");
    }
    mvwaddstr (win, 3, 9, "Move into a monster to fight it.");
    mvwaddstr (win, 4, 9, "Use the '5' or 'g' key followed by a direction key to move");  
    mvwaddstr (win, 5, 9, "   in that direction until something interesting is found.");  
    //mvwaddstr (win, 6, 9, "'F'+<dir> will fight even if no monster is seen.");

    wattrset (win, Flags.mVIKeys ? ColorMap[kMagenta] : ColorMap[kBrightMagenta]);
    mvwaddch (win, 4, 18, '5');
    mvwaddch (win, 4, 25, 'g');

    wattrset (win, Flags.mVIKeys ? ColorMap[kBrightMagenta] : ColorMap[kMagenta]);
    mvwaddstr (win, 8, 2,  "y k u  Y K U");
    mvwaddstr (win, 10, 2, "h   l  H   L");
    mvwaddstr (win, 12, 2, "b j n  B J N");
    wattrset (win, A_NORMAL);
    mvwaddstr (win, 9, 2,  " \\|/    \\|/");
    mvwaddstr (win, 10, 3,  "-@-");
    mvwaddstr (win, 10, 10, "-@-");
    mvwaddstr (win, 11, 2, " /|\\    /|\\");

    if (Flags.mVIKeys) {
        mvwaddstr (win, 8, 16, "Nethack/vi movement keys are enabled; disable ");
    } else {
        mvwaddstr (win, 8, 16, "Nethack/vi movement keys are also supported; enable ");
    }
    mvwaddstr (win, 9, 16, "    them via the Options Menu ('O').");
    mvwaddstr (win, 10, 16, "YUHJKLBN will move in the direction until something ");
    mvwaddstr (win, 11, 16, "   interesting is found. ");

    wattrset (win, Flags.mVIKeys ? ColorMap[kBrightMagenta] : ColorMap[kMagenta]);
    //mvwaddch (win, 6, 10, 'F');
    mvwaddstr (win, 10, 16, "YUHJKLBN");
    wattrset (win, ColorMap[kBrightMagenta]);
    mvwaddch (win, 9, 48, 'O');
    wattrset (win, A_NORMAL);
    
    row = 14;
    for (i = 0; i < KEY_MAX; i++) {
        Command cmd = mKey2Cmd[i];
        int key = i;

        if (cmd <= kAdjust || cmd >= kGodMode) continue;

        if (i < 0x1f) {
            mvwaddch (win, row, 3, '^' | ColorMap[kBrightMagenta]);
            key = i | 0x40;
        } else if (0x80 == (i & 0x80)) {
            mvwaddch (win, row, 2, 'M' | ColorMap[kBrightMagenta]);
            mvwaddch (win, row, 3, '-' | ColorMap[kBrightMagenta]);
            key = i & 0x70;
        }

        mvwaddch (win, row, 4, key | ColorMap[kBrightMagenta]);
        {
            char buf[80];
            strncpy (buf, mCommandHelp[cmd], sizeof(buf));
            mvwaddstr (win, row, 7, buf);
        }
        if (height-1 == ++row) {
            mvwaddstr (win, row, 1, "--More--");
            update_panels (); 
            doupdate();
            I->getChar (win);
            werase (win);
            row = 0;
        }
    }
    mvwaddstr (win, row, 1, "--End--");
    update_panels ();
    doupdate();
    I->getChar (win);
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
}
