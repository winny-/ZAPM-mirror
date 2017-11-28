/* God Mode commands */

#include "Global.h"
#include "Util.h"
#include "Hero.h"



static int
showRooms ()
{
    int x; int y;
    for (y = 0; y < MAPMAXROWS; y++) {
        for (x = 0; x < MAPMAXCOLUMNS; x++) {
            int c = ' ';
            if (!Level->isInRoom (x, y)) {
                continue;
            }
            int room = Level->getRoomID (x, y);
            if (room < 10) {
                c = '0' + room;
            } else if (room < 36) {
                c = 'a' + room - 10;
            } else if (room < 62) {
                c = 'A' + room - 36;
            } else {
                c = '*';
            }

            if (Level->stairsOK (x, y)) 
                c |= ColorMap[kBrightMagenta];
                   
            Level->setMemory (x, y, c);
            Level->drawSq (x, y);
        }
    }
    I->pauseXY (Hero.mX, Hero.mY);
    return 0;
}




void
shHero::doGodMode ()
{
    shMenu menu ("What is thy bidding, master?", 0);
    int choice;

    menu.addItem ('b', "buff up", (void *) 8, 1);
    menu.addItem ('d', "diagnostics", (void *) 12, 1);
    menu.addItem ('f', "create feature", (void *) 10, 1);
    menu.addItem ('i', "identify item", (void *) 1, 1);
    menu.addItem ('l', "gain level", (void *) 2, 1);
    menu.addItem ('m', "create monster", (void *) 3, 1);
    menu.addItem ('o', "create object", (void *) 4, 1);
    menu.addItem ('r', "reveal map", (void *) 5, 1);
    menu.addItem ('s', "spoilers", (void *) 6, 1);
    menu.addItem ('t', "transport", (void *) 7, 1);
    menu.addItem ('T', "level transport", (void *) 9, 1);
    menu.addItem ('R', "show RoomIDs", (void *) 11, 1);

    menu.getResult ((const void **) &choice, NULL);
    
    if (1 == choice) {
        identifyObjects (1);
    } else if (2 == choice) {
        mXP += 1000;
        levelUp ();
        //I->p ("You've obtained level %d!", mCLevel);
        I->drawScreen ();
    } else if (3 == choice) {
        char desc[100];
        shMonsterIlk *ilk;
        shMonster *monster;
        int x = mX;
        int y = mY;
        
        I->getStr (desc, 80, "Create what kind of monster?");
        ilk = findAMonsterIlk (desc);
        if (NULL == ilk) {
            I->p ("There's no such monster!");
            return;
        }
        Level->findNearbyUnoccupiedSquare (&x, &y);
        if (0 == I->getSquare ("Where should it spawn? (select a location)",
                               &x, &y, -1))
        {
            return;
        }
        monster = new shMonster (ilk);
        if (0 == Level->putCreature (monster, x, y)) {
            I->drawScreen ();
        } else {
            //FIX: might not have been deleted
            //delete monster;
        }
    } else if (4 == choice) {
        char desc[100];
        shObject *obj;
        I->getStr (desc, 80, "Create what object?");
        obj = createObject (desc, 0);
        if (obj) {
            if (!Hero.isBlind ()) {
                obj->setAppearanceKnown ();
            }
            if (0 == addObjectToInventory (obj)) {
                I->p ("The object slips from your hands!");
                Level->putObject (obj, mX, mY);
            }
            I->drawScreen ();
        } else {
            I->p ("Do not know how to make %s.", desc);
        }
    } else if (5 == choice) {
        Level->reveal ();
        I->p ("Map revealed.");
    } else if (6 == choice) {
        monsterSpoilers ();
    } else if (7 == choice) {
        int x = -1, y = -1;
        if (I->getSquare ("Transport to what location?", &x, &y, -1)) {
            Hero.transport (x, y, 100);
            I->drawScreen ();
        }
    } else if (8 == choice) {
        Hero.mMaxHP += 100;
        Hero.mHP = Hero.mMaxHP;
        addObjectToInventory (
            createObject ("optimized disintegration ray gun", 0));
    } else if (9 == choice) {
        int i;
        shMenu lmenu ("Warp to what level?", 0);
        shMapLevel *L;
        for (i = 1; i < Maze.count (); i++) {
            L = Maze.get (i);
            lmenu.addItem (i <= 26 ? i + 'a' - 1 
                                   : i + 'A' - 27, 
                           L->mName, L, 1);
        }
        lmenu.getResult ((const void **) &L, NULL);
        if (L) {
            Level->warpCreature (&Hero, L);
            I->drawScreen ();
        }
    } else if (10 == choice) {
        shFeature::Type t;
        int x = Hero.mX, y = Hero.mY;
        shMenu fmenu ("Create what kind of feature?", 0);
        fmenu.addItem ('a', "pit trap", (void *) shFeature::kPit, 1);
        fmenu.addItem ('b', "acid pit trap", (void *) shFeature::kAcidPit, 1);
        fmenu.addItem ('c', "radiation trap", (void *) shFeature::kRadTrap, 1);
        fmenu.addItem ('d', "hole", (void *) shFeature::kHole, 1);
        fmenu.addItem ('e', "trap door", (void *) shFeature::kTrapDoor, 1);
        fmenu.getResult ((const void **) &t, NULL);
        if (!t) {
            return;
        }
        if (0 == I->getSquare ("Where should I put it? (select a location)",
                               &x, &y, -1))
        {
            return;
        }
        Level->addTrap (x, y, t);
    } else if (11 == choice) {
        showRooms ();
        return;
    } else if (12 == choice) {
        Hero.doDiagnostics ();
        return;
    }
}


