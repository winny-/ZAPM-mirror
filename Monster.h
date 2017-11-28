class shMonsterType;

#ifndef MONSTER_H
#define MONSTER_H

#include "Util.h"
#include "Creature.h"
#include "Attack.h"
#include "Interface.h"

class shMonster : public shCreature
{
    friend class shMonsterIlk;
public:
    enum Strategy {
        kWander,      /* wander around looking for a fight */
        kDefend,      /* stay put, if Hero approaches, fight but don't pursue */
        kLurk,        /* stay put, if Hero approaches, fight and pursue */
        kPassive,     /* stay put, never do anything */
        kSessile,     /* stay put, attack any adjacent creature */
        kHide,        /* same as sessile, but pursue if hero finds us */
        kHatch,       /* alien egg strategy */
        kChase,       /* pursue Hero */
        kPet,         /* pet strategy */
        kPeaceful,    /* peaceful - wander slowly, maybe get equipment */
        kShopKeep,
        kAngryShopKeep,
        kGuard,
    };

    enum Tactic {
        kNewEnemy,  /* respond to new threat */
        kReady,     /* ready for next tactic */
        kFight,     /* attempt close combat */
        kShoot,     /* attempt ranged combat */
        kMoveTo,    /* move to a location */
    };

    enum Disposition {
        kHelpful,
        kFriendly,
        kIndifferent,
        kUnfriendly,
        kHostile
    };

    int mTame; /* 0 == wild, 1 = loyal */

    int mEnemyX;  /* set if monster is threatened by enemy other than hero */
    int mEnemyY;  /* (right now only used for pets) */
    int mDestX;
    int mDestY;
    union {
        struct {
            int mShopId;
            int mHomeX;  /* coordinates of the "home spot" in front of */
            int mHomeY;  /* the shop door */
            int mBill;   /* amount owed for destroyed items */
        } mShopKeeper;
        struct {
            int mSX;       /* coordinates of guarded rectangle */
            int mSY;
            int mEX;
            int mEY;
            int mToll;     /* toll: -1 for none, 0 for already paid */
            int mChallengeIssued;
        } mGuard;
        struct {
            int mHatchChance;
        } mAlienEgg;
    };
    Strategy mStrategy;
    Tactic mTactic;
    Disposition mDisposition;
    shDirection mPlannedMoves[100];
    shTime mSpellTimer;
    int mPlannedMoveIndex;

    char mPlaceHolder; /* kludge saves me 22 seconds */

    /* constructor */
    shMonster () {}
    shMonster (shMonsterIlk *ilk, int extralevels = 0);

    void saveState (int fd);
    void loadState (int fd);

    void takeTurn ();
    char *the (char *buff, int len);
    char *an (char *buff, int len);
    char *your (char *buff, int len);
    char *getDescription (char *buff, int len);

    int numHands ();
    int getMutantLevel ();

    void followHeroToNewLevel ();

    void makeAngry ();
    void newEnemy (shCreature *c);

    int checkThreats ();
    void findPetGoal ();
    void findTreasure ();

    int likesMoney ();
    int likesWeapons ();
    int needsWeapon ();

    int readyWeapon ();
    int setupPath ();
    int doQuickMoveTo (shDirection dir = kNoDirection);
    int clearObstacle (int x, int y);
    int drinkBeer ();

    enum SquareFlags {
        kHero =       0x1,
        kMonster =    0x2,
        kEnemy =      0x4,
        kLinedUp =    0x10,
        kDoor =       0x100,
        kWall =       0x200,
        kTrap =       0x400,
        kFreeMoney =  0x1000,
        kFreeWeapon = 0x2000,
        kFreeArmor =  0x4000,
        kFreeItem =   0xf000,
    };

    int findSquares (int flags, shCoord *coords, int *info);

    int doAttack (shCreature *target, int *elapsed);
    void doRangedAttack (shAttack *attack, shDirection dir);
    int useMutantPower ();

    int doSessile ();
    int doHide ();
    int doLurk ();
    int doHatch ();
    int doWander ();
    int doPet ();
    int doShopKeep ();
    int doAngryShopKeep ();
    int doGuard ();
    int doRetaliate ();
    
    int doFight ();
    int doMoveTo ();

    int meleeAttack (shObject *weapon, shAttack *attack, int x, int y);

    int isHostile () { return kHostile == mDisposition; }
    int isPet () { return mTame; }
    void makePet ();
    int die (shCauseOfDeath how, char *killer = NULL);

};


struct shMonsterIlk
{
    friend class shMonster;
 public:
    //constructor:
    shMonsterIlk (char *name,
                  shCreatureType ty,
                  struct shMonsterIlk *parent,
                  shThingSize size,
                  int hitdice,
                  int baselevel,
                  int str, int con, int agi, int dex, int in, int wis, int cha,
                  int speed,
                  int numhands,
                  int ac,
                  int numappearingdice,
                  int numappearingdiesides,
                  shMonster::Strategy strategy,
                  int peacefulchance,
                  int probability,
                  char symbol,
                  shColor color);

    void addAttack (shAttack::Type type, int etype, int numdice, int dicesides,
                    int attacktime, int prob = 1,
                    int energy2 = kNoEnergy, int numdice2 = 0, 
                    int dicesides2 = 6);

    void addRangedAttack (shAttack::Type type, shAttack::Effect effect,
                          int etype, int numdice, int dicesides,
                          int attacktime, int range = 10, int prob = 1,
                          int energy2 = kNoEnergy, int numdice2 = 0, 
                          int dicesides2 = 6);

    void addMutantPower (shMutantPower power);

    void addEquipment (char *description);
    void addIntrinsic (shIntrinsics intrinsic);
    void addResistance (shEnergyType energy, int amount);
    void addBlurb (char *blurb);

    void spoilers ();
    //void addSkill ();

// private:
    //representation:
    int mId;
    shCreatureType mType;
    class shMonsterIlk *mParent;  //more general type (e.g. warhorse/horse)
    
    char *mName;

    shThingSize mSize;        // size category
    int mHitDice;
    int mBaseLevel;
    int mMutantLevel;

    int mStr;
    int mCon;
    int mAgi;
    int mDex;
    int mInt;
    int mWis;
    int mCha;

    int mSpeed;   

    int mNumHands;    // number of hands that can wield weapons

    int mNaturalArmorBonus;  
    int mInateIntrinsics;
    int mInateResistances[kMaxEnergyType];

    char mMutantPowers[kMaxMutantPower];
    shVector <shAttack *> mAttacks;
    shVector <shAttack *> mRangedAttacks;
    shVector <char *> mStartingEquipment;

    char mNumAppearingDice;
    char mNumAppearingDieSides;
    int mProbability; // probability

    shGlyph mGlyph;

    shMonster::Strategy mDefaultStrategy;
    shMonster::Disposition mDefaultDisposition;
    int mPeacefulChance;
    char *mBlurb;
};


void initializeMonsters ();


shMonsterIlk *pickAMonsterIlk (int level);
shMonsterIlk *findAMonsterIlk (char *name);
shMonster * generateMonster (int level);

struct shMonsterReadyEvent : public shEvent
{
    shMonster *mMonster;
    
    void fire ();
    void saveState (int fd);
    void loadState (int fd);
};

void monsterSpoilers ();

extern shVector <shMonsterIlk *> MonsterIlks;

#endif








