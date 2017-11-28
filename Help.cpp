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

    win = newwin (25, 80, 0, 0);
    if (!win) {
        I->p ("Couldn't create help window.");
        return;
    }
    panel = new_panel (win);
    mvwaddstr (win, 0, 35, "ZapM Command Help");

    wattrset (win, ColorMap[kBrightMagenta]);

    mvwaddstr (win, 2, 2, "7 8 9  y k u");
    mvwaddstr (win, 4, 2, "4   6  h   l");
    mvwaddstr (win, 6, 2, "1 2 3  b j n");

    wattrset (win, A_NORMAL);
    mvwaddstr (win, 3, 2, " \\|/    \\|/");
    mvwaddstr (win, 4, 3,  "-@-");
    mvwaddstr (win, 4, 10, "-@-");
    mvwaddstr (win, 5, 2, " /|\\    /|\\");

    mvwaddstr (win, 2, 16, "Use the numeric keypad for movement.");
    mvwaddstr (win, 3, 16, "Nethack/vi movement keys (hjkl, yubn) are also supported.");
    mvwaddstr (win, 4, 16, "Move into a monster to fight it.");
    mvwaddstr (win, 5, 16, "Use the '5' or 'g' key followed by a direction key to move");  
    mvwaddstr (win, 6, 16, "in that direction until something interesting is found.");  


    
    row = 8;
    for (i = 0; i < KEY_MAX; i++) {
        Command cmd = mKey2Cmd[i];
        int key = i;

        if (cmd < kMoveDown || cmd >= kGodMode) continue;

        if (i < 0x1f) {
            mvwaddch (win, row, 3, '^' | ColorMap[kBrightMagenta]);
            key = i | 0x40;
        } else if (0x80 == i & 0x80) {
            mvwaddch (win, row, 2, 'M' | ColorMap[kBrightMagenta]);
            mvwaddch (win, row, 3, '-' | ColorMap[kBrightMagenta]);
            key = i & 0x70;
        }

        mvwaddch (win, row, 4, key | ColorMap[kBrightMagenta]);
        mvwaddstr (win, row, 7, mCommandHelp[cmd]);

        if (24 == ++row) {
            mvwaddstr (win, row, 1, "--More--");
            update_panels (); 
            doupdate();
            I->getChar ();
            werase (win);
            row = 0;
        }
    }
    mvwaddstr (win, row, 1, "--End--");
    update_panels ();
    doupdate();
    I->getChar ();
    hide_panel (panel);
    del_panel (panel);
    delwin (win);
    update_panels (); 
    I->drawScreen ();
}
