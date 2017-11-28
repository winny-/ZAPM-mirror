#ifndef ATTACK_H
#define ATTACK_H

#include "Global.h"

enum shEnergyType {
    kNoEnergy,
    kSlashing,
    kPiercing,
    kConcussive,

    kBlinding,
//    kBrainDraining,  /* neuralizer, brain bug */
    kBrainExtracting,/* mi-go */
    kBugging,
    kBurning,
    kChoking,
    kConfusing,      
    kCorrosive,      /* acid */
    kCreditDraining, /* creeping credits :-) */
    kDisarming,
    kDisintegrating, /* antimatter */
    kElectrical,
    kFaceHugging,
    kForce,
    kLaser,
    kFreezing,
    kHealing,
    kHosing,         /* ftp daemon breathe traffic */
    kMagnetic,
    kParalyzing,
    kPoisonous,
    kRadiological,
    kRestoring,
    kPsychic,
    kSickening,      /* viruses */
    kStunning,
    kTimeWarping,
    kTransporting,
    kViolating,
    kWebbing,
    kMaxEnergyType
};

#define IS_SPECIAL_ENERGY(_e) ((_e) > kConcussive)

enum shAttackFlags {
/* low 8 bits reserved for objectilk flags */
    kMelee =           0x100,
    kMissile =         0x200, /* a grenade */
    kAimed =           0x400,
    kTwoHanded =       0x800,
    kFinesse =         0x1000,
    kAmmo =            0x2000,
    kTouchAttack =     0x4000,
    kSecondaryAttack = 0x10000,
    kSelectiveFire =   0x20000,
};

struct shAttack
{
    enum Type {        /* primarily used for descriptive purposes */
        kNoAttack,
        kAcidBath,     /* acid trap */
        kAnalProbe,
        kAcidSplash,   /* acid blood */
        kAttach,       /* e.g. attaching a restraining bolt */
        kBite,
        kBlast,        /* explosion */
        kBullet,
        kBolt,         /* energy bolt, e.g. pea shooter, blaster bolt */
        kBreatheBugs,
        kBreatheFire,
        kBreatheTime,
        kBreatheTraffic,
        kBreatheViruses,
        kClaw,
        kClub,
        kChoke,
        kCook,
        kCrush,
        kCut,
        kDisintegrationRay,
        kExplode,
        kExtractBrain, /* mi-go */
        kFaceHug,      /* facehugger */
        kFlash,
        kFreezeRay,
        kGammaRay,
        kGaussRay,
        kHeadButt,
        kHealingRay,
        kHeatRay,
        kImpact,       /* football, improvised thrown weapon */
        kKick,
        kLaser,
        kLegalThreat,
        kLightningBolt,
        kMentalBlast,
        kPoisonRay,
        kPunch,
        kQuaff,
        kQuill,
        kRail,
        kRestorationRay,
        kShot,         /* shotgun pellets */
        kSlash,
        kSlime,
        kSmash,
        kSpawnPrograms,
        kStab,
        kStasisRay,
        kSting,
        kSword,
        kTailSlap,
        kTouch,
        kTransporterRay,
        kWeb,
        kZap,
        kMaxAttackType
    };


    enum Effect {
        kSingle,    /* single target */
        kCone,      /* a spreading area-effect attack UNIMPLEMENTED */
        kBeam,      /* a straight line area-effect attack */ 
        kSpread,    /* a disc that wraps around walls UNIMPLEMENTED */
        kBurst,     /* a disc that doesn't penetrate walls */
        kOther,
    };

    shObjectIlk *mWeaponIlk; /* weapon, can be NULL  */
    shMutantPower mPower;
    char mType;              /* shAttack::Type */
    char mEffect;            /* shAttack::Effect */
    int mFlags;              /* shWeaponFlags are valid here */
    int mRadius;             /* blast radius in squares */
    int mRange;              /* in squares for beams (+d6 will be added), 
                                o/w in feet */
    short mProb;             /* melee: rel. prob. of monst choosing this attack 
                                ranged: inverse prob of using this attack */
    short mAttackTime;       /* AP spent recovering after attack */
    struct {
        char mEnergy;         /* shEnergyType */
        char mNumDice;        /* number of dice */
        char mDieSides;       /* sides per die */
    } mDamage[2];

    shAttack () {}

    shAttack (shObjectIlk *weaponilk, Type type, Effect effect, int flags, 
              shEnergyType energytype, int numdice, int diesides,
              int attacktime = FULLTURN, int prob = 1,
              int range = 0, 
              shEnergyType energytype2 = kNoEnergy, int numdice2 = 1)
    {
        mWeaponIlk = weaponilk;
        mType = type;
        mEffect = effect;
        mFlags = flags;
        mRange = range;
        mRadius = 1;
        mAttackTime = attacktime;
        mProb = prob;
        mDamage[0].mEnergy = energytype;
        mDamage[0].mNumDice = numdice;
        mDamage[0].mDieSides = diesides;
        mDamage[1].mEnergy = energytype2;
        mDamage[1].mNumDice = numdice2;
        mDamage[1].mDieSides = 6;   /* secondary damage almost always d6*/
    }

    int isSingleAttack () { return kSingle == mEffect; }
    int isAimedAttack () { return kAimed & mFlags; }
    int isMeleeAttack () { return kMelee & mFlags; }
    int isMissileAttack () { return kMissile & mFlags; }
    int isTouchAttack () { return kTouchAttack & mFlags; }
    int isSpecialAttack () { return IS_SPECIAL_ENERGY (mDamage[0].mEnergy); }
    int isSecondaryAttack () { return kSecondaryAttack & mFlags; }

    int getThreatRange (shObject *weapon, shCreature *target); 
    int getCriticalMultiplier (shObject *weapon, shCreature *target);

    int verb2p (char *buf, int len);
    int verb3p (char *buf, int len);
    const char * noun ();

};

extern shAttack ImprovisedObjectAttack;
extern shAttack ImprovisedMissileAttack;
extern shAttack KickedFootballAttack;
extern shAttack GroundCollisionAttack;
extern shAttack PitTrapDamage;
extern shAttack AcidPitTrapDamage;
extern shAttack OpticBlastAttack;
extern shAttack WebAttak;
extern shAttack AcidBloodAttack;
extern shAttack ExplodingMonsterAttack;
extern shAttack BasicExplodingMonsterAttack;

#endif
