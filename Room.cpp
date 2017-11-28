#include <stdlib.h>
#include <string.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"
#include "Hero.h"
#include "MapBuilder.h"


int 
shMapLevel::testSquares (int x1, int y1, int x2, int y2, shTerrainType what)
{
    int x, y;

    for (x = x1; x <= x2; x++) {
        for (y = y1; y <= y2; y++) {
            if (!TESTSQ (x, y, what)) {
                return 0;
            }
        }
    }
    return 1;
}


/* this routine determines if this room is some kind of special room,
   and fills it with objects and monsters and stuff.
 */

void
shMapLevel::decorateRoom (int sx, int sy, int ex, int ey)
{
    if ((ex-sx) * (ey-sy) <= 20 + RNG (1, 20) &&
        mDLevel > 1 &&
        !RNG (1 + mDLevel) &&
        !(kHasShop & mFlags) &&
        makeShop (sx, sy, ex, ey)) 
    {
        return;
    }

    while (!RNG (8)) {  /* secret treasure niche */
        int x, y, dx, dy, horiz;
    failedniche:
        if (!RNG (222)) break;
        if (RNG (2)) {
            x = RNG (sx + 1, ex - 1);
            dx = x; 
            horiz = 1;
            if (RNG (2)) {
                if (!testSquares (x - 1, sy - 2, x + 1, sy - 1, kStone)) 
                    goto failedniche;
                y = sy - 1;
                dy = sy;
            } else {
                if (!testSquares (x - 1, ey + 1, x + 1, ey + 2, kStone)) 
                    goto failedniche;
                y = ey + 1;
                dy = ey;
            }
        } else {
            horiz = 0;
            y = RNG (sy + 1, ey - 1);
            dy = y; 
            horiz = 1;
            if (RNG (2)) {
                if (!testSquares (sx - 2, y - 1, sx - 1, y + 1, kStone)) 
                    goto failedniche;
                x = sx - 1;
                dx = sx;
            } else {
                if (!testSquares (ex + 1, y - 1, ex + 2, y + 1, kStone)) 
                    goto failedniche;
                x = ex + 1;
                dx = ex;
            }
        }
        SETSQ (x-1, y-1, kHWall);
        SETSQ (x,   y-1, kHWall);
        SETSQ (x+1, y-1, kHWall);
        SETSQ (x-1, y,   kVWall);
        SETSQ (x,   y,   kFloor);
        SETSQ (x+1, y,   kVWall);
        SETSQ (x-1, y+1, kHWall);
        SETSQ (x,   y+1, kHWall);
        SETSQ (x+1, y+1, kHWall);

        SETSQ (dx, dy, kFloor);
        addDoor (dx, dy, horiz, 0, RNG (6), !RNG (3), !RNG(3));
        SETSQFLAG (x, y, kHallway);
        if (RNG (6)) putObject (generateObject (mDLevel), x, y);
        if (RNG (2)) putObject (generateObject (mDLevel), x, y);

    }

    if (mDLevel > 7 && !RNG (15)) {
        /* monolith room */
        shMonster *m = new shMonster (findAMonsterIlk ("monolith"));
        putCreature (m, (sx + ex) / 2, (sy + ey) / 2);
        return;
    }

    if ((mDLevel > 6) && !RNG (22) &&
        makeNest (sx, sy, ex, ey)) 
    {
        return;
    }

    mundaneRoom (sx, sy, ex, ey);
}


void
shMapLevel::mundaneRoom (int sx, int sy, int ex, int ey)
{
    int i, npiles, n;
    int x, y;

    if (mDLevel != TOWNLEVEL) {
        if (RNG (mDLevel + 6) > 9) {
            /* dark room */
            for (x = sx; x <= ex; x++) {
                for (y = sy; y <= ey; y++) {
                    setLit (x, y,
                            x > sx && y > sy ? -1 : 0,  //NW
                            x < ex && y > sy ? -1 : 0,  //NE
                            x > sx && y < ey ? -1 : 0,  //SW
                            x < ex && y < ey ? -1 : 0); //SE
                }
            }
        }
        
        if (!RNG (30)) {
            /* radioactive room */
            for (x = sx + 1; x < ex; x++) {
                for (y = sy + 1; y < ey; y++) {
                    mSquares[x][y].mFlags |= shSquare::kRadioactive;
                }
            }
        }
    }

    /* treasure expectation per room: 
       version 0.3:
         0.375 piles x 1.1875 obj per pile + 
         (~) 0.125 niches x 0.8 obj per niche 
         ~ .5453125 objs per room

       version 0.2.9:
         0.375 piles x 1.375 obj per pile 
         = .515625 obj per room
   */

    npiles = RNG (4) ? 0 : RNG (2) + RNG (2) + RNG (2);

    while (npiles--) {
        x = RNG (sx + 1, ex - 1);
        y = RNG (sy + 1, ey - 1);
        if (TESTSQ (x, y, kFloor)  && !isObstacle (x, y)) {
            n = RNG (10) ? 1 : RNG (2, 3);
            for (i = 0; i < n; i++) {
                putObject (generateObject (mDLevel), x, y);
            }
        }
    }

    if (0 == RNG (9)) {
        x = RNG (sx + 1, ex - 1);
        y = RNG (sy + 1, ey - 1);
        if (TESTSQ (x, y, kFloor) && !isObstacle (x, y) && 
            0 == countObjects (x, y)) 
        {
            addVat (x, y);
        }
    }

    while (0 == RNG (12)) {
        x = RNG (sx + 1, ex - 1);
        y = RNG (sy + 1, ey - 1);
        if (TESTSQ (x, y, kFloor) && !isObstacle (x, y) && 
            !getFeature (x, y)) 
        {
            shFeature::Type ttype;
        retrap:
            /* I'm using this retrap label because putting a for or while
               loop here uncovers a bug in g++! */

                ttype = (shFeature::Type) RNG (shFeature::kPit, 
                                              shFeature::kPortal);
                switch (ttype) {
                case shFeature::kPit:
                case shFeature::kTrapDoor:
                    break;
                case shFeature::kAcidPit:
                    if (mDLevel < 7) goto retrap;
                    break;
                case shFeature::kHole:
                    if (RNG (5)) goto retrap;
                    break;
                case shFeature::kWeb:
                    goto retrap;
                case shFeature::kRadTrap:
                    if (mDLevel < 5) goto retrap;
                    break;
                case shFeature::kPortal:
                default: /* these traps are unimplemented */
                    goto retrap;
                }

            addTrap (x, y, ttype);
        }
    }

}


int
shMapLevel::makeGarbageCompactor (int sx, int sy, int ex, int ey)
{
    int x;
    int my;
    int roomid = mSquares[sx][sy].mRoomId;

    mRooms[roomid].mType = shRoom::kGarbageCompactor;

    if ((sy + ey) % 2) {
        return 0;
    }
    my = (sy + ey) / 2;

    for (x = sx + 1; x <= ex -1; x++) {
        addMovingHWall (x, sy+1, sy+1, my);     //north wall
        addMovingHWall (x, ey-1, ey-1, my+1);   //south wall
    }

    mTimeOuts.mCompactor = -1;
    mCompactorState = 0;

    return 1;
}


shFeature *
shMapLevel::addMovingHWall (int x, int y, int beginy, int endy)
{
    shFeature *wall = new shFeature ();
    wall->mType = shFeature::kMovingHWall;
    wall->mX = x;
    wall->mY = y;
    wall->mMovingHWall.mBeginY = beginy;
    wall->mMovingHWall.mEndY = endy;
//    wall->mMovingHWall.mRoomId = roomid;

    mFeatures.add (wall);
    return wall;
}


shFeature *
shMapLevel::addMachinery (int x, int y)
{
    shFeature *f = new shFeature ();
    f->mType = shFeature::kMachinery;
    f->mX = x;
    f->mY = y;
    mFeatures.add (f);
    return f;
}


void
shMapLevel::moveWalls (int action)
{
    shFeature *f;
    
    int north = (1 == action || -1 == action) ? 1 : 0;

    int squares[20];
    int n = 0;
    int x, y;
    int i, j;
    int seen = 0;
    int interrupt = 0;
    int heard = 0;

    // move the wall
    y = -1;
    for (i = 0; i < mFeatures.count (); i++) {
        f = mFeatures.get (i);
        if (shFeature::kMovingHWall != f->mType) {
            continue;
        }
        if (action > 0) {
            /* close in */
            if (north && f->mMovingHWall.mBeginY < f->mMovingHWall.mEndY && 
                f->mY < f->mMovingHWall.mEndY) 
            {
                //if (Hero.canSee (f->mX, f->mY)) 
                //    setMemory (f->mX, f->mY, ' ');
                mVisibility[f->mX][f->mY] = 0;
                y = f->mY + 1;
                squares[n++] = f->mX;
                f->mY++;
                addMachinery (f->mX, f->mY-1);
            } else if (!north && f->mMovingHWall.mBeginY > f->mMovingHWall.mEndY &&
                f->mY > f->mMovingHWall.mEndY) 
            {
                //if (Hero.canSee (f->mX, f->mY)) 
                //    setMemory (f->mX, f->mY, ' ');
                mVisibility[f->mX][f->mY] = 0;
                y = f->mY - 1;
                squares[n++] = f->mX;
                f->mY--;
                addMachinery (f->mX, f->mY+1);
            } else {
                continue;
            }
            if (Hero.canSee (f->mX, f->mY)) {
                interrupt++;
                seen++;
            } else if (distance (&Hero, f->mX, f->mY) < 100) {
                heard++;
            }
        } else if (action < 0) {
            /* reset */
            int oldy = f->mY;
            if (north && f->mMovingHWall.mBeginY < f->mMovingHWall.mEndY && 
                f->mY > f->mMovingHWall.mBeginY) 
            {
                y = f->mY - 1;
                shFeature *machinery = getFeature (f->mX, y);
                if (machinery)
                    removeFeature (machinery);
                f->mY--;
            } else if (!north && 
                       f->mMovingHWall.mBeginY > f->mMovingHWall.mEndY &&
                       f->mY < f->mMovingHWall.mBeginY)
            {
                y = f->mY + 1;
                shFeature *machinery = getFeature (f->mX, y);
                if (machinery)
                    removeFeature (machinery);
                f->mY++;
            } else {
                continue;
            }
            if (Hero.canSee (f->mX, oldy))
                interrupt++;
        }
    }

    if (!Hero.getStoryFlag ("walls moving")) {
        if (seen) {
            I->p ("The walls are moving!");
            Hero.setStoryFlag ("walls moving", 1);
        } else if (heard && !Hero.getStoryFlag ("walls heard")) {
            I->p ("You hear a loud rumbling!");
            Hero.setStoryFlag ("walls heard", 1);
        }
    }

    // displace objects and creatures

    if (n) {
        shuffle (squares, n, sizeof(int));

        for (i = 0; i < n; i++) {
            x = squares[i];

            shObjectVector *v = getObjects (x, y);
            if (v) {
                int y2 = north ? y + 1 : y - 1;
                int safe = !isObstacle (x, y2);
                for (j = 0; j < v->count (); j++) {
                    shObject *obj = v->get (j);
                    if (safe) {
                        putObject (obj, x, y2);
                    } else {
                        delete obj;
                    }
                }
                if (Hero.mX == x && Hero.mY == y2) {
                    I->p ("%s pushed into your vicinity.",
                          v->count () > 1 ? "Some objects are" 
                                          : "An object is");
                } 
                delete v;
                setObjects (x, y, NULL);
            }

            shCreature *c = getCreature (x, y);
            if (c) { 
                if (c->mZ < 0) {
                    if (c->isHero ()) {
                        I->p ("You are sealed below the moving wall!");
                        I->p ("That's not supposed to be possible!");
                        I->p ("Please send me a bug report! -cyrus");
                    }
                } else {
                    pushCreature (c, north ? kSouth : kNorth);
                }
            }
        }
    }
    if (interrupt)
        Hero.interrupt ();
}


//RETURNS: 0 on success, -1 o/w
int 
shMapLevel::pushCreature (shCreature *c, shDirection dir)
{
    shDirection dirlist[3];
    int r = RNG (1, 2);
    int i;
    
    dirlist[0] = dir;
    dirlist[r] = (shDirection) ((dir + 1) % 8);
    dirlist[3-r] = (shDirection) ((dir + 7) % 8);

    for (i = 0; i < 3; i++) {
        int x = c->mX;
        int y = c->mY;
        shDirection dir = dirlist[i];

        moveForward (dir, &x, &y);
        
        if (isObstacle (x, y) || isOccupied (x, y))
            continue;

        return moveCreature (c, x, y);
    }
    
    if (c->isHero ()) {
        I->p ("You are crushed between the walls!");
    } else if (Hero.canSee (c)) {
        I->p ("%s is crushed!", THE (c));
    }
    c->die (kMisc, "Crushed by a garbage compactor");
    return -1;
}





void 
shMapLevel::magDoors (int action)  /* 1 lock, -1 unlock */
{
    shFeature *f;
    int i;
    int heard = 0;

    for (i = 0; i < mFeatures.count (); i++) {
        f = mFeatures.get (i);
        if (f->isMagneticallySealed ()) {
            if (action > 0 && !f->isOpenDoor () && !f->isLockedDoor ()) {
                // TODO: if the door is open, slam it shut
                f->lockDoor ();
            } else if (action < 0 && !f->isOpenDoor () && f->isLockedDoor ()) {
                f->unlockDoor ();
            } else {
                continue;
            }
            if (distance (&Hero, f->mX, f->mY) < 50) {
                heard++;
            }
        }
    }

    if (heard) {
        I->p ("You hear %s.", heard > 1 ? "some clicks" : "something click");
    }
}


int
shMapLevel::makeHospital (int sx, int sy, int ex, int ey)
{
    int dx;
    int dy;
    shMonster *docbot;
    int roomid = mSquares[sx][sy].mRoomId;
    int i;

    dx = (sx + ex + RNG (0, 1)) / 2;
    dy = (sy + ey + RNG (0, 1)) / 2;
    
    docbot = new shMonster (findAMonsterIlk ("docbot"));
    putCreature (docbot, dx, dy);
    docbot->mDoctor.mHomeX = dx;
    docbot->mDoctor.mHomeY = dy;
    docbot->mDoctor.mRoomID = roomid;
    for (i = 0; i < kMedMaxService; i++)
        docbot->mDoctor.mPermute[i] = i;
    shuffle (docbot->mDoctor.mPermute, kMedMaxService, sizeof (int));

    mRooms[roomid].mType = shRoom::kHospital;

    I->debug ("made hospital on level %d", Maze.find(this));

    return 1;
}


/* returns 1 if shop successfully created, 0 o/w
 */

int
shMapLevel::makeShop (int sx, int sy, int ex, int ey, int kind)
{
    int x, y;
    int dx = -1;
    int dy = -1;
    shFeature *f;
    shFeature *door = NULL;
    shMonster *clerk;
    int roomid = mSquares[sx][sy].mRoomId;

    /* find the door (we only make shops in rooms with exactly 1 door */
    for (x = sx + 1; x < ex; x++) {
        f = getFeature (x, sy);
        if (f && f->isDoor ()) {
            if (door) return 0;
            door = f;
            dx = x; 
            dy = sy;
        }
        f = getFeature (x, ey);
        if (f && f->isDoor ()) {
            if (door) return 0;
            door = f;
            dx = x; 
            dy = ey;
        }
    }
    for (y = sy + 1; y < ey; y++) {
        f = getFeature (sx, y);
        if (f && f->isDoor ()) {
            if (door) return 0;
            door = f;
            dx = sx; 
            dy = y;
        }
        f = getFeature (ex, y);
        if (f && f->isDoor ()) {
            if (door) return 0;
            door = f;
            dx = ex; 
            dy = y;
        }
    }

    if (!door) {
        /* wtf? there are no doors to this room? */
        return 0;
    }

    /* make sure door is a normal automatic door */

    door->mType = shFeature::kDoorClosed;
    door->mDoor = (door->mDoor & shFeature::kHoriz) | shFeature::kAutomatic;

    if (dx == sx) {
        sx++; dx++;
    } else if (dx == ex) {
        ex--; dx--;
    }
    if (dy == sy) {
        sy++; dy++;
    } else if (dy == ey) {
        ey--; dy--;
    }

    if (-1 == kind) { /* randomly determind kind of shop */
        switch (RNG (20)) {
        case 0: case 1: case 2: case 3: case 4: 
        case 5: case 6: case 7: case 8:
            kind = shRoom::kGeneralStore; break;
        case 9: case 10: case 11:
            kind = shRoom::kHardwareStore; break;
         case 12: case 13: case 14:
            kind = shRoom::kSoftwareStore; break;
        case 15: case 16:
            kind = shRoom::kArmorStore; break;
        case 17: case 18:
            kind = shRoom::kWeaponStore; break;
        case 19:
            kind = shRoom::kImplantStore; break;
        }
    }
    
    for (x = sx + 1; x < ex; x++) {
        for (y = sy + 1; y < ey; y++) {
            shObject *obj = NULL;

            switch (kind) {
            case shRoom::kGeneralStore:
                obj = generateObject (-1); break;
            case shRoom::kHardwareStore:
                obj = createTool (); break;
            case shRoom::kSoftwareStore:
                if (RNG (8)) {
                    obj = createFloppyDisk ();
                } else {
                    obj = createObject ("computer", 0);
                } break;
            case shRoom::kArmorStore:
                obj = createArmor (); break;
            case shRoom::kWeaponStore:
                obj = createWeapon (); break;
            case shRoom::kImplantStore:
                obj = createImplant (); break;
            }
            obj->setUnpaid ();
            putObject (obj, x, y);
        }
    }
    
    clerk = new shMonster (findAMonsterIlk ("clerkbot"));
    putCreature (clerk, dx, dy);
    clerk->mShopKeeper.mHomeX = dx;
    clerk->mShopKeeper.mHomeY = dy;
    clerk->mShopKeeper.mShopId = roomid;
    clerk->mShopKeeper.mBill = 0;
    mRooms[roomid].mType = (shRoom::Type) kind;
    //mRooms[roomid].mShopKeeper = clerk;
    mFlags |= kHasShop;

    I->debug ("made shop on level %d", Maze.find(this));

    return 1;
}


int
shMapLevel::makeNest (int sx, int sy, int ex, int ey)
{
    int x, y;
    int roomid = mSquares[sx][sy].mRoomId;
    
    if (mDLevel > 12) {
        shMonster *queen = new shMonster (findAMonsterIlk ("alien queen"));
        queen->mStrategy = shMonster::kLurk;
        putCreature (queen, RNG (sx + 1, ex - 1), RNG (sy + 1, ey -1));
    }

    I->debug ("made nest on level %d", Maze.find (this));

    for (x = sx + 1; x < ex ; x++) {
        for (y = sy + 1; y < ey; y++) {
            if (!isOccupied (x, y)) {
                shMonster *alien = NULL;
                if (x%4 && y%3) {
                    alien = new shMonster (findAMonsterIlk ("alien egg"));
                } else if (!RNG (8) && mDLevel >= 8) { 
                    alien = new shMonster (findAMonsterIlk ("alien warrior"));
                    alien->mStrategy = shMonster::kLurk;
                }
                if (alien) {
                    putCreature (alien, x, y);
                }
            }
        }
    }


    mRooms[roomid].mType = shRoom::kNest;
    return 1;
}


shFeature *
shMapLevel::addTrap (int x, int y, shFeature::Type type)
{
    shFeature *trap = new shFeature ();
    trap->mX = x;
    trap->mY = y;
    if (shFeature::kHole != type) {
        trap->mTrapUnknown = 1;
        trap->mTrapMonUnknown = 1;
    }
retry:
    switch (type) {
    case shFeature::kSewagePit:
        if (kSewage != getSquare (x, y) ->mTerr) {
            type = shFeature::kPit;
        }
        break;
    case shFeature::kPit:
    case shFeature::kAcidPit:
        switch (getSquare (x, y) ->mTerr) {
        case kSewage:
            type = shFeature::kSewagePit;
            break;
        default:
            break;
        }
        break;
    case shFeature::kHole:
    case shFeature::kTrapDoor:
        if (noDig () || isBottomLevel ()) {
            type = shFeature::kPit;
            goto retry;
        } else {
            trap->mDest.mLevel = 0;
            trap->mDest.mX = -1;
            trap->mDest.mY = -1;
        }
        break;
    default:
        break;
    }
    trap->mType = type;
    mFeatures.add (trap);
    return trap;
}
