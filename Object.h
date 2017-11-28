struct shEntity;
struct shObject;
#include "ObjectType.h"


#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>

#include "Global.h"
#include "Util.h"



typedef shVector <shObject *> shObjectVector ;

int selectObjectsByFlag (shObjectVector *dest, shObjectVector *src, int flag);
int selectObjects (shObjectVector *dest, shObjectVector *src, shObjectIlk *ilk);
int selectObjects (shObjectVector *dest, shObjectVector *src, 
                   shObjectType type);
int selectObjectsByFunction (shObjectVector *dest, shObjectVector *src, 
                             int (shObject::*idfunc) (), int neg = 0);
int unselectObjectsByFunction (shObjectVector *dest, shObjectVector *src, 
                               int (shObject::*idfunc) ());



/* an shThing is an shEntity that can be associated with an shEvent.
   almost every shEntity is an shThing, examples include shObject 
   and shCreature
*/

struct shThing : shEntity
{
    ~shThing ();

    void addEvent (shEvent *);
    void removeEvent (shEvent *);

    shVector <shEvent *> mEvents; /* events that refer to this creature; they
                                       will be cancel()'d by our destructor */
};


/* final */
struct shObject : shThing
{
    /* NOTE: don't forget to update constructor AND split () method 
       when adding new fields to this class!
       oh yeah and saveObject() and loadObject() too!
    */

    shObjectIlk *mIlk;
    short mCount;             // for mergeable objects
    short mCharges;           // for chargeable objects (maxhp for corpse)
    short mHP;                // hit points
    signed char mDamage;      // corrosion / fire damage
    signed char mEnhancement; 
    char mLetter;             // letter in Hero's inventory display
    signed char mBugginess;   // optimized +1 / debugged 0 / buggy -1 
    shTime mLastEnergyBill;   // last time energy was billed to the item
    enum Flags {
        kKnownBugginess =   0x1,
        kKnownFooproof =    0x2,     
        kKnownEnhancement = 0x4,     //also, # of charges 
        kKnownAppearance =  0x8,     //disk labels, canister types
        kKnownExact =       0xf,

        kFooproof =         0x10,  //rustproof, fireproof, etc. 
                                   // also used for cracked software
        
        kWorn =             0x20,
        kWielded =          0x40,
        kActive =           0x80,  // turned on, for example
        kInUse =            0xe0,  // worn, wielded, or active
        kRadioactive =      0x100,
        kUnpaid =           0x200, /* belongs to shopkeeper */
        kToggled =          0x400,
    };
    int mFlags;

    enum Location {
        kNowhere,
        kFloor,
        kInventory,
    } mLocation;
    int mX;                   
    int mY;
    shCreature *mOwner;


    char *mUserName;          /* name given by user with name command */
    union {
        shMonsterIlk *mCorpseIlk;
        shImplantIlk::Site mImplantSite;
    };
//    shMonster *mShopKeeper;

        

    //constructor:
    shObject () {
        mIlk = NULL; mCount = 0; mEnhancement = 0; mCharges = 0; mHP = 0; 
        mLetter = 0; mBugginess = 0; mFlags = 0; mDamage = 0;
        mLastEnergyBill = MAXTIME; mUserName = NULL;
//      mShopKeeper = NULL;
    }

    void saveState (int fd);
    void loadState (int fd);

        
    int getDescription (char *buf, int len);
    int getShortDescription (char *buf, int len);
    char *getFarDescription ();
    int isA (shObjectIlk *type);
    inline int isA (shObjectType type) { return mIlk->mType == type; }
    int isA (const char *ilkname);
    int getIlkFlag (int id) { return mIlk->mFlags & id; }
    int getFlag (int id) { return mFlags & id; }
    void setFlag (int id) { mFlags |= id; }
    void resetFlag (int id) { mFlags &= ~id; }
    int getMass () { return mCount * mIlk->mWeight; }
    void name (char *newname = NULL);
    void nameIlk ();
    inline void maybeName () 
    { 
        if (!isIlkKnown () && !mIlk->mUserName) nameIlk (); 
    } 
    inline int isIdentified () { 
        return (kKnownExact == (kKnownExact & mFlags)) && isIlkKnown (); 
    }
    inline int isUnidentified () { return !isIdentified (); }
    inline int isChargeable () { return getIlkFlag (kChargeable); }
    inline int isChargeKnown () { return getFlag (kKnownEnhancement); }
    inline void setChargeKnown () { setFlag (kKnownEnhancement); }
    inline int isBugginessKnown () { return getFlag (kKnownBugginess); }
    inline void setBugginessKnown () { setFlag (kKnownBugginess); }
    inline void resetBugginessKnown () { resetFlag (kKnownBugginess); }
    inline int isAppearanceKnown () { return getFlag (kKnownAppearance); }
    inline void setAppearanceKnown () { setFlag (kKnownAppearance); }
    inline void resetAppearanceKnown () { resetFlag (kKnownAppearance); }
    inline int isFooproofKnown () { return getFlag (kKnownFooproof); }
    inline void setFooproofKnown () { setFlag (kKnownFooproof); }
    inline void resetFooproofKnown () { resetFlag (kKnownFooproof); }
    inline int isCrackedKnown () { return isFooproofKnown (); }
    inline void setCrackedKnown () { setFooproofKnown (); }
    inline int isUnique () { return getIlkFlag (kUnique); }
    inline int isIlkKnown () { return getIlkFlag (kIdentified); }
    inline void setIlkKnown () 
    {
        setAppearanceKnown ();
        if (!isIlkKnown ()) {
            if (mIlk->mUserName) { 
                free (mIlk->mUserName); 
                mIlk->mUserName = NULL;
            }
        }
        mIlk->mFlags |= kIdentified; 
    }
    inline int isEnhancementKnown () { return getFlag (kKnownEnhancement); }
    inline void setEnhancementKnown () { setFlag (kKnownEnhancement); }
    inline int isEnhanceable () { return getIlkFlag (kEnhanceable); }
    inline int isRadioactive () { return getFlag (kRadioactive); }

    inline void identify () { setFlag (kKnownExact); setIlkKnown (); }

    shEnergyType vulnerability ();
    inline int isFooproof () { return getFlag (kFooproof); }
    inline void setFooproof () { setFlag (kFooproof); }    
    inline int isIndestructible () { return getIlkFlag (kIndestructible); }
    int isFlammable () { return isA (kFloppyDisk); }
    inline int isBugProof () { return getIlkFlag (kBugProof); }
    inline int isBuggy () { return -1 == mBugginess; }
    inline int isOptimized () { return 1 == mBugginess; }
    inline void setBuggy () { if (!isBugProof ()) mBugginess = -1; }
    inline void setOptimized () { if (!isBugProof ()) mBugginess = 1; }
    inline void setDebugged () { mBugginess = 0; }
    inline void setBugginess (int b) { if (!isBugProof ()) mBugginess = b; }
    inline int isCracked () { return isA (kFloppyDisk) && isFooproof (); }
    inline void setCracked () { setFooproof (); }    

    inline int isInUse () { return getFlag (kInUse); }
    inline int isActive () { return getFlag (kActive); }
    inline void setActive () { setFlag (kActive); mLastEnergyBill = Clock; }
    inline void resetActive () 
    {
        resetFlag (kActive); 
        mLastEnergyBill = MAXTIME;
    }
    inline int isWorn () { return getFlag (kWorn); }
    inline void setWorn () { setFlag (kWorn); }
    inline void resetWorn () { resetFlag (kWorn); }
    inline int isWielded () { return getFlag (kWielded); }
    inline void setWielded () { setFlag (kWielded); }
    inline void resetWielded () { resetFlag (kWielded); }
    inline int isUnpaid () { return getFlag (kUnpaid); }
    inline void setUnpaid () { setFlag (kUnpaid); }
    inline void resetUnpaid () { resetFlag (kUnpaid); }
    inline int isToggled () { return getFlag (kToggled); }
    inline void setToggled () { setFlag (kToggled); }
    inline void resetToggled () { resetFlag (kToggled); }

    inline int isPoweredArmor () { 
        return isA (kArmor) && getIlkFlag (kPowered); 
    }
    inline int isMeleeWeapon () {
        return isA (kWeapon) && getIlkFlag (kMelee); 
    }
    inline int isThrownWeapon () {
        return isA (kWeapon) && getIlkFlag (kMissile) && ! getIlkFlag (kAimed);
    }
    inline int isAimedWeapon () {
        return isA (kRayGun) || (isA (kWeapon) && getIlkFlag (kAimed));
    }
    inline int isWeldedWeapon () { 
        return isWielded () && isBuggy () &&
            (isMeleeWeapon () || isAimedWeapon ());
    }
    inline int isSelectiveFireWeapon () {
        return isA (kWeapon) && getIlkFlag (kSelectiveFire);
    }

    inline int isUseable () { 
        return (isA (kCanister) && isIlkKnown () && 
                ((shCanisterIlk *) mIlk)->mUseFunc) 
            || (isA (kTool) && ((shToolIlk *) mIlk)->mUseFunc)
            || (isWielded () && isSelectiveFireWeapon ()) 
            || (isA ("empty ray gun") && isIlkKnown ());
    }
    int isAmmo (shObject *weapon);

    void impact (shCreature *c, shDirection dir, shCreature *thrower = NULL);
    void impact (int x, int y, shDirection dir, shCreature *thrower = NULL);
    void impact (shFeature *c, shDirection dir, shCreature *thrower = NULL);

    int sufferDamage (shAttack *attack, int x, int y, int multiplier = 1,
                      int specialonly = 0);

    inline int getConferredIntrinsics () { 
        return mIlk->mCarriedIntrinsics | 
            (isInUse () ? mIlk->mInUseIntrinsics : 0);
    }

    void applyConferredResistances (shCreature *target);

    void draw (shInterface *I);
    int canMerge (shObject *obj = NULL);
    void merge (shObject *obj); /* obj will be deleted */
    shObject *split (int count);

    int these (char *buf, int buflen) {
        int l;
        if (mCount > 1) {
            l = snprintf (buf, buflen, "these %d ", mCount);
        } else {
            l = snprintf (buf, buflen, "this ");
        }
        return l + getDescription (buf + l, buflen - l);
    }

    int the (char *buf, int buflen) {
        int l;
        if (mCount > 1) {
            l = snprintf (buf, buflen, "the %d ", mCount);
        } else {
            l = snprintf (buf, buflen, "the ");
        }
        return l + getDescription (buf + l, buflen - l);
    }

    int an (char *buf, int buflen) {
        char tempbuf[80];

        getDescription (tempbuf, 80);
        if (mCount > 1) {
            return snprintf (buf, buflen, "%d %s", mCount, tempbuf);
        } else {
            return snprintf (buf, buflen, "%s %s", 
                             isUnique () ? "the" :
                             isvowel (tempbuf[0]) ? "an" : "a", tempbuf);
        }
    }

    int inv (char *buf, int buflen);

    int your (char *buf, int buflen) {
        int l;
        if (mCount > 1) {
            l = snprintf (buf, buflen, "your %d ", mCount);
        } else {
            l = snprintf (buf, buflen, "your ");
        }
        return l + getShortDescription (buf + l, buflen - l);
    }

    int her (char *buf, int buflen, shCreature *owner) {
        int l;
        char pronoun[4] = "his";
        if (mCount > 1) {
            l = snprintf (buf, buflen, "%s %d ", pronoun, mCount);
        } else {
            l = snprintf (buf, buflen, "%s ", pronoun);
        }
        return l + getShortDescription (buf + l, buflen - l);
    }


    /* any object can be used as a weapon, so we supply these routines: */
    int getThreatRange (shCreature *target);
    int getCriticalMultiplier ();
    
    int getArmorBonus ();
    int getPsiModifier ();
};


int compareObjects (shObject **obj1, shObject **obj2);


int useTool (shObject *tool);
int useCanister (shObject *can);
int quaffCanister (shObject *can);
int executeFloppyDisk (shObject *computer, shObject *disk);
int loadRayGun (shObject *gun);
int selectWeaponFireMode (shObject *gun);

void identifyObjects (int howmany);

shObject *createMoney (int count);
shObject *createWeapon (char *desc = NULL, 
                        int count = -22, int bugginess = -2,
                        int enhancement = -22, int charges = -22);
shObject *createRayGun (char *desc = NULL, 
                        int bugginess = -2, int charges = -22);
shObject *createArmor (char *desc = NULL, 
                       int count = -22, int bugginess = -2,
                       int enhancement = -22, int charges = -22);
shObject *createTool (char *desc = NULL, 
                      int count = -22, int bugginess = -2,
                      int enhancement = -22, int charges = -22);
shObject *createCanister (char *desc = NULL, 
                          int count = -22, int bugginess = -2,
                          int enhancement = -22, int charges = -22);
shObject *createFloppyDisk (char *desc = NULL, 
                            int count = -22, int bugginess = -2,
                            int enhancement = -22, int charges = -22);
shObject *createImplant (char *desc = NULL,
                         int count = -22, int bugginess = -2, 
                         int enhancement = -22, int charges = -22);
shObject *createCorpse (shCreature *m);


void makePlural (char *buf, int len);
shObject *createObject (const char *desc, int flags);

shObjectIlk *findAnIlk (shVector <shObjectIlk *> *ilks, const char *name,
                        int abstractokay = 0);

shObjectIlk *findAnAbstractIlk (shVector <shObjectIlk *> *ilks, 
                                const char *name);

shObject *generateObject (int level = 1);

void initializeMoney ();
void initializeWeapons ();
void initializeArmor ();
void initializeTools ();
void initializeRayGuns ();
void initializeCanisters ();
void initializeFloppyDisks ();
void initializeImplants ();
void initializeWreck ();
void initializeArtifacts ();
void initializeObjects ();

extern shObject *TheNothingObject;

#endif 
