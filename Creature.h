#ifndef CREATURE_H
#define CREATURE_H

#define ABILITY_MODIFIER(score) ((score) / 2 - 5)
#define ABILITY_BONUS(score) ((score) < 12 ? 0 : (score) / 2 - 5)
#define ABILITY_PENALTY(score) ((score) > 9 ? 0 : (score) / 2 - 5)

enum shActionState 
{
    kActing,   /* actively taking turns */
    kWaiting,  /* don't process, on another level */
    kDead,     /* don't process, will be cleaned up soon */
};


enum shGender {
    kFemale =    0x1,
    kMale =      0x2,
};

enum shCreatureType {
    /* non-living */
    kBot,
    kDroid,
    kProgram,
    kConstruct,
    /* living: */
    kEgg,
    kOoze,
    /* living, and has mind: */
    kCyborg,
    kAberration, 
    kAnimal,
    kAlien,
    kBeast,
    kHumanoid,
    kMutant,   /* mutant humanoid */
    kInsect,
    kOutsider,
    kVermin,
};
#define IS_ROBOT(_crtype) (_crtype <= kDroid)
#define IS_PROGRAM(_crtype) (_crtype == kProgram)
#define IS_ALIVE(_crtype) (_crtype > kConstruct)
#define HAS_MIND(_crtype) (_crtype > kOoze)
#define RADIATES(_crtype) (_crtype <= kDroid || \
                           (_crtype >= kCyborg && _crtype != kAlien))


enum shCreatureSymbols {
    kSymHero = '@',
    kSymBot = 'x',
    kSymDroid = 'X',
    kSymHumanoid = 'h',
    kSymCritter = 'c',
    kSymBird = 'B',
    kSymWorm = 'w',
    kSymInsect = 'i',
};


enum shCondition {
    kSpeedy =        0x1,       /* from temporary speed boost item */
/*  kBlind =         0x2,          now an intrinsic */
    kConfused =      0x4,       /* mental blast, canister of beer, etc. */
/*  kDeaf =          0x8,
    kDazzled =       0x10, */
    kHosed =         0x20,      /* traffic */
    kViolated =      0x40,      /* anal probe */
    kGrappled =      0x80,
/*  kHallucinating = 0x100, */
    kSickened =      0x200,     /* virus attack */
    kParalyzed =     0x400,     /* various attacks, hypnosis, etc. */
/*  kPinned =        0x800,
    kProne =         0x1000,*/
    kStunned =       0x2000,    /* stun grenades, etc */
    kAsleep =        0x4000,

/*  kFatigued =      0x10000,
    kExhausted =     0x30000,

    kSatiated =      0x100000,
    kHungry =        0x200000,
    kWeak =          0x600000,
    kFainting =      0xe00000, */

    kFleeing =       0x1000000, /* but not necessarily frightened - for mons */
    kFrightened =    0x2000000,

    kBurdened =      0x10000000,
    kStrained =      0x30000000,
    kOvertaxed =     0x70000000,
    kOverloaded =    0xf0000000
};
const int kEncumbrance = kOverloaded;



#define SKILL_KEY_ABILITY(_code) (((_code) & kSkillAbilityMask) >> 8)


#include "Util.h"
#include "Object.h"
#include "Interface.h"
#include "Profession.h"



int sportingD20 ();


enum shAbilityIndex {
    kStr = 1,
    kCon = 2,
    kAgi = 3,
    kDex = 4,
    kInt = 5,
    kWis = 6,
    kCha = 7
};


struct shAbilities {
    char mStr;
    char mCon;
    char mAgi;
    char mDex;
    char mInt;
    char mWis;
    char mCha;

    char 
    getByIndex (int idx) 
    { 
        switch (idx) {
        case kStr: return mStr;
        case kCon: return mCon;
        case kAgi: return mAgi;
        case kDex: return mDex;
        case kInt: return mInt;
        case kWis: return mWis;
        case kCha: return mCha;
        default: abort (); return -1;
        }
    }

    void setByIndex (int idx, int val);
    void changeByIndex (int idx, int delta);
};


enum shCauseOfDeath {
    kSlain,  /* slain in combat by a monster */
    kKilled, /* killed by something (usually YASD) */
    kAnnihilated,
    kEmbarassment,
    kSuddenDecompression,
    kTransporterAccident,
    kSuicide, 
    kDrowned,
    kBrainJarred,
    kMisc, /* miscellaneous stupid stuff, use killer text */
    kQuitGame,
    kWonGame,
};

/* timeout keys */

enum shTimeOutType {
    ASLEEP,
    BLINDED,
    CONFUSED,
    FLEEING,
    FRIGHTENED,
    HOSED,
    PARALYZED,
    SICKENED,
    SPEEDY,
    STUNNED,
    TELEPATHY,
    TRAPPED,
    VIOLATED,
    XRAYVISION,
};

class shCreature : public shThing
{
    friend class Interface;
 public:

    int mX;
    int mY;
    int mZ; /* -1, 0, 1  for in pit, on ground, flying */
    class shMapLevel *mLevel;
    shMonsterIlk *mIlk;

    struct TimeOut {
        shTimeOutType mKey;
        shTime mWhen;
        
        TimeOut () { };
        TimeOut (shTimeOutType key, shTime when) {
            mKey = key;
            mWhen = when;
        }
    };

    shCreatureType mType;
    shProfession *mProfession;

    char mName[30]; 
    int mGender;

    int mAP;        /* action points */
    
    int mCLevel;    // character level

    int mAC;        // armor class
    int mConcealment;

    int mBAB;       // base attack bonus

    int mHP;        // hit points
    int mMaxHP;
    shTime mLastRegen; //time of last HP regen

    int mNaturalArmorBonus;
    shAbilities mAbil;;
    shAbilities mMaxAbil;
    shAbilities mExtraAbil; // component of maxabil from items
    int mChaDrain;  // psionic drain to charisma

    int mSpeed;
    int mSpeedBoost; /* e.g. psionic speed boost */
        
    char mReflexSaveBonus; /* when dodging area effects */
    char mWillSaveBonus;   /* when saving against psychic attacks */

    /* resistances represented as damage reduction */
    char mInateResistances[kMaxEnergyType]; /* permanent */
    char mResistances[kMaxEnergyType];      /* inate + that from equip */

    int mFeats;
    char mMutantPowers[kMaxMutantPower];
    int mHidden;             /* hide score (negative iff Hero has spotted, but
                                monster doesn't know) */
    enum { kNothing, kObject, kFeature, kMonster } mMimic;
    shTime mSpotAttempted;   /* last time hero tried to spot creature */
    int mTrapped;            /* special trap timeout */
    int mDrowning;           /* air left in lungs */
    int mInateIntrinsics;    /* permanent + those from mutant powers */
    int mIntrinsics;         /* inate + those gained from equipment/implants */
    int mBuggyIntrinsics;    /* TODO: gained from buggy equipment/implants */
    int mConditions;
    short mRad;              /* radiation exposure */
    int mCarryingCapacity;   /* all weights are in grams */
    short mPsiModifier;      /* effect of equipment on psionic attacks */
    short mToHitModifier;    /* effect of equipment on BAB */
    short mDamageModifier;   /* effect of eqiupment on damage rolls */

    int mWeight;    

    shTime mLastMoveTime;    /* time of last movement */
    shDirection mDir;        /* direction of current movement */
    int mLastX;              /* previous x,y coordinates */
    int mLastY;
    shActionState mState;
    shGlyph mGlyph;
#define TRACKLEN 5
    shCoord mTrack[TRACKLEN]; /* mTrack[0] == our last position */


    shObject *mWeapon;       /* must be the first ptr field (see save ()) */

    shObject *mJumpsuit;     /* worn under body armor */
    shObject *mBodyArmor;
    shObject *mHelmet;
    shObject *mBoots;
    shObject *mGoggles;
    shObject *mBelt; 

    shObject *mImplants[shImplantIlk::kMaxSite];
    
    union {
        shObjectIlk *mMimickedObject;
        shFeature::Type mMimickedFeature;
        shMonsterIlk *mMimickedMonster;
    };

    shVector <TimeOut *> mTimeOuts;
    shVector <shSkill *> mSkills;
    shObjectVector *mInventory;
    shMapLevel *mLastLevel;  /* previous level */

    
    //constructor:
    shCreature ();
    //destructor:
    virtual ~shCreature ();

    void saveState (int fd);
    void loadState (int fd);

    int isA (const char *ilk);

    virtual int isHero () { return 0; }
    virtual const char *the () = 0;
    virtual const char *an () = 0;
    virtual const char *your () = 0;
    virtual const char *getDescription () = 0;
    virtual const char *herself ();

    virtual int interrupt () { return 0; }

    virtual const char *her (const char *thing);

    int reflexSave (shAttack *attack, int DC);
    int willSave (int DC);

    int reflectAttack (shAttack *attack, shDirection *dir);
    //RETURNS: 1 if attack kills us; o/w 0
    int sufferAbilityDamage (shAbilityIndex idx, int amount,
                             int ispermanent = 0);

    //RETURNS: 1 if attack kills us; o/w 0
    int sufferDamage (shAttack *attack, shCreature *attacker = NULL,
                      int bonus = 0, int multiplier = 1, int divider = 1);
    //RETURNS: 1 if the creature really dies; o/w 0
    virtual int die (shCauseOfDeath how, const char *killer = NULL);
    virtual int die (shCauseOfDeath how, shCreature *killer) {
        return die (how, (char *) NULL);
    }
    void pDeathMessage (const char *monname, shCauseOfDeath how, 
                        int presenttense = 0);

    virtual int checkRadiation ();

    virtual shTime setTimeOut (shTimeOutType key, shTime howlong, 
                               int additive = 1);
    virtual TimeOut *getTimeOut (shTimeOutType key);
    inline void clearTimeOut (shTimeOutType key) { setTimeOut (key, 0, 0); }
    int checkTimeOuts ();


    //RETURNS: 1 if successful
    virtual int addObjectToInventory (shObject *obj, int quiet = 0);
    void removeObjectFromInventory (shObject *obj);
    shObject *removeOneObjectFromInventory (shObject *obj);
    shObject *removeSomeObjectsFromInventory (shObject *obj, int cnt);
    void useUpOneObjectFromInventory (shObject *obj);
    //RETURNS: number of objects used
    int useUpSomeObjectsFromInventory (shObject *obj, int cnt);
    int owns (shObject *obj);
    
    //RETURNS: 1 if successful, 0 o/w
    virtual int wield (shObject *obj, int quiet = 0);
    //RETURNS: 1 if successful, 0 o/w
    virtual int unwield (shObject *obj, int quiet = 0);
    int expendAmmo (shObject *weapon, int cnt = 0);
    int hasAmmo (shObject *weapon);

    inline int isAlive () { return IS_ALIVE (mType); }
    inline int isProgram () { return IS_PROGRAM (mType); }
    inline int isRobot () { return kBot == mType || kDroid == mType; }
    inline int isInsect () { return kInsect == mType; }
    inline int radiates () { return RADIATES (mType); }
    
    virtual int isHostile () { return 0; }

    virtual int don (shObject *obj, int quiet = 0);
    virtual int doff (shObject *obj, int quiet = 0);
    void damageEquipment (shAttack *atk, shEnergyType energy);
    int transport (int x, int y, int safe);

    virtual int useMutantPower () = 0;


    //RETURNS: 1 if successful, 0 o/w
    int openDoor (int x, int y);
    int closeDoor (int x, int y);

    void shootLock (shObject *weapon, shAttack *attack, shFeature *door, 
                    int attackmod);

    virtual int numHands () { return 0; }

/* TODO: check for welded 2-handed weapon... */
    int hasFreeHand () { return 1; } 
    
    virtual int getStr () { return mAbil.mStr; }
    virtual int getCon () { return mAbil.mCon; }
    virtual int getAgi () { return mAbil.mAgi; }
    virtual int getDex () { return mAbil.mDex; }
    virtual int getInt () { return mAbil.mInt; }
    virtual int getWis () { return mAbil.mWis; }
    virtual int getCha () { return mAbil.mCha; }

    int getPsionicDC (int powerlevel);

    void computeIntrinsics ();
    void computeAC ();
    int sewerSmells ();

    int getAC (int flatfooted = 0, shCreature *attacker = NULL) 
    { 
        return flatfooted ? mAC - ABILITY_BONUS (getAgi ()) : mAC;
    }

    int getTouchAC (int flatfooted = 0, shCreature *attacker = NULL) 
    {
        return flatfooted ? 10 + ABILITY_PENALTY (getAgi ()) 
                          : 10 + ABILITY_MODIFIER (getAgi ());
    }

    shThingSize getSize ();

    int isImmuneToCriticalHits () { return isHero (); }
    int getResistance (shEnergyType etype) { 
        return mResistances[etype] > 100 ? 1000 : mResistances[etype];
    }
    void addSkill (shSkillCode c, int access, shMutantPower power = kNoMutantPower);
    shSkill *getSkill (shSkillCode c, shMutantPower power = kNoMutantPower); 

    int getSkillModifier (shSkillCode c, shMutantPower power = kNoMutantPower);
    int exerciseSkill (shSkillCode c, int amt, shMutantPower power = kNoMutantPower);
    int gainRank (shSkillCode c, int howmany = 1, 
                  shMutantPower power = kNoMutantPower);

    int getWeaponSkillModifier (shObjectIlk *ilk);

    int exerciseWeaponSkill (shObject *weapon = NULL, int amt = 1)
    {
        if (NULL == weapon) {
            return exerciseSkill (kUnarmedCombat, amt);
        } else if (weapon->isA (kWeapon)) {
            return exerciseSkill ( ((shWeaponIlk *)weapon->mIlk) -> mSkill, amt);
        } else { /* right now there is no improvised weapon skill */
            return 0;
        }
    }

    void levelUp ();

    int countEnergy (int *tankamount = NULL);
    int loseEnergy (int amount);
    void gainEnergy (int amount);

    int countMoney ();
    int loseMoney (int amount);
    int gainMoney (int amount);

    /* conditions */

    int getEncumbrance () { return mConditions & kEncumbrance; }
    int isGrappling (shCreature *target = NULL) {
        return mConditions & kGrappled;
    }

    int hasMind () { return HAS_MIND (mType) || isA ("brain mold"); }
    int isMoving ();
    virtual int isPet () { return 0; }

    int hasTelepathy () { return mIntrinsics & kTelepathy; }
    int hasMotionDetection () { return mIntrinsics & kMotionDetection; }
    int hasRadiationDetection () { return mIntrinsics & kRadiationDetection; }
    int hasScent () { return mIntrinsics & kScent; }
    int hasXRayVision () { return mIntrinsics & kXRayVision; }
    int hasReflection () { return mIntrinsics & kReflection; }
    int hasAutoSearching () { return mIntrinsics & kAutoSearching; }
    int hasTranslation () { return mIntrinsics & kTranslation; }
    int hasHealthMonitoring () { return mIntrinsics & kHealthMonitoring; }
    int hasRadiationProcessing () { return mIntrinsics & kRadiationProcessing;}
    int hasCrazyIvan () { return mIntrinsics & kCrazyIvan; }
    int hasAutoRegeneration () { return mIntrinsics & kAutoRegeneration; }
    int hasLightSource () { return mIntrinsics & kLightSource; }
    int hasPerilSensing () { return mIntrinsics & kPerilSensing; }
    int hasShield () { return mIntrinsics & kShielded; }
    int hasBugSensing () { return mIntrinsics & kBugSensing; }
    int isFlying () { return mIntrinsics & kFlying; }
    int isBlind () { return mIntrinsics & kBlind; }
    int isLucky () { return mIntrinsics & kLucky; }
    int hasNarcolepsy () { return mIntrinsics & kNarcolepsy; }
    int hasBrainShield () { return mIntrinsics & kBrainShielded; }
    int isScary () { return mIntrinsics & kScary; }
    int hasAcidBlood () { return mIntrinsics & kAcidBlood; }
    int isMultiplier () { return mIntrinsics & kMultiplier; }
    int hasAirSupply () { return mIntrinsics & kAirSupply; }
    int isBreathless () { return mIntrinsics & kBreathless; }
    int canSwim () { return mIntrinsics & kCanSwim; }
    int hasNightVision () { return mIntrinsics & kNightVision; }
    int canHideUnder (shObject *o) { return mFeats & kHideUnderObject 
                                         && o->mIlk->mSize > getSize (); }
    int canHideUnder (shFeature *f) { return mFeats & kHideUnderObject 
                                          && f->isHidingSpot (); } 
    int canHideUnderObjects () { return mFeats & kHideUnderObject; }
    int canHideUnderWater () { return mFeats & kHideUnderObject; }
    int canMimicMoney () { return mFeats & kMimicMoney; }
    int canMimicObjects () { return mFeats & kMimicObject; }
    int isExplosive () { return mFeats & kExplosive; }
    int isSessile () { return mFeats & kSessile; }
    int noTreasure () { return mFeats & kNoTreasure; }
    int isUnique () { return mFeats & kUniqueMonster; }
    int isWarpish () { return mFeats & kWarpish; }
    int isLawyer () { return isA ("lawyer"); }
    int isMonolith () { return isA ("monolith"); }

    void checkConcentration ();

    void setBlind () { mIntrinsics |= kBlind; }
    void resetBlind () { mIntrinsics &= ~kBlind; }
    void makeBlinded (int howlong);
    void resetBlinded ();
    int isConfused () { return mConditions & kConfused; }
    void makeConfused (int howlong);
    void resetConfused ();
    int isParalyzed () { return mConditions & kParalyzed; }
    void makeParalyzed (int howlong);
    int isSpeedy () { return mConditions & kSpeedy; }
    void makeSpeedy (int howlong);
    void resetSpeedy ();
    int isHosed () { return mConditions & kHosed; }
    void makeHosed (int howlong);
    void resetHosed ();
    int isSickened () { return mConditions & kSickened; }
    void makeSickened (int howlong);
    void resetSickened ();
    int isStunned () { return mConditions & kStunned; }
    void makeStunned (int howlong);
    void resetStunned ();
    int isAsleep () { return mConditions & kAsleep; }
    void makeAsleep (int howlong);
    void wakeUp ();
    int isViolated () { return mConditions & kViolated; }
    void makeViolated (int howlong);
    void resetViolated ();
    int isTrapped () { return mTrapped; }
    int isUnderwater () { 
        if (mZ >= 0 || !mLevel) return 0;
        shFeature *f = mLevel->getFeature (mX, mY);
        if (f && shFeature::kSewagePit == f->mType) return 1;
        return 0;
    }
    void makeFleeing (int howlong);
    void resetFleeing ();
    int isFleeing () { return mConditions & kFleeing; }
    void makeFrightened (int howlong);
    void resetFrightened ();
    int isFrightened () { return mConditions & kFrightened; }

    void sterilize ();
    void revealSelf ();

    int isFlankedBy (shCreature *creature = NULL) { return 0; }
    int isProne () { return 0; }
    int isSitting () { return 0; }
    int isAdjacent (int x, int y) { return areAdjacent (mX, mY, x, y); }
    int isInShop () { return mLevel->isInShop (mX, mY); }

    virtual int isSurprisedBy (shCreature *creature = NULL) { return 0; }

    int canSmell (shCreature *c);

    //RETURNS:  0 for completely concealed target, 100 for visible target
    //MODIFIES: if cover is non-null, sets cover on scale of 0 to 100
    int canSee (int x, int y, int *cover = NULL) {
        if (NULL != cover) {
            *cover = 0;
        }
        if (isBlind ()) { 
            return (x == mX && y == mY) ? 100 : 0;
        } else if (isHero ()) {
            if (areAdjacent (x, y, mX, mY)) {
                return 100;
            } else if (mLevel->isInLOS (x, y)) {
                if (mLevel->isLit (x, y, mX, mY) || 
                    (hasLightSource () && distance (this, x, y) <= 25))
                {
                    return 100;
                }
            } else if (mLevel->isLit (x, y, mX, mY) &&
                       hasXRayVision () && distance (this, x, y) < 25)
            {
                return 100;
            }
        } else {
            return 100;
        }
        return 0;
    }
    

    int canSee (shCreature *c, int *cover = NULL) {
        if (NULL != cover) {
            *cover = 0;
        }
        if (isBlind ()) { 
            return (c->mX == mX && c->mY == mY) ? 100 : 0;
        } else if (c->isHero ()) {
            if (areAdjacent (mX, mY, c->mX, c->mY))
                return 100;
            if (Level->isInLOS (mX, mY) ||
                (hasXRayVision () && distance (this, c->mX, c->mY) < 25))
            {
                return 100;
            }
        } else if (isHero ()) {
            if (areAdjacent (mX, mY, c->mX, c->mY))
                return 100;
            if (mLevel->isInLOS (c->mX, c->mY)) {
                if (mLevel->isLit (c->mX, c->mY, mX, mY) || 
                    (hasLightSource () && 
                     distance (this, c->mX, c->mY) <= 25) || 
                    (hasNightVision () && c->radiates () &&
                     distance (this, c->mX, c->mY) <= 100))
                {
                    return 100;
                }
            } else if (mLevel->isLit (c->mX, c->mY, mX, mY) &&
                       hasXRayVision () && distance (this, c->mX, c->mY) < 25)
            {
                return 100;
            }
        } else {
            return canSee (c->mX, c->mY, cover);
        }
        return 0;
    }


    int shortestPath (int ox, int oy, int dx, int dy,
                      shDirection *resultbuf, int buflen);
    int estimateDistance (int ox, int oy, int dx, int dy);
    int moveCost (int ox, int oy, int dx, int dy);

    int hpWarningThreshold () { return mini (mMaxHP-1, maxi (mMaxHP/6, 5)); }
    
    virtual void newEnemy (shCreature *c) { }

    virtual int doMove (shDirection dir);

    /* fighting functions; see Fight.cpp */

    int attackRoll (shAttack *attack, shObject *weapon, int attackmod, int AC,
                    shCreature *target);
    int rangedAttackHits (shAttack *attack, shObject *weapon, int attackmod,
                          shCreature *target, int *dbonus);
    int resolveRangedAttack (shAttack *attack, shObject *weapon, 
                             int attackmod, shCreature *target);
    int shootWeapon (shObject *weapon, shDirection dir);
    int resolveMeleeAttack (shAttack *attack, shObject *weapon, 
                            shCreature *target);
    void projectile (shObject *obj, int x, int y, shDirection dir, 
                     shAttack *attack, int range);
    int throwObject (shObject *obj, shDirection dir);

    /* can effects */
    int healing (int hpmaxdice);
    int fullHealing (int hpmaxdice);

    /* mutant powers */
    int telepathy (int on);
    int digestion (shObject *obj);
    int opticBlast (shDirection dir);
    int shootWeb (shDirection dir);
    int hypnosis (shDirection dir);
    int xRayVision (int on);
    int mentalBlast (int x, int y);
    int regeneration ();
    int pyrokinesis (int x, int y);
    int restoration ();
    int charm ();
    int adrenalineControl (int on);
    int haste (int on);
    int teleportation ();
    int telekinesis ();
    int invisibility (int on);
    int ceaseAndDesist ();

    /* initialization crap */

    void rollAbilityScores (int strbas, int conbas, int agibas, int dexbas, 
                            int intbas, int wisbas, int chabas);
    int statsViability ();
    void rollHitPoints (int hitdice, int diesize);

    virtual void takeTurn () = 0;
};



#endif

