#ifndef GLOBAL_H
#define GLOBAL_H

/************************************************************************
Proposed version numbering guide (adopted with version 0.3.0):

first number:  a general assessment of how "big" the game is.  This will be 
               set to "1" when the game has achieved a certain level of 
               completeness, and thereafter incremented upon the addition of
               major new features (such as a new dungeon branch?).
second number: incremented with every new release that changes gameplay 
               (i.e. new or changed monster / item / class / etc) and/or
               causes savefile incompatibility with prior versions.
third number:  incremented with bug fix releases that don't affect gameplay
               (*very* trivial changes permitted) or savefile compatibility.

**************************************************************************/

#define ZAPM_VERSION "0.6.3"

#define SH_DEBUG

#ifdef DJGPP
#include <stdarg.h>
#include <stdio.h>
int snprintf (char *str, size_t size, const char *format, ...);
int vsnprintf (char *str, size_t size, const char *format, va_list ap);
#endif


#include <assert.h>
#include "config.h"

typedef signed int shTime;    //ms
#define MAXTIME 99999999 /*so sloppy*/

#define LONGTURN  2000
#define SLOWTURN  1500
#define DIAGTURN  1414
#define FULLTURN  1000
#define HALFTURN  500
#define QUICKTURN 250

struct shInterface;
class shMenu;
class shCreature;
class shMonsterIlk;
class shMonster;
struct shObjectIlk;
struct shObject;
class shHero;
class shMapLevel;
struct shEntity;
struct shThing;
struct shFeature;

struct shCoord { char mX; char mY; };

enum shObjectType
{
    kUninitialized = 0,
    kMoney,
    kImplant,
    kFloppyDisk,
    kCanister,
    kTool,
    kArmor,
    kWeapon,
    kProjectile,
    kFood,
    kDevice,
    kRayGun,
    kMaxObjectType
};

struct shFlags {
    int mVIKeys;
    int mAutopickup;
    int mAutopickupTypes[kMaxObjectType];
};

enum shDirection {
    kNorth = 0,
    kNorthEast = 1,
    kEast = 2,
    kSouthEast = 3,
    kSouth = 4,
    kSouthWest = 5,
    kWest = 6,
    kNorthWest = 7,
    kUp = 8,
    kDown = 9,
    kOrigin = 10,
    kNoDirection = 11
};

extern shTime Clock;         // global clock
extern shTime MonsterClock; 

// kinda lame to include curses here, but now everything needs to 
// know about chtype...

#include <curses.h>

enum shColor {
    kBlack = 0,
    kRed,
    kGreen,
    kYellow,
    kBlue,
    kMagenta,
    kCyan,
    kGray,
    kBrightRed,
    kBrightGreen,
    kBrightYellow,
    kBrightBlue,
    kBrightMagenta,
    kBrightCyan,
    kWhite,
    kDarkRed,
    kDarkGreen,
    kBrown,
    kDarkBlue,
    kDarkMagenta,
    kDarkCyan,
    kDarkGray
};

struct shGlyph {
    chtype mChar;
    attr_t mAttr;
    shColor mForeground;
};


enum shSkillCode {
    kNoSkillCode =        0x0000,

    kSkillAbilityMask =   0x0f00,

    kWeaponSkill =        0x0010,
    kParameterizedSkill = 0x4000,

    kStrSkill =           0x0100,

    kConSkill =           0x0200,
    kConcentration =      0x0201,
    
    kAgiSkill =           0x0300,
    kHide =               0x0303,
    kMoveSilently =       0x0304,

    kMeleeWeapon =        0x0310,
    kSword =              0x0311,
    kUnarmedCombat =      0x0312,

    kDexSkill =           0x0400,
    kOpenLock =           0x0401,
    kRepair =             0x0404,

    kGrenade =            0x0410,
    kHandgun =            0x0411,
    kLightGun =           0x0412,
    kHeavyGun =           0x0413,

    kIntSkill =           0x0500,
    kSearch =             0x0505,
    kHacking =            0x0506,

    kWisSkill =           0x0600,
    kListen =             0x0602,
    kSpot =               0x0605,

    kChaSkill =           0x0700,

    kMutantPower =        0x5001,

    kUninitializedSkill = 0xffff,
};


enum shMutantPower {
    kNoMutantPower = 0x0,
    kIllumination,
    kDigestion,
    kHypnosis,
    kRegeneration,
    kOpticBlast,
    kHaste,
    kTelepathyPower,
    kMentalBlast,
    kPyrokinesis,
    kRestoration,
    kAdrenalineControl,
    kXRayVisionPower,
    kTelekinesis,
    kInvisibility,
    kCharm,
    kTeleport,
/* monster only powers*/
    kHallucinate,
    kTerrify,
    kDarkness,
    kMaxMutantPower
};

const char *getMutantPowerName (shMutantPower id);


#include "Event.h"

extern int GameOver;

extern shHero Hero;
extern shMapLevel *Level;  /* points to the current level */
extern shVector <shMapLevel *> Maze;  /* all the levels */

extern shMonsterIlk *Earthling;

extern shFlags Flags;

#define STRINGIFY(X) #X

#ifndef DATADIR
#define DATADIR "/usr/games/lib/zapmdir/"
#endif

extern char DataDir[256];

#endif
