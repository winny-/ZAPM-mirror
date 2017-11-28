#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include <stdlib.h>

#include "Attack.h"


/* Various types of objects in the game.
   ObjectType is the most general classification of an object,
   ObjectIlk is a more specific subclassification.
*/

extern shGlyph ObjectGlyphs[];

enum shThingSize
{                 /* creature size */
    kFine,        /* 6 in or less */
    kDiminutive,  
    kTiny,
    kSmall,
    kMedium,
    kLarge,
    kHuge,
    kGigantic,
    kColossal
};

enum shMaterialType  //borrowed from Nethack
{
    kLiquid,
    kWax,
    kVegetable,
    kFleshy,
    kPaper,
    kCloth,
    kPlastic,
    kLeather,
    kWood,
    kBone,
    kIron,
    kBrass,
    kTin,
    kBronze,
    kLead,
    kSteel,
    kAluminum,
    kTitanium,
    kCarbonFiber,
    kPlasteel,
    kAdamantium,
    kEndurium,
    kDepletedUranium,
    kCopper,
    kSilver,
    kElectrum,
    kGold,
    kPlatinum,
    kMithril,
    kSilicon,
    kGlass,
    kCrystal,
    kGem,
    kMineral,
    kRock,
};


enum shIntrinsics {
    kTelepathy =           0x1,
    kMotionDetection =     0x2,
    kRadiationDetection =  0x4,
    kScent =               0x8,
    kXRayVision =          0x10,
    kReflection =          0x20,
    kAutoSearching =       0x40,
    kTranslation =         0x80,  /* babel fish */
    kHealthMonitoring =    0x100,
    kRadiationProcessing = 0x200,
    kCrazyIvan =           0x400,
    kAutoRegeneration =    0x800,
    kLightSource =         0x1000,
    kPerilSensing =        0x2000,
    kBlind =               0x4000,
    kShielded =            0x8000, /* protected by a shield generator */
    kBugSensing =          0x10000, /* software engineer */
    kFlying =              0x20000,
    kLucky =               0x40000,
    kNarcolepsy =          0x80000,
    kBrainShielded =       0x100000,
    kScary =               0x200000,
    kAcidBlood =           0x400000,
    kMultiplier =          0x800000,
    kAirSupply =           0x1000000, /* has air supply */
    kBreathless =          0x2000000, /* doesn't need to breathe */
    kCanSwim =             0x4000000, /* can swim */
    kNightVision =         0x8000000,
};


/* low 8 bits reserved for objectilk, high 24 bits for derived ilks */
enum shObjectIlkFlags {
    kMergeable = 0x1,
    kChargeable = 0x2,
    kIdentified = 0x4,
    kEnhanceable = 0x8,
    kBugProof = 0x10,
    kUsuallyBuggy = 0x20,
    kUnique = 0x40,
    kIndestructible = 0x80,
};


struct shObjectDescData {
    char mDesc[40];
    shColor mColor;
};

extern shObjectDescData CanisterData[30];
extern shObjectDescData ImplantData[30];
extern shObjectDescData FloppyData[30];
extern shObjectDescData RayGunData[30];
extern shObjectDescData JumpsuitData[30];
extern shObjectDescData BeltData[5];

void randomizeIlkNames ();

extern shVector <shObjectIlk *> ObjectIlks;

struct shObjectIlk
{
    int mId;
    shObjectType mType;
    struct shObjectIlk  *mParent;  // parent ilk, useful for isA ()
                                    // e.g. bullet is parent of silver bullet
    
    const char *mName;       /* precise name, e.g. "canister of restoration */
    const char *mAppearance; /* ignorant description, e.g. "green canister" */
    const char *mVagueName;  /* blind name, e.g. "canister" */
    const char *mUserName;   /* name given by user, e.g.: "tastes like crap" */
    shGlyph mGlyph;

    shMaterialType mMaterial;

    int mCost;         /* cost in shop */

    int mFlags;
    int mCarriedIntrinsics; /* conferred to the carrier */
    int mWornIntrinsics;    /* conferred only when worn */
    int mWieldedIntrinsics; /* conferred only when wielded */
    int mActiveIntrinsics;  /* conferred only when active */

    int mProbability;  /* relative probability when randomly generating; 
                          -1 indicates an abstract ilk */

    int mWeight;       // in grams
    shThingSize mSize;// physical size / unwieldiness.
    int mHardness;
    int mHP;           // base HP
    int mEnergyUse;    /* for items that continuously use energy (e.g. geiger 
                          counters, flashlights), ticks per cell consumed
                          for items that use energy in bursts (e.g. weapons), 
                          cells consumed per use */
    
    //constructor:
    shObjectIlk () {
        mId = ObjectIlks.add (this);
        mFlags = 0; mCarriedIntrinsics = 0; mWornIntrinsics = 0;
        mWieldedIntrinsics = 0; mActiveIntrinsics = 0;
        mVagueName = NULL; mUserName = NULL;
    }

    int isA (shObjectIlk *ilk);

    char *getRayGunColor (); /* kludgey, not reentrant*/

};


struct shWeaponIlk : shObjectIlk
{
    int mRange;   // in ft, 0 indicates melee weapon
    int mThreatRange;
    int mToHitBonus;
    int mCriticalMultiplier;
    shObjectIlk *mAmmoType; 
    int mAmmoBurst; /* no of rounds of ammo /energy cells consumed per burst */
    shSkillCode mSkill;

    shAttack mAttack;

    //constructor:
    shWeaponIlk (const char *name, const char *vaguename, 
                 const char *appearance, 
                 shColor color, 
                 shSkillCode skill,
                 shWeaponIlk *parent,
                 shMaterialType material, int flags, int weight,
                 shThingSize size, int hardness, int hp, 
                 shObjectIlk *ammo, int ammoburst,
                 int hitbonus,
                 shAttack::Type atktype, shAttack::Effect atkeffect,
                 int range, int radius, shEnergyType entype,
                 int numdice, int dicesides, int atktime, 
                 shEnergyType entype2, int numdice2, 
                 int cost, int prob);
};

struct shRayGunIlk : shObjectIlk
{
    shAttack mAttack;
    shRayGunIlk (const char *name, const char *appearance, shColor color,
                 int flags,
                 shAttack::Type atktype, shEnergyType entype, 
                 int numdice, int dicesides, int cost, int prob);
};


struct shImplantIlk : shObjectIlk
{
    enum Site {
        kFrontalLobe,
        kParietalLobe,
        kOccipitalLobe,
        kTemporalLobe,
        kCerebellum,
        kLeftEar,
        kRightEyeball,

        kMaxSite,
        kAnyEar,
        kAnyEye,
        kAnyBrain,
    };

    Site mSite;
    int mPsiModifier; 
    
    shImplantIlk (const char *name, const char *vaguename, 
                  const char *appearance, 
                  shColor color, 
                  shMaterialType material, int flags, 
                  Site site, int intrinsics,
                  int psimodifier,
                  int cost, int prob);
};

const char *describeImplantSite (shImplantIlk::Site site);

enum shArmorFlags {
    kPowered = 0x100, /* the weight of worn powered armor doesn't encumber */
};


struct shArmorIlk : shObjectIlk
{
    char mArmorBonus;
    char mResistances[kMaxEnergyType]; /* resistances granted by this armor */

    int mEquipSpeed;  //ms to don or doff armor
    char mSpeedPenalty; //in % - 100% penalty == movement takes twice as long
    int mPsiModifier; 
    int mToHitModifier;
    int mDamageModifier;

    //constructor:
    shArmorIlk (const char *name, const char *vaguename, 
                const char *appearance, 
                shColor color, 
                shArmorIlk *parent,
                shMaterialType material, int flags, int weight,
                shThingSize size, int hardness, int hp, int psimod, int bonus,
                shEnergyType specialtype, int specialbonus,
                int equipspeed, int speedpenalty, int cost, int prob);

};


struct shDeviceIlk : shObjectIlk
{
    shMonsterIlk *mWreckIlk;

    shDeviceIlk (const char *name, const char *vaguename, 
                 const char *appearance,
                 shColor color, shDeviceIlk *parent,
                 int cost, shMaterialType material, int flags, int weight,
                 shThingSize size, int hardness, int hp,
                 int prov);
};


typedef int shToolFunc (shObject *tool); /* returns ms elapsed */

struct shToolIlk : shObjectIlk
{
    shToolFunc *mUseFunc;

    shToolIlk (const char *name, const char *vaguename, 
               const char *appearance, 
               shColor color, shToolIlk *parent,
               int cost, shMaterialType material, int flags, int weight, 
               shThingSize size, int hardness, int hp, 
               int energyuse, shToolFunc *usefunc,
               int prob);
};

extern shToolIlk *EnergyTank;
extern shToolIlk *PowerPlant;

int makeRepair (shObject *tool);


typedef int shCanisterFunc (shObject *canister); /* returns ms elapsed */
typedef int shCanisterUseFunc (shObject *canister);

struct shCanisterIlk : shObjectIlk
{
    shCanisterUseFunc *mUseFunc;
    shCanisterFunc *mQuaffFunc;
    shCanisterFunc *mExplodeFunc;
    shCanisterIlk (const char *name, const char *appearance, int flags, 
                   shCanisterUseFunc *usefunc,
                   shCanisterFunc *quafffunc,
                   shCanisterFunc *explodefunc, 
                   int cost,
                   int prob);
};


typedef int shFloppyDiskFunc (shObject *computer, shObject *disk); 

struct shFloppyDiskIlk : shObjectIlk
{
    shFloppyDiskFunc *mUseFunc;
    shFloppyDiskIlk (const char *name, const char *appearance,
                     shFloppyDiskFunc *usefunc,
                     int cost, int prob);
};

shObjectIlk *pickAnIlk (shVector <shObjectIlk *> *ilks);

extern shObjectIlk WreckIlk;
extern shObjectIlk MoneyIlk;
extern shVector <shObjectIlk *> WeaponIlks;
extern shVector <shObjectIlk *> ArmorIlks;
extern shVector <shObjectIlk *> ToolIlks;
extern shVector <shObjectIlk *> CanisterIlks;
extern shVector <shObjectIlk *> FloppyDiskIlks;
extern shVector <shObjectIlk *> ImplantIlks;
extern shVector <shObjectIlk *> ArtifactIlks;
extern shVector <shObjectIlk *> RayGunIlks;
extern shObjectIlk EnergyCellIlk;


extern shToolIlk *Computer;
extern shToolIlk *MasterKey;
extern shToolIlk *LockPick;


extern shArmorIlk *shHelmet;
extern shArmorIlk *shGoggles;
extern shArmorIlk *shBodyArmor;
extern shArmorIlk *shJumpsuit;
extern shArmorIlk *shBelt;

#define BZ 
#define PCT

#endif 






