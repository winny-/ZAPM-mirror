#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"
#include "Hero.h"

#if 0 
char gASCIITiles[256] = {
    ' ', '|', '-', '+', '+', '+', '+', '+', '+', '+', '|', '|', 'x', '.'
};
#else 
char gASCIITiles[256] = {
    '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'x', '.'
};
#endif

#define SETSQ(_x, _y, _w) mSquares[_x][_y].mTerr = _w
#define SETROOM(_x, _y, _r) mSquares[_x][_y].mRoomId = _r

const char *
stringDirection (shDirection d) 
{
    static const char *txt[12] = {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW", "U", "D", "O", "0"
    };

    return txt[d];
}


/* returns move-cost distance in feet between two squares */
int
rlDistance (int x1, int y1, int x2, int y2)
{
    int horiz = x2 > x1 ? x2 - x1 : x1 - x2;
    int vert = y2 > y1 ? y2 - y1 : y1 - y2;

    if (vert > horiz) {
        return (int) (5 * (sqrt (2) * horiz + vert - horiz));
    } else {
        return (int) (5 * (sqrt (2) * vert + horiz - vert));
    }
}



//RETURNS: distance in feet between 2 squares
int 
distance (int x1, int y1, int x2, int y2)
{
    return (int) (sqrt ((x1 - x2) * (x1 - x2) * 25 + 
                        (y1 - y2) * (y1 - y2) * 25)); 
}


//RETURNS: distance in feet between 2 entities
int
distance (shCreature *e1, shCreature *e2)
{
    return distance (e1->mX, e1->mY, e2->mX, e2->mY);
}


int
distance (shCreature *e1, int x1, int y1)
{
    return distance (e1->mX, e1->mY, x1, y1);
}


int
isDiagonal (shDirection d)
{
    return (kNorthEast == d || kSouthEast == d || 
            kSouthWest == d || kNorthWest ==d);
}


/* MODIFIES: *x, *y, and *z to reflect movement of one square in direction d 
   RETURNS:  true if the new square is inbounds, false o/w
*/
int
shMapLevel::moveForward (shDirection d, int *x, int *y, 
                         int *z /* = NULL */)
{
    switch (d) {
    case kNorth:
        *y -= 1; break;
    case kEast:
        *x += 1; break;
    case kSouth:
        *y += 1; break;
    case kWest:
        *x -= 1; break;
    case kNorthEast:
        --*y; ++*x; break;
    case kSouthEast:
        ++*y; ++*x; break;
    case kSouthWest:
        ++*y; --*x; break;
    case kNorthWest:
        --*y; --*x; break;
    case kUp:
        if (z) ++*z; break;
    case kDown:
        if (z) --*z; break;
    case kNoDirection:
    case kOrigin:
        break;
    default:
        abort ();
    }
    return (*x >= 0 && *x < mColumns && *y >= 0 && *y < mRows);
}


shDirection
vectorDirection (int x1, int y1)
{
    return vectorDirection (0, 0, x1, y1);
}


shDirection
vectorDirection (int x1, int y1, int x2, int y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;

    if (0 == dx) {
        return dy < 0 ? kNorth : 
               dy > 0 ? kSouth : kNoDirection;
    }
    double slope = dy / dx;
       
    if (slope > 2) {
        return dy < 0 ? kNorth : kSouth;
    } else if (slope > 0.5) {
        return dy < 0 ? kNorthWest : kSouthEast;
    } else if (slope > -0.5) {
        return dx < 0 ? kWest : kEast;
    } else if (slope > -2) {
        return dy < 0 ? kNorthEast : kSouthWest;
    } else {
        return dy < 0 ? kNorth : kSouth;
    }
}

/* returns: the direction from the first square to the second, if the squares
            are lined up in one of the 8 primary directions; o/w kNoDirection
 */
shDirection
linedUpDirection (int x1, int y1, int x2, int y2)
{
    if (x1 == x2) {
        return (y1 < y2) ? kSouth : kNorth;
    } else if (y1 == y2) {
        return (x1 < x2) ? kEast : kWest;
    } else if (x1 - y1 == x2 - y2) {
        return (x1 < x2) ? kSouthEast : kNorthWest;
    } else if (x1 + y1 == x2 + y2) {
        return (x1 < x2) ? kNorthEast : kSouthWest;
    } else {
        return kNoDirection;
    }
}


shDirection 
linedUpDirection (shCreature *e1, shCreature *e2)
{
    return linedUpDirection (e1->mX, e1->mY, e2->mX, e2->mY);
}




int
areAdjacent (int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    return ! (dx > 1 || dx < -1 || dy > 1 || dy < -1);
}

//RETURNS: distance in feet between 2 creature
int
areAdjacent (shCreature *c1, shCreature *c2)
{
    return areAdjacent (c1->mX, c1->mY, c2->mX, c2->mY);
}


int
shFeature::getDescription (char *buf, int len)
{
    const char *d;

    switch (mType) {
    case kDoorClosed: d = "door"; break;
    case kStairsUp: d = "staircase up"; break;
    case kStairsDown: d = "staircase down"; break;
    case kComputerTerminal: d = "computer terminal"; break;
    case kRadTrap: d = "radiation trap"; break;
    case kDoorOpen: d = "open doorway"; break;
    case kVat: d = "sludge vat"; break;
    case kPit: d = mTrapUnknown ? "pit trap" : "pit"; break;
    case kAcidPit: d = mTrapUnknown ? "acid pit trap" : "acid pit"; break;
    case kTrapDoor: d = "trap door"; break;
    case kHole: d = "hole"; break;
    case kDoorHiddenHoriz:
    case kDoorHiddenVert: 
        d = "wall"; break;
    default: d = "strange feature"; break;
    }
    return snprintf (buf, len, "%s", d);
}


void
shMapLevel::debugDraw ()
{
    int x, y;
    printf ("\n\n\n");
    for (y = 0; y < mRows; y++) {
        for (x = 0; x < mColumns; x++) {
            putc (gASCIITiles[mSquares[x][y].mTerr], stdout);
        }
        putc ('\n', stdout);
    }
}


void 
shMapLevel::setLit (int x, int y, int lit)
{
    if (lit) { 
        mSquares[x][y].mFlags &= ~shSquare::kDark;
    } else { 
        mSquares[x][y].mFlags |= shSquare::kDark; 
        if (I->mSqGlyphs[kStoneFloor] == getMemory (x, y)) {
            setMemory (x, y, ' ');
        }
    }
}


void
shMapLevel::layCorridor (int x1, int y1, int x2, int y2)
{
    int i;
    int swap;

    if (y1 > y2) {
        swap = y1; y1 = y2; y2 = swap;
    }
    if (x1 > x2) {
        swap = x1; x1 = x2; x2 = swap;
    }
    if (x1 == x2) {
        for (i = y1; i <= y2; i++) {
            SETSQ (x1, i, kStoneFloor);
            SETSQ (x1 - 1, i, kVWall); // west wall
            SETSQ (x1 + 1, i, kVWall); // east wall
        }
    }
    else if (y1 == y2) {
        for (i = x1; i <= x2; i++) {
            SETSQ (i, y1, kStoneFloor);     // floor
            SETSQ (i, y1 - 1, kHWall); // north wall
            SETSQ (i, y1 + 1, kHWall); // south wall
        }
    }
    else {
        abort ();
    }
}

void
shMapLevel::layRoom (int x1, int y1, int x2, int y2)
{
    int i, j;
    
    SETSQ (x1, y1, kNWCorner); SETROOM (x1, y1, mNumRooms);
    SETSQ (x1, y2, kSWCorner); SETROOM (x1, y2, mNumRooms);
    SETSQ (x2, y1, kNECorner); SETROOM (x2, y1, mNumRooms);
    SETSQ (x2, y2, kSECorner); SETROOM (x2, y2, mNumRooms);
    for (i = x1 + 1; i < x2; i++) {
        SETSQ (i, y1, kHWall); SETROOM (i, y1, mNumRooms);
        SETSQ (i, y2, kHWall); SETROOM (i, y2, mNumRooms);
    }
    for (i = y1 + 1; i < y2; i++) {
        SETSQ (x1, i, kVWall); SETROOM (x1, i, mNumRooms);
        SETSQ (x2, i, kVWall); SETROOM (x2, i, mNumRooms);
    }

    for (i = x1 + 1; i < x2; i++) {
        for (j = y1 + 1; j < y2; j++) {
            SETSQ (i, j, kStoneFloor);
            SETROOM (i, j, mNumRooms);
        }
    }

    mRooms[mNumRooms].mType = shRoom::kNormal;
    mNumRooms++;
}

void
shMapLevel::buildDenseLevel ()
{
    int x, y, l;

    //dumbass T setup:

    x = RNG (40);
    y = RNG (8) + 6;
    l = 15 + RNG (20);

    printf ("%d %d %d\n", x, y, l); 

    layCorridor (x, y, x + l, y);
    layRoom (x + RNG (5), RNG (5), x + l - RNG (5), y - 1);
    SETSQ (x + l/2, y-1, kStoneFloor);
}


void
shMapLevel::buildMaze ()
{
    int i;
    shMapLevel *A, *L;
 
    Maze.add (NULL); /* dummy zeroth level */
    for (i = 1; i <= BUNKERLEVELS; i++) {
        if (TOWNLEVEL == i) {
            Maze.add (new shMapLevel (i, kTown));
        } else if (BUNKERLEVELS == i) {
            Maze.add (new shMapLevel (i, kRabbit));
        } else {
            Maze.add (new shMapLevel (i, kBunkerRooms));
        }
    }
/*
    { 
        int x, y;
        shMonster *rabbit = new shMonster (findAMonsterIlk ("killer rabbit"));
        Maze.get (i - 1) -> findUnoccupiedSquare (&x, &y);
        Maze.get (i - 1) -> putCreature (rabbit, x, y);
    }
*/
    for (i = 1; i < TOWNLEVEL-1; i++) {
        Maze.get (i) -> addDownStairs (-1, -1, Maze.get (i + 1), -1, -1);
    }
    Maze.get (TOWNLEVEL - 1) -> 
        addDownStairs (-1, -1, Maze.get (TOWNLEVEL), 7, 3);
    Maze.get (TOWNLEVEL) -> 
        addDownStairs (6, 17, Maze.get (TOWNLEVEL + 1), -1, -1);
    for (i = TOWNLEVEL + 1; i < BUNKERLEVELS - 1; i++) {
        Maze.get (i) -> addDownStairs (-1, -1, Maze.get (i + 1), -1, -1);
    }
    Maze.get (BUNKERLEVELS - 1) ->
        addDownStairs (-1, -1, Maze.get (BUNKERLEVELS), 20, 10);
    for (i = 0; i < CAVELEVELS; i++) {
        L = new shMapLevel (i + TOWNLEVEL + 1, kRadiationCave);
        Maze.add (L);
        if (0 == i) {
            Maze.get (TOWNLEVEL) -> addDownStairs (53, 14, L, -1, -1);
        } else {
            A -> addDownStairs (-1, -1, L, -1, -1);
        }
        A = L;
    }
    L = new shMapLevel (i + TOWNLEVEL + 1, kMainframe);
    Maze.add (L);
    A -> addDownStairs (-1, -1, L, 6, 16);
    { 
        int x = RNG (40, 50);
        int y = RNG (2, 10);
        
        L->findNearbyUnoccupiedSquare (&x, &y);
        L->putObject (createObject ("Bizarro Orgasmatron", 0), x, y);
    }
}


//constructor

shMapLevel::shMapLevel (int level, MapType type)
    : mCrList (), mFeatures (), mExits ()
{
    int x; int y;
retry:
    I->debug ("building level %d", level);

    memset (&mSquares, 0, sizeof (mSquares));
    memset (&mObjects, 0, sizeof (mObjects));
    memset (&mCreatures, 0, sizeof (mCreatures));
    for (y = 0; y < MAPMAXROWS; y++) 
        for (x = 0; x < MAPMAXCOLUMNS; x++)
            setMemory (x, y, ' ');
    mRooms[0].mType = shRoom::kNotRoom;
    
    mRows = MAPMAXROWS;
    mColumns = MAPMAXCOLUMNS;
    mRows = 20;
    mColumns = 60;
    mFlags = 0;
    mNumRooms = 1;
    mDLevel = level;

    switch (type) {
    case kTown:
        snprintf (mName, 12, "Robot Town");
        buildTown ();
        return;
    case kRadiationCave:
        snprintf (mName, 12, "Gamma Caves");
        buildCave ();
        return;
    case kMainframe:
        snprintf (mName, 12, "Mainframe");
        buildMainframe ();
        return;
    case kRabbit:
        snprintf (mName, 12, "Rabbit Hole");
        buildRabbitLevel ();
        return;
    case kBunkerRooms:
    default:
        snprintf (mName, 12, "Space Base");
        if (0 == buildBunkerRooms ()) {
            goto retry;
        }
        return;
    }
}

shMonster *
shMapLevel::getShopKeeper (int x, int y)
{
    int i;
    if (!isInShop (x, y)) return NULL;
    
    for (i = 0; i < mCrList.count (); i++) {
        shMonster *c = (shMonster *) mCrList.get (i);
        if (c->isA ("clerkbot") &&
            c->mShopKeeper.mShopId == mSquares[x][y].mRoomId)
        {
            return c;
        }
    }
    return NULL;
    //return mRooms[mSquares[x][y].mRoomId].mShopKeeper;
}


shMonster *
shMapLevel::getGuard (int x, int y)
{
    int i;
    
    for (i = 0; i < mCrList.count (); i++) {
        shMonster *c = (shMonster *) mCrList.get (i);
        if (c->isA ("guardbot") &&
            Hero.canSee (c) &&
            !c->isPet ())
        {
            return c;
        }
    }
    return NULL;
}


shObject *
shMapLevel::findObject (int x, int y, char *ilk)
{
    shObjectVector *v = getObjects (x, y);
    int i;

    if (!v) {
        return NULL;
    }
    for (i = 0; i < v->count (); i++) {
        if (v->get (i) -> isA (ilk)) {
            return v->get (i);
        }
    }
    return NULL;
}


/* might delete obj! */
int
shMapLevel::putObject (shObject *obj, int x, int y)
{
    int i;
    if (isFloor (x, y)) {
        if (NULL == mObjects[x][y]) {
            mObjects[x][y] = new shObjectVector ();
        }
        for (i = 0; i < mObjects[x][y] -> count (); i++) {
            shObject *fobj = mObjects[x][y] -> get (i);
            if (fobj->canMerge (obj)) {
                fobj->merge (obj);
                return 0;           
            }
        }
        obj->mLocation = shObject::kFloor;
        obj->mX = x;
        obj->mY = y;
        mObjects[x][y] -> add (obj);
        return 0;
    }
    return -1;
}


int
shMapLevel::warpCreature (shCreature *c, shMapLevel *newlevel)
{
    int res, x, y;
    int oldx = c->mX;
    int oldy = c->mY;

    if (c->isHero () && isInShop (c->mX, c->mY)) {
        Hero.leaveShop ();
    }
    removeCreature (c);
    do {
        newlevel->findSquare (&x, &y);
        if (newlevel->getFeature (x, y)) {
            /* just to be safe */
            continue;
        }
        res = newlevel->putCreature (c, x, y);
        if (1 == res) {
            return 1;
            break;
        }
    } while (-1 == res);
    if (c->isHero ()) {
        Level = newlevel;
        Hero.checkForFollowers (this, oldx, oldy);
    }
    return 0;
}

shAttack SlammingDoorDamage =
    shAttack (NULL, shAttack::kCrush, shAttack::kSingle, 0, kConcussive, 1, 6);

shAttack RadTrapDamage =
    shAttack (NULL, shAttack::kGammaRay, shAttack::kSingle, 0, 
              kRadiological, 1, 30);

/* returns non-zero if the creature at x,y dies */
int
shMapLevel::checkTraps (int x, int y, int savedcmod)
{
    if (!isOccupied (x, y)) return 0;
    shCreature *c = getCreature (x, y);
    shFeature *f = getFeature (x, y);
    char buf[64];
    c->the (buf, 64);

    if (!f) return 0;
    
    if (!f->mTrapUnknown) savedcmod -= 5;

    if (shFeature::kDoorOpen == f->mType &&
        shFeature::kBerserk & f->mDoor &&
        0 == countObjects (x, y) &&
        RNG (3))
    { /* berserk door! */
        I->drawScreen ();
        if (c->isHero ()) {
            f->mTrapUnknown = 0;
            if (Hero.reflexSave (NULL, 20 + savedcmod)) {
                I->p ("The door slams shut!  "
                      "You jump out of the way!");
                return 0;
            }
            I->p ("The door slams shut on you!");
        } else if (Hero.canSee (x, y)) {
            I->p ("The door slams shut on %s!", buf);
            f->mTrapUnknown = 0;
            f->mTrapMonUnknown = 0;
        } else {
            I->p ("You hear a crunching sound.");
            f->mTrapMonUnknown = 0;
        }
        if (c->sufferDamage (&SlammingDoorDamage)) {
            if (c != &Hero && Hero.canSee (c)) {
                I->p ("%s is %s!", buf, c->deathVerb ());
            }
            c->die (kKilled, "a slamming door");
            return 1;
        }
    } else if (shFeature::kPit == f->mType) {   
        if (c->isHero ()) {
            if (c->isFlying ()) {
                if (!f->mTrapUnknown) {
                    I->p ("You fly over a pit.");
                }
                return 0;
            }
            f->mTrapUnknown = 0;
            f->mTrapMonUnknown = 0;
            if (Hero.reflexSave (NULL, 20 + savedcmod)) {
                I->p ("You escape a pit trap.");
                return 0;
            }
            I->p ("You fall into a pit!");
        } else {
            if (c->isFlying ()) {
                return 0;
            }
            if (Hero.canSee (x, y)) {
                I->p ("%s falls into a pit!", buf);
                if (f->mTrapUnknown) {
                    I->drawScreen ();
                    I->pauseXY (x, y);
                }
                f->mTrapUnknown = 0;
                f->mTrapMonUnknown = 0;
            }
        }
        c->mTrapped = NDX (2, 6);
        if (c->sufferDamage (&PitTrapDamage)) {
            if (!c->isHero () && Hero.canSee (c)) {
                I->p ("%s is %s!", buf, c->deathVerb ());
            }
            c->die (kMisc, "Fell into a pit");
            return 1;
        }
    } else if (shFeature::kAcidPit == f->mType) {
        if (c->isHero ()) {
            if (c->isFlying ()) {
                if (!f->mTrapUnknown) {
                    I->p ("You fly over an acid pit.");
                }
                return 0;
            }
            if (Hero.reflexSave (NULL, 20 + savedcmod)) {
                I->p ("You escape an acid pit trap.");
                return 0;
            }
            I->p ("You fall into a pit!");
            I->p ("You land in a pool of acid!");
            f->mTrapUnknown = 0;
            f->mTrapMonUnknown = 0;
        } else {
            if (c->isFlying ()) {
                return 0;
            }
            if (Hero.canSee (x, y)) {
                I->p ("%s falls into a pool of acid!", buf);
                if (f->mTrapUnknown) {
                    I->drawScreen ();
                    I->pauseXY (x, y);
                }
                f->mTrapUnknown = 0;
                f->mTrapMonUnknown = 0;
            }
        }
        c->mTrapped = NDX (2, 6);
        if (c->sufferDamage (&AcidPitTrapDamage)) {
            if (!c->isHero () && Hero.canSee (c)) {
                I->p ("%s is %s!", buf, c->deathVerb ());
            }
            c->die (kMisc, "Dissolved in acid");
            return 1;
        } else {
            c->setTimeOut (TRAPPED, 1000, 0);
        }
    } else if (shFeature::kTrapDoor == f->mType) {
        if (c->isHero ()) {
            if (c->isFlying ()) {
                if (!f->mTrapUnknown) {
                    I->p ("You fly over a trap door.");
                }
                return 0;
            }
            f->mTrapUnknown = 0;
            f->mTrapMonUnknown = 0;
            if (Hero.reflexSave (NULL, 20 + savedcmod)) {
                I->p ("You escape a trap door.");
                return 0;
            }
            I->p ("A trap door opens underneath you!");
            I->drawScreen ();
            I->pauseXY (Hero.mX, Hero.mY);
        } else {
            if (c->isFlying ()) {
                return 0;
            }
            if (Hero.canSee (x, y)) {
                I->p ("A trap door opens underneath %s!", buf);
                f->mTrapUnknown = 0;
                f->mTrapMonUnknown = 0;
            }
        }
        {
            shMapLevel *dest = this;
            for (dest = getLevelBelow ();
                 dest->getLevelBelow () && !RNG (3);
                 dest = dest->getLevelBelow ())
            ;
            return warpCreature (c, dest);
        }
    } else if (shFeature::kHole == f->mType) {
        if (c->isHero ()) {
            if (c->isFlying ()) {
                if (!f->mTrapUnknown) {
                    I->p ("You fly over a hole.");
                }
                return 0;
            }
            if (Hero.reflexSave (NULL, 15 + savedcmod)) {
                I->p ("You avoid a hole.");
                return 0;
            }
            I->p ("You fall into a hole!");
            I->pauseXY (Hero.mX, Hero.mY);
        } else {
            if (c->isFlying ()) {
                return 0;
            }
            if (Hero.canSee (x, y)) {
                I->p ("%s falls through a hole!", buf);
            }
        }
        {
            shMapLevel *dest = this;
            for (dest = getLevelBelow ();
                 dest->getLevelBelow () && !RNG (3);
                 dest = dest->getLevelBelow ())
            ;
            return warpCreature (c, dest);
        }
    } else if (shFeature::kRadTrap == f->mType) {
        if (c->isHero ()) {
            I->p ("You are bathed in a green glow!");
            f->mTrapUnknown = 0;
            f->mTrapMonUnknown = 0;
        } else {
            if (Hero.canSee (x, y)) {
                I->p ("%s is bathed in a green glow!", buf);
                f->mTrapUnknown = 0;
            }
            f->mTrapMonUnknown = 0;
        }
        if (c->sufferDamage (&RadTrapDamage)) {
            c->die (kMisc, "Radiation trap.");
            return 1;
        }
    }
    return 0;
}

/*
  MODIFIES: opens or closes any doors that may have been affected by a 
            creature leaving or entering the square x,y
  RETURNS:  non-zero if the creature at x,y dies
*/

int
shMapLevel::checkDoors (int x, int y)
{
    int tx, ty;

    for (tx = x - 1; tx <= x + 1; tx++) {
        for (ty = y - 1; ty <= y + 1; ty++) {
            shFeature *f = getFeature (tx, ty);
            if (NULL == f) {
                continue;
            } else if (tx == x && ty == y) {
                continue;
            } else {
                if (isOccupied (x, y)) {
                    if (shFeature::kDoorClosed == f->mType && 
                        shFeature::kAutomatic & f->mDoor)
                    {
                        f->mType = shFeature::kDoorOpen;
                        /*
                        if (Hero.canSee (tx, ty)) {
                            //I->p ("The door whooshes open.");
                        } else {
                            if (Hero.mCLevel > 1) {
                                I->p ("You hear a door whoosh open.");
                            } else {
                                I->p ("You hear a whooshing sound.");
                            }
                        }
                        */
                    }
                } 
                else if (shFeature::kDoorOpen == f->mType &&
                         shFeature::kAutomatic & f->mDoor &&
                         0 == countObjects (tx, ty) &&
                         !isOccupied (tx, ty))
                {
                    int ux, uy;
                    for (ux = tx - 1; ux <= tx + 1; ux++) {
                        for (uy = ty - 1; uy <= ty + 1; uy++) {
                            if (!isInBounds (ux, uy)) {
                                continue;
                            }
                            if (isOccupied (ux, uy)) {
                                goto endeffect;
                            }
                        }
                    }
                    f->mType = shFeature::kDoorClosed;
                    if (Hero.canSee (tx, ty)) {
                        //I->p ("The door whooshes shut.");
                    } else {
                        if (Hero.mCLevel > 1) {
                            // I->p ("You hear a door whoosh shut.");
                        } else {
                            // I->p ("You hear a whooshing sound.");
                        }
                    }
                }
            endeffect:
                continue;
            }
        }
    }
    return 0;
}


//RETURNS: 0 on success, non-zero if the creature dies

int
shMapLevel::moveCreature (shCreature *c, int x, int y)
{
    assert (! isOccupied (x, y));

    mCreatures[c->mX][c->mY] = NULL;
    mCreatures[x][y] = c;
    if (checkDoors (c->mX, c->mY)) return 1;
    c->mLastX = c->mX;
    c->mLastY = c->mY;
    c->mLastLevel = this;
    c->mX = x;
    c->mY = y;
    c->mLastMoveTime = Clock;
    if (checkTraps (c->mX, c->mY)) return 1;
    if (checkDoors (x, y)) return 1;
    if (&Hero == c) {
        Hero.newLocation ();    
        Hero.lookAtFloor ();
    }
    return 0;
}

/* RETURNS: 0 on success, 
            1 if the creature dies
            -1 if failed otherwise
*/
int
shMapLevel::putCreature (shCreature *c, int x, int y)
{
    int killed = 0;
    if (isFloor (x, y) && NULL == mCreatures[x][y]) {
        c->mLevel = this;
        c->mX = x;
        c->mY = y;
        c->mLastMoveTime = Clock;
        mCreatures[x][y] = c;
        mCrList.add (c);
        killed = checkDoors (x, y);
        if (killed) return killed;
        if (&Hero == c) {
            Hero.newLocation ();
            Hero.lookAtFloor ();
            if (!(kHeroHasVisited & mFlags)) {
                mFlags |= kHeroHasVisited;
                if (mDLevel != 1) {
                    Hero.earnScore (500);
                }
                if (mCrList.count () < 3) {
                    /* we wouldn't want the Hero to be lonely! */
                    int n = 0;
                    while (n < 10 + mDLevel / 5) {
                        n += spawnMonsters ();
                        if (!RNG (100)) n+= 10; /* in case it's impossible */
                    }
                }
            }
        }
        return 0;
    }
    return -1;
}


shCreature *
shMapLevel::removeCreature (shCreature *c)
{
    int x = c->mX;
    int y = c->mY;
    
    if (-1 == mCrList.remove (c)) {
        /* sometimes called when the creature is not actually on the level */
        return NULL;
    }
    mCreatures[x][y] = NULL;
    checkDoors (x, y);
    c->mLastX = x;
    c->mLastY = y;
    c->mLastLevel = this;
    return c;
}


//MODIFIES: spawns a monster or a group of monsters somewhere on the map
//RETURNS: number of monsters spawned
int
shMapLevel::spawnMonsters ()
{
    shMonsterIlk *ilk;
    shMonster *monster;    
    int x = -1;
    int y = -1;
    int n;
    int res = 0;
//    int difficulty = (mLevel + Hero.mCLevel + 1) / 2;

#if 0
    if (RNG (2)) {
        res = spawnMonsters ();
        if (res > RNG (22)) {
            return res;
        }
    }
#endif

    switch (mMapType) {
    case kTown:
        /* biased towards droids: */
        do {
            ilk = pickAMonsterIlk (RNG (mDLevel));
        } while (!ilk ||
                 !(kBot == ilk->mType ||
                   kDroid == ilk->mType ||
                   !RNG (3)));
        break;
    case kRadiationCave:
        do {
            ilk = pickAMonsterIlk (RNG (mDLevel));
        } while (!ilk ||
                 !(kOoze == ilk->mType ||
                   kAbberation == ilk->mType ||
                   kInsect == ilk->mType ||
                   kMutant == ilk->mType ||
                   kAlien == ilk->mType ||
                   kBeast == ilk->mType ||
                   (kBot == ilk->mType && RNG (2))));
        break;
    case kMainframe:
        do {
            ilk = pickAMonsterIlk (RNG (mDLevel));
        } while (!ilk ||
                 !(kProgram == ilk->mType));
        break;
    case kRabbit:
        return 1;
    case kBunkerRooms:
    default:
        do {
            ilk = pickAMonsterIlk (RNG ((mDLevel + Hero.mCLevel + 1) / 2)); 
        } while (!ilk);
        break;
    }
    n = NDX (ilk->mNumAppearingDice, ilk->mNumAppearingDieSides);
    n = mini (n, mDLevel + Hero.mCLevel);

    do {
        if (0 != findUnoccupiedSquare (&x, &y)) {
            return res;
        }
      /* make sure not to spawn monsters too close to Hero */
    } while (distance (x, y, Hero.mX, Hero.mY) < 40); 

    while (n--) {
        monster = new shMonster (ilk);
        if (0 == putCreature (monster, x, y)) {
            res++;
        } else {
            //FIX: might not have been deleted
            //delete monster;
        }
        if (0 != findNearbyUnoccupiedSquare (&x, &y)) {
            return res;
        }
    }

    return res;
}


void
shMapLevel::removeFeature (shFeature *f)
{
    mFeatures.remove (f);
    delete f;
}


void
shMapLevel::findSuitableStairSquare (int *x, int *y)
{
    while (1) {
    reloop:
        *x = RNG (mColumns);
        *y = RNG (mRows);
        if (isInRoom (*x, *y) && 
            !isInShop (*x, *y) &&
            NULL == getFeature (*x, *y))
        {
            switch (mMapType) {
            case kBunkerRooms:
            {
                shRoom *room = getRoom (*x, *y);
                int i;
                for (i = 0; i < mExits.count (); i++) {
                    shFeature *f = mExits.get (i);
                    if (room == getRoom (f->mX, f->mY)) {
                        goto reloop;
                    }
                }
            }
            default:
                ;
            }
            return;
        }
    }
}


/* connect this level to one below it with a staircase*/

void
shMapLevel::addDownStairs (int x, int y, 
                           shMapLevel *destlev, int destx, int desty)
{
    shFeature *stairsup = new shFeature ();
    shFeature *stairsdown = new shFeature ();
    shFeature *f;

    stairsup->mType = shFeature::kStairsUp;
    stairsdown->mType = shFeature::kStairsDown;
    
    if (-1 == x) {
        findSuitableStairSquare (&x, &y);
    }
    if (-1 == destx) {
        destlev->findSuitableStairSquare (&destx, &desty);
    }

    stairsdown->mX = x;
    stairsdown->mY = y;
    stairsdown->mDest.mX = destx;
    stairsdown->mDest.mY = desty;
    stairsdown->mDest.mLevel = Maze.find (destlev);
    f = getFeature (x, y);
    if (f) {
        removeFeature (f);
    }
    mFeatures.add (stairsdown);
    mExits.add (stairsdown);

    stairsup->mX = destx;
    stairsup->mY = desty;
    stairsup->mDest.mX = x;
    stairsup->mDest.mY = y;
    stairsup->mDest.mLevel = Maze.find (this);
    f = destlev->getFeature (x, y);
    if (f) {
        destlev->removeFeature (f);
    }
    destlev->mFeatures.add (stairsup);
    destlev->mExits.add (stairsup);
}




void
shMapLevel::addDoor (int x, int y, int horiz,
                     int open /* = -1 */, 
                     int lock /* = -1 */,
                     int secret /* = -1 */)
{
    shFeature *door = new shFeature ();

    door->mX = x;
    door->mY = y;
    door->mDoor = 0;

    /* just in case */
    SETSQ (x, y, kStoneFloor);

    if (-1 == lock) {
        lock = !RNG (13);
    }
    if (-1 == secret) {
        secret = !RNG (10);
    }
    if (secret) open = 0;
    if (-1 == open) {
        open = !RNG (4);
    }
    if (open) {
        door->mType = shFeature::kDoorOpen;
    } else if (!secret) {
        door->mType = shFeature::kDoorClosed;
        if (RNG (3) && !lock) {
            door->mDoor |= shFeature::kAutomatic;
            if (!RNG (50)) {
                door->mDoor |= shFeature::kBerserk;
                door->mTrapUnknown = 1;
                door->mTrapMonUnknown = 0;
            }
        }
    } else { /* hidden door */
        door->mType = horiz ? shFeature::kDoorHiddenHoriz 
                            : shFeature::kDoorHiddenVert;
        door->mTrapUnknown = 1;
        door->mTrapMonUnknown = 0;
    }

    if (!door->isOpenDoor () && lock) {
        door->mDoor |= shFeature::kLocked;
        lock = 1;
    }

    switch (RNG (lock ? 4 : 5)) { /* what kind of locking mechanism */
    case 0: door->mDoor |= shFeature::kLockRed; break;
    case 1: door->mDoor |= shFeature::kLockGreen; break;
    case 2: door->mDoor |= shFeature::kLockBlue; break;
    case 3: door->mDoor |= shFeature::kLockOrange; break;
    case 4: break;
    }

    mFeatures.add (door);
}


int
shMapLevel::findSquare (int *x, int *y)
{
    *x = RNG (mColumns);
    *y = RNG (mRows);
    return 0;
}


int
shMapLevel::findUnoccupiedSquare (int *x, int *y)
{
    int attempts = 100;

    *x = RNG (mColumns);
    *y = RNG (mRows);

    while (attempts--) {
        if ((isFloor (*x, *y)) &&
            (0 == isOccupied (*x, *y)) &&
            (0 == isObstacle (*x, *y)))
        {
            shFeature *f = getFeature (*x, *y);
            if (!f || !f->isTrap ()) {
                return 0;
            }
        }
        *x = RNG (mColumns);
        *y = RNG (mRows);
    }
    return -1;
}


int
shMapLevel::findAdjacentUnoccupiedSquare (int *x, int *y)
{
    shDirection dirlist[8] = 
        { kNorth, kSouth, kEast, kWest, 
          kNorthWest, kSouthWest, kNorthEast, kSouthEast };
    int x1, y1;
    int i;

    shuffle (&dirlist[0], 8, sizeof (shDirection));

    for (i = 0; i < 8; i++) {
        x1 = *x; y1 = *y;
        if (moveForward (dirlist[i], &x1, &y1)) {
            if ((isFloor (x1, y1)) &&
                (0 == isOccupied (x1, y1)) &&
                (0 == isObstacle (x1, y1)))
            {
                *x = x1;
                *y = y1;
                return 0;
            }
        }
    }
    return -1;
}


int
shMapLevel::countAdjacentCreatures (int ox, int oy)
{
    int x;
    int y;
    int cnt = 0;

    for (x = ox - 1; x <= ox + 1; x++) {
        for (y = oy - 1; y <= oy + 1; y++) {
            if (x == ox && y == oy) continue;
            if (isInBounds (x, y) && getCreature (x, y)) {
                ++cnt;
            }
        }
    }
    return cnt;
}


int
shMapLevel::findNearbyUnoccupiedSquare (int *x, int *y)
{
    int attempts = 100;

    if (0 == findAdjacentUnoccupiedSquare (x, y)) {
        return 0;
    }
    do {
        *x += RNG (3) - 1;
        *y += RNG (3) - 1;

        if (*x < 0) *x = 1;
        if (*y < 0) *y = 1;
        if (*x >= mColumns) *x = mColumns - 2;
        if (*y >= mRows) *y -= mRows - 2;

        if ((isFloor (*x, *y)) &&
            (0 == isOccupied (*x, *y)) &&
            (0 == isObstacle (*x, *y)))
        {
            return 0;
        }
    } while (attempts--);
    return -1;
}


shMapLevel *
shMapLevel::getLevelBelow ()
{
    int i;
    shVector <shFeature *> v;
    shFeature *f;

    for (i = 0; i < mExits.count (); i++) {
        f = mExits.get (i);
        if (shFeature::kStairsDown == f->mType) {
            v.add (f);
        }
    }
    if (v.count ()) {
        f = v.get (RNG (v.count ()));
        return Maze.get (f->mDest.mLevel);
    } else {
        return NULL;
    }
}
