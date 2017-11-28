/* God Mode commands */

#include "Global.h"
#include "Util.h"
#include "Hero.h"

void
shHero::doGodMode ()
{
    shMenu menu ("What is thy bidding, master?", 0);
    int choice;

    menu.addItem ('b', "buff up", (void *) 8, 1);
    menu.addItem ('i', "identify item", (void *) 1, 1);
    menu.addItem ('l', "gain level", (void *) 2, 1);
    menu.addItem ('m', "create monster", (void *) 3, 1);
    menu.addItem ('o', "create object", (void *) 4, 1);
    menu.addItem ('r', "reveal map", (void *) 5, 1);
    menu.addItem ('s', "spoilers", (void *) 6, 1);
    menu.addItem ('t', "transport", (void *) 7, 1);
    menu.addItem ('T', "level transport", (void *) 9, 1);

    menu.getResult ((void **) &choice, NULL);
    
    if (1 == choice) {
        identifyObjects (1);
    } else if (2 == choice) {
        mXP += 1000;
        levelUp ();
        I->p ("You've obtained level %d!", mCLevel);
        I->drawScreen ();
    } else if (3 == choice) {
        char desc[80];
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
        char desc[80];
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
        shMenu lmenu ("warp to what level?", 0);
        shMapLevel *L;
        for (i = 1; i < Maze.count (); i++) {
            L = Maze.get (i);
            lmenu.addItem (i < 26 ? i + 'a' : i + 'A', L->mName, L, 1);
        }
        lmenu.getResult ((void **) &L, NULL);
        if (L) {
            Level->warpCreature (&Hero, L);
            I->drawScreen ();
        }
    }
}
