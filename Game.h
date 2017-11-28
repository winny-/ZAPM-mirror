#ifndef GAME_H
#define GAME_H

void initializeOptions ();

int nameOK (const char *name);

int newGame (const char *name);

int loadGame (const char *name);

int saveGame ();

void gameLoop ();

/* exitZapm(const code)  declared in Global.h */

#endif
