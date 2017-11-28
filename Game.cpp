#include "Global.h"
#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Hero.h"

int GameOver = 0;

shTime Clock;
shTime MonsterClock = 0;

shVector <shMapLevel *> Maze;  /* all the levels */

void maybeSpawnMonsters ()
{
    if (RNG (Level->mCrList.count () + 10) < 5 + Level->mDLevel / 4) {
        Level->spawnMonsters ();
        MonsterClock += RNG (240000, 720000);
    } else {
        I->debug ("%d monsters already on the level, decided to wait.",
                  Level->mCrList.count ());
        MonsterClock += RNG (60000, 240000);
    }
}

int 
newGame ()
{
    char namebuf[20];
    char *name;

    Clock = 0;

    randomizeIlkNames ();
    initializeObjects ();

    shMapLevel::buildMaze ();
    Level = Maze.get (1);
    Hero.mLevel = Level;

    
    name = getenv ("USER");
    if (!name) {
        I->getStr (namebuf, 16, "What is your name?");
        name = namebuf;
    }
    I->pageLog ();
    shProfession *profession = chooseProfession ();
    if (!profession) { 
        return -1;
    }
    Hero.init (name, profession);

    MonsterClock = RNG (30000, 90000);
    return 0;
}


int 
speedToAP (int speed) 
{
    /* I used to use a complicated priority queue to order events, but I've
       now implemented a speed system that's borrowed from Angband.  
    */

    if (speed >= 100) {
        /* an extra turn for each +100 to speed */
        return speed;
    } else {
        /* every one else gets a turn for each -100 to speed */
        return 10000 / (200 - speed); 
    }
}


void
gameLoop ()
{
    int i;
    shCreature *c;
    shMapLevel *oldlevel = Level;

    Level->computeVisibility ();
    I->drawScreen ();

    while (!GameOver) {
        /* cleanup any dead monsters */
        for (i = Level->mCrList.count () - 1; 
             i >= 0;
             --i) 
        {
            c = Level->mCrList.get (i);
            if (kDead == c->mState) {
                Level->mCrList.remove (c); /* redundant */
                delete c;
            }
        }
        /* maybe hero has a turn */
        I->debug ("hero has %d AP", Hero.mAP);
        if (Hero.mAP >= 0) {
            Hero.takeTurn ();
        }

        if (oldlevel != Level) {
            oldlevel = Level;
            Level->computeVisibility ();
            I->drawScreen ();
            continue;
        }

        /* maybe some monsters have a turn */
        for (i = Level->mCrList.count () - 1; 
             i >= 0 && !GameOver; 
             --i) 
        {
            c = Level->mCrList.get (i);
            if (kDead == c->mState) {
                continue;
            }
            if (c->mAP >= 0) { /* no risk it's hero b/c she just expended AP*/
                c->takeTurn ();
            }
            
            /* award some AP for next time */
            c->mAP += speedToAP (c->mSpeed);
        }

        Clock += 100;

        /* maybe spawn new monsters */
        if (Clock > MonsterClock) {
            maybeSpawnMonsters ();
        }

        if (0 == Clock % 10000) {
            Hero.upkeep ();
        }
    }
}




/*
    switch (getEncumbrance ()) {
    case kBurdened: increaseTime (400); break;
    case kStrained: increaseTime (800); break;
    case kOvertaxed: increaseTime (1200); break;
    case kOverloaded: increaseTime (1600); break;
    }   
    if (Hero.isStunned ()) {
        increaseTime (RNG (500));
    }


*/
