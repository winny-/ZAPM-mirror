class shMapLevel;

#ifndef MAP_H
#define MAP_H

#define MAPMAXCOLUMNS 200
#define MAPMAXROWS    100
#define MAPMAXROOMS   40

#define TOWNLEVEL 8
#define BUNKERLEVELS 12
#define CAVELEVELS 6

enum shTerrainType {
/* impassable terrain first */
    kStone = 0,
    kVWall,
    kHWall,
    kCaveWall,
    kVirtualWall,
    kNWCorner,
    kNECorner,
    kSWCorner,
    kSECorner,
    kNTee,
    kSTee,
    kWTee,
    kETee,

/* passable terrain */
    kStoneFloor,
    kVirtualFloor,

    kMaxTerrainType = 255
};

#include "Global.h"
#include "Util.h"

#include "Interface.h"
#include "Object.h"
#include <curses.h>

#define leftQuarterTurn(_direction) (shDirection) (((_direction) + 6) % 8)
#define rightQuarterTurn(_direction) (shDirection) (((_direction) + 2) % 8)


#define leftTurn(_direction) (shDirection) (((_direction) + 7) % 8)
#define rightTurn(_direction) (shDirection) (((_direction) + 1) % 8)
#define uTurn(_direction) (shDirection) (((_direction) + 4) % 8)
#define isHorizontal(_direction) (2 == (_direction) % 4)
int isDiagonal (shDirection dir);

shDirection vectorDirection (int x1, int y1, int x2, int y2);
shDirection vectorDirection (int x1, int y1);
shDirection linedUpDirection (int x1, int y1, int x2, int y2);
shDirection linedUpDirection (shCreature *e1, shCreature *e2);

const char *stringDirection (shDirection d);

//RETURNS: distance in feet between points (squares are 5x5)
int distance (int x1, int y1, int x2, int y2);
int distance (shCreature *c1, shCreature *c2);
int distance (shCreature *c1, int x1, int y1);

int rlDistance (int x1, int y1, int x2, int y2);

int areAdjacent (int x1, int y1, int x2, int y2);
int areAdjacent (shCreature *c1, shCreature *c2);

struct shTerrainSymbol
{
    char mGlyph;
    char mColor;
    char *desc;
};


enum shSpecialEffect 
{
    kExplosionEffect,
    kColdEffect,
    kPoisonEffect,
    kInvisibleEffect,
    kRadiationEffect,
};


struct shFeature
{
    enum Type {
        /* impassable features first */
        kDoorHiddenVert,
        kDoorHiddenHoriz,
        kDoorBerserkClosed, /* fake type */
        kDoorClosed,

        /* passable features */

        kStairsUp,
        kStairsDown,
        kVat,
        kComputerTerminal,

        kPit,     /* min trap type */
        kAcidPit,
        kRadTrap,
        kHole,
        kTrapDoor,
        kPortal,  /* max trap type */
        
        kDoorOpen,
        kDoorBerserkOpen, /* fake type */
        kMaxFeatureType
    };

    enum DoorFlags {
        kTrapped = 0x1,
        kAutomatic = 0x2,
        kBerserk = 0x4,
        kLocked = 0x8,
        kLockBroken = 0x10,
        kHoriz = 0x40,
        kLockRed = 0x100,
        kLockGreen = 0x200,
        kLockBlue = 0x400,
        kLockOrange = 0x800,
    };
    

    char mType;
    char mTrapUnknown;    /* is the hero unaware of this trap? */
    char mTrapMonUnknown; /* are the monsters unaware of this trap? */ 
    char mSportingChance; /* bonus for searching, kicking doors down,
                             incremented with each attempt */
    shTime mSpotAttempted;

    int mX;
    int mY;
    union {
        struct {  /* used for stairs, portals */
            short mLevel;
            short mX;
            short mY;
        } mDest;
        short mDoor;
        struct {
            short mHealthy; /* 0 for really gross, higher is cleaner */
            short mRadioactive; 
        } mVat;
    };

    //constructor:
    shFeature ()
    {
        mTrapUnknown = 0;
        mTrapMonUnknown = 0;
        mSportingChance = 0;
        mSpotAttempted = 0;
    }

    int
    isObstacle () {
        return mType <= kDoorClosed;
    }

    int 
    isDoor () { 
        switch (mType) { 
        case kDoorHiddenVert:
        case kDoorHiddenHoriz:
        case kDoorClosed:
        case kDoorOpen:
            return 1;
        default:
            return 0;
        }
    }
    int isOpenDoor () { return kDoorOpen == mType; }
    int isTrappededDoor () { return isDoor () && mDoor & kTrapped; }
    int isAutomaticDoor () { return isDoor () && mDoor & kAutomatic; }
    int isBerserkDoor () { return isDoor () && mDoor & kBerserk; }
    int isLockedDoor () { return isDoor () && mDoor & kLocked; }
    int isLockBrokenDoor () { return isDoor () && mDoor & kLockBroken; }
    int isHorizDoor () { return isDoor () && mDoor & kHoriz; }
    shObjectIlk *keyNeededForDoor () {
        if (!isDoor ()) return NULL;
        if (isLockBrokenDoor ()) return NULL;
        if (mDoor & kLockRed) return findAnIlk (&ToolIlks, "red keycard");
        if (mDoor & kLockGreen) return findAnIlk (&ToolIlks, "green keycard");
        if (mDoor & kLockBlue) return findAnIlk (&ToolIlks, "blue keycard");
        if (mDoor & kLockOrange) return findAnIlk (&ToolIlks, "orange keycard");
        return NULL;
    }
    void lockDoor () { mDoor |= kLocked; }
    void unlockDoor () { mDoor &= ~kLocked; }
  
    int 
    isTrap () 
    {
        return isBerserkDoor () || (mType >= kPit && mType <= kPortal);
    }

    int getDescription (char *buf, int len);
    int 
    the (char *buf, int len)
    {
        snprintf (buf, len, "the ");
        return 4 + getDescription (buf + 4, len - 4);
    }
};


struct shSquare
{
    enum shSquareFlags
    {
        kHallway =      0x1,
        kRadioactive = 0x20,
        kDark =        0x40,
    };

    chtype mTerr;  // shTerrainType
    char mFlags;
    char mRoomId;

    char *the ()
    {
        switch (mTerr) {
        case kStone:
        case kCaveWall:
            return "the cavern wall";
        case kVWall:     case kHWall:     case kNWCorner:
        case kNECorner:  case kSWCorner:  case kSECorner:
        case kNTee:      case kSTee:      case kWTee:
        case kETee:
            return "the wall";
        case kStoneFloor:
            return "the stone floor";
        case kVirtualFloor:
            return "the ground";
        default:
            return "the unknown terrain feature";
        }
    }
};


struct shRoom {
    enum Type {
        kNotRoom = 0,
        kNormal,
        kCavern,
        kGeneralStore,
        kHardwareStore,
        kSoftwareStore,
        kArmorStore,
        kWeaponStore,
        kImplantStore,
        kNest,     /* alien nest */
    };
    
    Type mType;

    shMonster *mShopKeeper;
};


class shMapLevel : shEntity
{
    friend class shInterface;
    friend struct shMonsterSpawnEvent;
    friend class shHero;
    friend struct shHeroUpkeepEvent;

    enum MapFlags {
        kHeroHasVisited = 0x1,
        kHasShop = 0x2,
    };

    enum MapType {
        kBunkerRooms,
        kTown,
        kRabbit,
        kRadiationCave,
        kMainframe,
    };

public:
//private:
    int mDLevel;       // dungeon level (difficulty)
    MapType mMapType;
    char mName[12];
    int mRows;
    int mColumns;
    int mFlags;
    int mNumRooms;

    shRoom mRooms[MAPMAXROOMS]; /* room 0 is the non-room */
    shSquare mSquares[MAPMAXCOLUMNS][MAPMAXROWS];
    chtype mRemembered[MAPMAXCOLUMNS][MAPMAXROWS];
    shObjectVector *mObjects [MAPMAXCOLUMNS][MAPMAXROWS];
    shVector <shCreature *> mCrList;
    shVector <shFeature *> mFeatures;
    shVector <shFeature *> mExits;
    shCreature *mCreatures[MAPMAXCOLUMNS][MAPMAXROWS];
    unsigned char mVisibility[MAPMAXCOLUMNS][MAPMAXROWS];
    
public:
    //constructor
    shMapLevel () { }
    shMapLevel (int level, MapType type);

    void saveState (int fd);
    void loadState (int fd);

    inline shSquare *
    getSquare (int x, int y)
    {
        return &mSquares[x][y];
    }

    inline shFeature *
    getFeature (int x, int y)
    {
        int i;
        shFeature *f;
        for (i = 0; i < mFeatures.count (); i++) {
            f = mFeatures.get (i);
            if (f->mX == x && f->mY == y) {
                return f;
            }
        }
        return NULL;
    }

    
    inline shFeature *
    getKnownFeature (int x, int y)
    {
        shFeature *f = getFeature (x, y);
        return f ? (f->mTrapUnknown ? NULL : f) 
                 : NULL; 
    }

    inline shCreature *
    getCreature (int x, int y)
    {
        return mCreatures[x][y];
    }

    inline char *
    the (char *buf, int len, int x, int y) 
    {
        shFeature *f = getFeature (x, y);
        if (f) {
            f->the (buf, len); 
            return buf;
        } else {
            snprintf (buf, len, "%s", getSquare (x, y) -> the ());
            return buf;
        }
    }


    shCreature *removeCreature (shCreature *c);

    //RETURNS: 1 if the square is still on the map
    int moveForward (shDirection, int *x, int *y, int *z = NULL);

    //RETURNS non-zero iff the square x,y is on the map
    inline int
    isInBounds (int x, int y)
    {
        return x >= 0 && x < mColumns && y >=0 && y < mRows;
    }

    //RETURNS: non-zero iff the square at x,y can be walked upon
    inline int
    isFloor (int x, int y)
    {
        return (kStoneFloor == mSquares[x][y].mTerr ||
                kVirtualFloor == mSquares[x][y].mTerr);
    }

    inline int
    appearsToBeFloor (int x, int y)
    {
        if (isFloor (x, y)) {
            shFeature *f = getFeature (x, y);
            if (f && (shFeature::kDoorHiddenVert == f->mType
                      || shFeature:: kDoorHiddenHoriz == f->mType))
            {
                return 0;
            } else {
                return 1;
            }
        } else {
            return 0;
        }
    }


    shRoom *
    getRoom (int x, int y)
    {
        return &mRooms[mSquares[x][y].mRoomId];
    }

    inline int
    isLit (int x, int y)
    {
        return !(mSquares[x][y].mFlags & shSquare::kDark);
    }


    void setLit (int x, int y, int lit = 1);

    int
    isInDoorWay (int x, int y)
    {
        shFeature *f = getFeature (x, y);
        return f && f->isDoor ();
    }
    

    int
    isInRoom (int x, int y)
    {
        return isFloor (x, y) && !(mSquares[x][y].mFlags & shSquare::kHallway);
    }

    int
    isInShop (int x, int y)
    {
        switch (mRooms[mSquares[x][y].mRoomId].mType) {
        case shRoom::kGeneralStore:
        case shRoom::kHardwareStore:
        case shRoom::kSoftwareStore:
        case shRoom::kArmorStore:       
        case shRoom::kWeaponStore:
        case shRoom::kImplantStore:
            return 1;
        case shRoom::kNotRoom:
        case shRoom::kNormal:
        case shRoom::kCavern:
        case shRoom::kNest:
            return 0;
        }
        return 0;
    }

    shMonster * getShopKeeper (int x, int y);
    shMonster * getGuard (int x, int y);

    //RETURNS: non-zero iff there is a creature on the square x,y
    inline int 
    isOccupied (int x, int y)
    {
        return NULL == mCreatures[x][y] ? 0 : 1;
    }


    //RETURNS: the objects on the square x,y
    inline shObjectVector *
    getObjects (int x, int y)
    {
        return mObjects[x][y];
    }


    inline shObject *
    removeObject (int x, int y, char *ilk)
    {
        shObject *res = findObject (x, y, ilk);
        if (res) {
            if (res->mCount > 1) {
                return res->split (1);
            }
            mObjects[x][y]->remove (res);
            if (0 == mObjects[x][y]->count ()) {
                delete mObjects[x][y];
                mObjects[x][y] = NULL;
            }
        }
        return res;
    }


    //RETURNS: count of objects on the square x,y
    inline int 
    countObjects (int x, int y)
    {
        return NULL == mObjects[x][y] ? 0 : mObjects[x][y]->count ();
    }


    shObject *findObject (int x, int y, char *ilk);

    
    inline void
    setObjects (int x, int y, shObjectVector *v)
    {
        mObjects [x][y] = v;
    }


    //RETURNS: non-zero iff the square at x,y contains impassable terrain
    inline int
    isObstacle (int x, int y)
    {
        shFeature *f = getFeature (x,y);
        if (f && f->isObstacle ()) {
            return 1;
        } else {
            return mSquares[x][y].mTerr <= kETee ? 1 : 0;
        }
    }

    //RETURNS: non-zero iff the square at x,y blocks line of sight
    inline int
    isOcclusive (int x, int y)
    {
        return isObstacle (x, y);
    }

    //RETURNS: non-zero iff the square x,y is in the Hero's LOS.  This does
    //         not take into account lighting, telepathy, blindness, etc.
    inline int
    isInLOS (int x, int y)
    {
        return mVisibility[x][y];
    }
    
    int existsLOS (int x1, int y1, int x2, int y2);

    //RETURNS: true if the square at x,y is radioactive
    inline int isRadioactive (int x, int y) { 
        int i;
        shFeature *f = getFeature (x, y);
        if (f && shFeature::kVat == f->mType 
              && f->mVat.mRadioactive) 
        { 
            return 1;
        }
        for (i = 0; i < mFeatures.count (); i++) {
            f = mFeatures.get (i);
            if (shFeature::kRadTrap == f->mType && 
                areAdjacent (f->mX, f->mY, x, y))
            {
                return 1;
            }
        }
        return mSquares[x][y].mFlags & shSquare::kRadioactive;
    }

    shMapLevel *getLevelBelow ();
    int isBottomLevel () { return NULL == getLevelBelow (); }
    int isTownLevel () { return kTown == mMapType; }

    void setVisible (int x, int y, int value) { mVisibility[x][y] = value; }

    chtype getMemory (int x, int y) { return mRemembered[x][y]; }
    inline void setMemory (int x, int y, chtype c) { 
        mRemembered[x][y] = c; 
    }

    void computeVisibility ();

    int findSquare (int *x, int *y);
    int findUnoccupiedSquare (int *x, int *y);
    int findAdjacentUnoccupiedSquare (int *x, int *y);
    int findNearbyUnoccupiedSquare (int *x, int *y);
    void findSuitableStairSquare (int *x, int *y);
    int countAdjacentCreatures (int ox, int oy);

    int rememberedCreature (int x, int y);

    void reveal ();

    void debugDraw ();
    void draw ();
    void feelSq (int x, int y);
    void drawSq (int x, int y, int forget = 0);
    void drawSqTerrain (int x, int y, int forget = 0, int draw = 1);
    int drawSqCreature (int x, int y);
    void drawSpecialEffect (int x, int y, shSpecialEffect e);


    int areaEffect (shAttack *atk, int x, int y, shDirection dir, 
                     shCreature *attacker);

    int areaEffectFeature (shAttack *atk, int x, int y, shCreature *attacker);
    int areaEffectCreature (shAttack *atk, int x, int y, shCreature *attacker);
    void areaEffectObjects (shAttack *atk, int x, int y, shCreature *attacker);



    int warpCreature (shCreature *c, shMapLevel *newlevel);
    int checkTraps (int x, int y, int savedcmod = 0);
    int checkDoors (int x, int y);
    int moveCreature (shCreature *c, int x, int y);
    int putCreature (shCreature *c, int x, int y);
    int spawnMonsters ();
    
    void makeNoise (int x, int y, int radius);

    int putObject (shObject *obj, int x, int y);

    void removeFeature (shFeature *f);
    void addDoor (int x, int y, int horiz, int open = -1, int lock = -1,
                  int secret = -1);
    void addDownStairs (int x, int y, 
                        shMapLevel *destlev, int destx, int desty);
    
    void addVat (int x, int y);
    shFeature *addTrap (int x, int y, shFeature::Type type);

    static void buildMaze ();    
    
private:

    int isClear (int x1, int y1, int x2, int y2);
    int enoughClearance (int x, int y, shDirection d, int m, int n);
    int fiveByFiveClearance (int x, int y);
    int buildRoomOrElRoom (int sx, int sy, int height);
    int buildTwistyCorridor (int x, int y, shDirection d);
    void buildSnuggledRooms ();
    void buildTwistyRooms ();
    void wallCorridors ();

    void buildDenseLevel ();

    int testSquares (int x1, int y1, int x2, int y2, shTerrainType what);

    void decorateRoom (int sx, int sy, int ex, int ey);
    int makeShop (int sx, int sy, int ex, int ey, int kind = -1);
    int makeNest (int sx, int sy, int ex, int ey);
    void mundaneRoom (int sx, int sy, int ex, int ey);

    int buildBunkerRoom (int sx, int sy, int ex, int ey);
    int buildBunkerRooms ();
    void buildTown ();
    int buildCaveRoom (int x, int y, int size);
    void buildCaveTunnel (int x1, int y1, int x2, int y2);
    int buildCave ();
    void buildRabbitLevel ();
    void buildMainframe ();

    void layCorridor (int x1, int y1, int x2, int y2);
    void layRoom (int x1, int y1, int x2, int y2);
    void fillRect (int sx, int sy, int ex, int ey, shTerrainType t);

};

struct shMonsterSpawnEvent : public shEvent
{
    void fire ();
    void saveState (int fd);

};



#endif
