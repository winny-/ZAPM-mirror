class shMapLevel;

#ifndef MAP_H
#define MAP_H

#define MAPMAXCOLUMNS 64
#define MAPMAXROWS    20
#define MAPMAXROOMS   80

#define TOWNLEVEL 8
#define BUNKERLEVELS 12
#define CAVELEVELS 6
#define SEWERLEVELS 5
#define CAVEBRANCH 10
#define MAINFRAMELEVELS 4

enum shTerrainType {
/* impassable terrain first */
    kStone = 0,
    kVWall,
    kHWall,
    kCaveWall,
    kSewerWall,
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
    kFloor,
    kCavernFloor,
    kSewerFloor,
    kVirtualFloor,
    kSewage,           /* hiding spot */
    kVoid,

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
#define isVerictal(_direction) (0 == (_direction) % 4)
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
    kNone = 0,
    kInvisibleEffect,
    kExplosionEffect,        /* first bright effect */
    kColdEffect,
    kHeatEffect,
    kPoisonEffect,
    kRadiationEffect,
    kDisintegrationEffect,
    kLaserBeamHorizEffect,
    kLaserBeamVertEffect,
    kLaserBeamFDiagEffect,
    kLaserBeamBDiagEffect,
    kLaserBeamEffect,

    kRailHorizEffect,
    kRailVertEffect,
    kRailFDiagEffect,
    kRailBDiagEffect,
    kRailEffect,

    kBinaryEffect,
    kBugsEffect,
    kVirusesEffect,
};


struct shFeature
{
    enum Type {
        /* impassable features first */
        kDoorHiddenVert,
        kDoorHiddenHoriz,
        kDoorBerserkClosed, /* fake type */
        kDoorClosed,
        kMovingHWall,          /* max occlusive */
        kMachinery,            /* max impassable */

        /* passable features */

        kStairsUp,
        kStairsDown,
        kVat,                  /* min hiding spot */
        kComputerTerminal,     /* max hiding spot */
 
        kPit,     /* min trap type */
        kAcidPit,
        kRadTrap,
        kSewagePit,
        kHole,
        kTrapDoor,
        kWeb,
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
        kMagneticallySealed = 0x20,
        kHoriz = 0x40,
        kLockRetina = 0x80,
        kLockRed = 0x100,
        kLockGreen = 0x200,
        kLockBlue = 0x400,
        kLockOrange = 0x800,
        kAlarmed = 0x1000
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
        struct {
            char mBeginY;
            char mEndY;
            short mRoomId;
        } mMovingHWall;
    };
    //char mHP;

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
        return mType <= kMachinery;
    }


    int
    isOcclusive () {
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
    int isRetinaDoor (){ return isDoor () && mDoor & kLockRetina; }
    int isMagneticallySealed () { return isDoor () 
                                      && mDoor & kMagneticallySealed; }
    int isAlarmedDoor () { return isDoor () && mDoor & kAlarmed; }
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

    int
    isHidingSpot ()
    {
        return (mType >= kVat && mType <= kComputerTerminal);
    }

    const char *getDescription ();
    char * the () {
        char *buf = GetBuf ();
        snprintf (buf, SHBUFLEN, "the %s", getDescription ());
        return buf;
    }
    char *an () {
        char *buf = GetBuf ();
        const char *tmp = getDescription ();
        snprintf (buf, SHBUFLEN, "%s %s", 
                  isvowel (tmp[0]) ? "an" : "a", tmp);
        return buf;
    }

};


struct shSquare
{
    enum shSquareFlags
    {
        kHallway =      0x1,
        kRadioactive =  0x2,
        kStairsOK =     0x4,  /* good place for stairs */
        kNoLanding =    0x8,  /* allow no landing here */

        kDarkNW =      0x10,
        kDarkNE =      0x20,
        kDarkSW  =     0x40,
        kDarkSE =      0x80,
        kDark =        0xf0,
    };

    shTerrainType mTerr;  // shTerrainType
    char mFlags;
    char mRoomId;

    const char *the ()
    {
        switch (mTerr) {
        case kStone:
        case kCaveWall:
            return "the cavern wall";
        case kVWall:     case kHWall:     case kNWCorner:
        case kNECorner:  case kSWCorner:  case kSECorner:
        case kNTee:      case kSTee:      case kWTee:
        case kETee:
        case kSewerWall:
            return "the wall";
        case kFloor:
        case kSewerFloor:
        case kVirtualFloor:
            return "the floor";
        case kCavernFloor:
            return "the cavern floor";
        case kSewage:
            return "the sewage";
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
        kHospital,
        kSewer,
        kGarbageCompactor,
    };
    
    Type mType;

    //shMonster *mShopKeeper;
    
};


class shMapLevel : shEntity
{
    friend struct shInterface;
    friend struct shMonsterSpawnEvent;
    friend class shHero;
    friend struct shHeroUpkeepEvent;

    enum MapFlags {
        kHeroHasVisited = 0x1,
        kHasShop =        0x2,
        kNoTransport =    0x4,
        kNoDig =          0x8,
    };
public:
    enum MapType {
        kBunkerRooms,
        kTown,
        kRabbit,
        kRadiationCave,
        kMainframe,
        kSewer,
        kSewerPlant,
    };

public:
//private:
    int mDLevel;       // dungeon level (difficulty)
    MapType mMapType;
    char mName[12];
    int mRows;
    int mColumns;
    int mType;
    int mFlags;
    int mNumRooms;
    int mCompactorState;
    struct {
        int mCompactor;
    } mTimeOuts;

    shRoom mRooms[MAPMAXROOMS]; /* room 0 is the non-room */
    shSquare mSquares[MAPMAXCOLUMNS][MAPMAXROWS];
    chtype mRemembered[MAPMAXCOLUMNS][MAPMAXROWS];
    shObjectVector *mObjects [MAPMAXCOLUMNS][MAPMAXROWS];
    shVector <shCreature *> mCrList;
    shVector <shFeature *> mFeatures;
    shVector <shFeature *> mExits;
    shCreature *mCreatures[MAPMAXCOLUMNS][MAPMAXROWS];
    unsigned char mVisibility[MAPMAXCOLUMNS][MAPMAXROWS];
    shSpecialEffect mEffects[MAPMAXCOLUMNS][MAPMAXROWS];
    int mNumEffects;
private:
    void reset ();
public:
    //constructor
    shMapLevel () : mCrList (), mFeatures (), mExits () { reset (); }
    shMapLevel (int level, MapType type);

    void saveState (int fd);
    void loadState (int fd);

    inline shSquare *
    getSquare (int x, int y)
    {
        return &mSquares[x][y];
    }

    inline shSpecialEffect 
    getSpecialEffect (int x, int y)
    {
        return mEffects[x][y];
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

    inline const char *
    the (int x, int y) 
    {
        shFeature *f = getFeature (x, y);
        if (f) {
            return f->the (); 
        } else {
            return getSquare (x, y) -> the ();
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
        return (kFloor == mSquares[x][y].mTerr ||
                kCavernFloor == mSquares[x][y].mTerr ||
                kSewerFloor == mSquares[x][y].mTerr ||
                kVirtualFloor == mSquares[x][y].mTerr ||
                kSewage == mSquares[x][y].mTerr);
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

    inline int 
    isWatery (int x, int y)
    {
        return kSewage == mSquares[x][y].mTerr;
    }

    inline int
    isHidingSpot (int x, int y)
    {
        if (kSewage == mSquares[x][y].mTerr) {
            return 1;
        }
        shFeature *f = getFeature (x, y);
        if (f)
            return f->isHidingSpot ();
        return 0;
    }            

    
    inline int
    getRoomID (int x, int y)
    {
        return mSquares[x][y].mRoomId;
    }

    shRoom *
    getRoom (int x, int y)
    {
        return &mRooms[mSquares[x][y].mRoomId];
    }

    /* is the square (x,y) dark from the viewpoint of (vx, vy) 

    */

    inline int
    isLit (int x, int y, int vx, int vy)
    {
        int flag = mSquares[x][y].mFlags & shSquare::kDark;
        int res;
        if (!flag)  //fully lit
            return 1;
        if (shSquare::kDark == flag)  //fully dark
            return 0;

        if (vy<=y) { //N
            if (vx <= x)
                res = !(flag & shSquare::kDarkNW);
            else 
                res = !(flag & shSquare::kDarkNE);
        } else {    // S
            if (vx <= x)
                res = !(flag & shSquare::kDarkSW);
            else 
                res = !(flag & shSquare::kDarkSE);
        }
        if (res) 
            return res;

        /* a door square is tricky.  A partially lit square becomes fully lit
           when its door is opened.  KLUDGE: If the Hero remembers a door is 
           here, then we'll say it's fully lit so that she can see when the 
           door is closed.
        */

        shFeature *f = getKnownFeature (x, y);
        if (f && f->isDoor ()) {
            if (f->isOpenDoor () ||
                '\'' == (getMemory (x, y) & A_CHARTEXT))
            { 
                return 1;
            }
        }
        return 0;
    }


    void setLit (int x, int y, int nw = 1, int ne = 1, int sw = 1, int se = 1);

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
    stairsOK (int x, int y)
    {
        return isFloor (x, y) && (mSquares[x][y].mFlags & shSquare::kStairsOK);
    }


    int
    landingOK (int x, int y)
    {
        return isFloor (x, y) && 
            !(mSquares[x][y].mFlags & shSquare::kNoLanding);
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
        case shRoom::kHospital:
        case shRoom::kSewer:
        case shRoom::kGarbageCompactor:
            return 0;
        }
        return 0;
    }

    int
    isInHospital (int x, int y)
    {
        return shRoom::kHospital == mRooms[mSquares[x][y].mRoomId].mType;
    }

    int
    isInGarbageCompactor (int x, int y)
    {
        return shRoom::kGarbageCompactor == mRooms[mSquares[x][y].mRoomId].mType;
    }


    shMonster * getShopKeeper (int x, int y);
    shMonster * getDoctor (int x, int y);
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
    removeObject (int x, int y, const char *ilk)
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


    shObject *findObject (int x, int y, const char *ilk);

    
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
        shFeature *f = getFeature (x,y);
        if (f && f->isOcclusive ()) {
            return 1;
        } else {
            return mSquares[x][y].mTerr <= kETee ? 1 : 0;
        }
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
    int isMainframe () { return kMainframe == mMapType; }

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
    int findLandingSquare (int *x, int *y);
    
    int rememberedCreature (int x, int y);

    void reveal ();

    void debugDraw ();
    void draw ();
    void feelSq (int x, int y);
    void drawSq (int x, int y, int forget = 0);
    void drawSqTerrain (int x, int y, int forget = 0, int draw = 1);
    int drawSqCreature (int x, int y);
    int drawSqSpecialEffect (int x, int y);

    inline void setSpecialEffect (int x, int y, shSpecialEffect e) {
        mEffects[x][y] = e;
        mNumEffects++;
    }
    void clearSpecialEffects ();
    int effectsInEffect () { return mNumEffects; }
    
    int areaEffect (shAttack *atk, shObject *weapon, int x, int y, 
                    shDirection dir, shCreature *attacker, 
                    int attackmod, int dbonus = 0);

    int areaEffectFeature (shAttack *atk, shObject *weapon, int x, int y, 
                           shCreature *attacker, 
                           int attackmod, int dbonus = 0);
    int areaEffectCreature (shAttack *atk, shObject *weapon, int x, int y, 
                            shCreature *attacker, 
                            int attackmod, int dbonus = 0);
    void areaEffectObjects (shAttack *atk, shObject *weapon, int x, int y, 
                            shCreature *attacker, int dbonus = 0);


    int warpCreature (shCreature *c, shMapLevel *newlevel);
    int checkTraps (int x, int y, int savedcmod = 0);
    int checkDoors (int x, int y);
    int moveCreature (shCreature *c, int x, int y);
    int putCreature (shCreature *c, int x, int y);
    int spawnMonsters ();
    int spawnPrograms (int x, int y, int count);
    
    void makeNoise (int x, int y, int radius);
    void alertMonsters (int x, int y, int radius, int destx, int desty);
    void attractWarpMonsters (int x, int y);
    void doorAlarm (shFeature *door);

    int putObject (shObject *obj, int x, int y);

    void removeFeature (shFeature *f);
    void addDoor (int x, int y, int horiz, int open = -1, int lock = -1,
                  int secret = -1, int alarmed = -1, int magsealed = 0, 
                  int retina = 0);
    void addDownStairs (int x, int y, 
                        shMapLevel *destlev, int destx, int desty);
    
    void addVat (int x, int y);
    void addMuck (int x, int y, shFeature::Type type);
    shFeature *addTrap (int x, int y, shFeature::Type type);
    shFeature *addMovingHWall (int x, int y, int sy, int ey);
    shFeature *addMachinery (int x, int y);

    void magDoors (int action);  /* 1 lock, -1 unlock */
    void moveWalls (int action);
    int pushCreature (shCreature *c, shDirection dir);

    int noTransport () { return mFlags & kNoTransport; }
    int noDig () { return mFlags & kNoDig; }


    static void buildMaze ();    

    const char *getDescription ();
    
private:

    static shMapLevel *buildBranch (MapType type, int depth, int dlevel, 
                                    shMapLevel **end, int ascending = 0);


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
    int makeHospital (int sx, int sy, int ex, int ey);
    int makeGarbageCompactor (int sx, int sy, int ex, int ey);
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
    int buildMainframeRoom (void *user, int col, int row, 
                            int x, int y, int width, int height);
    int buildMainframeJunction (void *user, int col, int row, 
                                int x, int y, int *widths, int *heights);
    void buildMainframeHelper (void *user, int x, int y, int depth);


    void buildSewerPlant ();
    int buildSewer ();
    int buildSewerRoom (void *user, int col, int row);
    void buildSewerHelper (void *user, int x, int y, int depth);
    int floodMuck (int sx, int sy, shTerrainType type, int amount);

    void layCorridor (int x1, int y1, int x2, int y2);
    void layRoom (int x1, int y1, int x2, int y2);
    void fillRect (int sx, int sy, int ex, int ey, shTerrainType t);
    void flagRect (int sx, int sy, int ex, int ey, 
                   shSquare::shSquareFlags flag, int value);
    void setRoomId (int x1, int y1, int x2, int y2, int id);

};

struct shMonsterSpawnEvent : public shEvent
{
    void fire ();
    void saveState (int fd);

};



#endif
