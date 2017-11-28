#ifndef HERO_H
#define HERO_H

#include "Global.h"
#include "Util.h"
#include "Creature.h"
#include "Monster.h"


struct shStoryFlag 
{ 
    const char *mName;
    int mValue; 
    shStoryFlag () {}
    shStoryFlag (const char *name, int value) 
    { 
        mName = strdup (name);
        mValue = value; 
    }
};


class shHero : public shCreature
{
    friend struct shInterface;

    int mXP;
    int mScore;
    int mBusy;
    int mSkillPoints;
    shVector <shStoryFlag *> mStoryFlags;
    
 public:
    shVector <shCreature *> mPets;

    void saveState (int fd);
    void loadState (int fd);

    int isHero () { return 1; }

    void init (const char *name, shProfession *profession);

    const char *the ();
    const char *an ();
    const char *your ();
    const char *getDescription ();

    int numHands () { return mIlk->mNumHands; }

    int die (shCauseOfDeath how, const char *killer = NULL);
    int die (shCauseOfDeath how, shCreature *killer);
    void epitaph (char *buf, int len, 
                  shCauseOfDeath how, const char *killer = NULL);
    void tallyScore ();
    void tomb (char *message);
    void logGame (char *message);

    int addObjectToInventory (shObject *obj, int quiet = 0);

    int wield (shObject *obj, int quiet = 0);
    int unwield (shObject *obj, int quiet = 0);
        
    void earnXP (int challenge);
    void earnScore (int points);
    void gainAbility ();

    void oldLocation (int newX, int newY, shMapLevel *newLevel);
    void newLocation ();
    void lookAtFloor (int menuok = 0);
    int getStoryFlag (const char *name) { 
        int i;
        for (i = 0; i < mStoryFlags.count (); i++) {
            if (0 == strcmp (name, mStoryFlags.get (i) -> mName)) {
                return mStoryFlags.get (i) -> mValue;
            }
        }
        return 0;
    }

    void setStoryFlag (const char *name, int value) {
        int i;
        for (i = 0; i < mStoryFlags.count (); i++) {
            if (0 == strcmp (name, mStoryFlags.get (i) -> mName)) {
                mStoryFlags.get (i) -> mValue = value;
                return;
            }
        }
        mStoryFlags.add (new shStoryFlag (name, value));
    }

    void resetStoryFlag (const char *name) {
        setStoryFlag (name, 0);
    }

    int looksLikeJanitor ();

    shObject *quickPickItem (shObjectVector *v, const char *action, 
                             int flags, int *count = NULL);

    int interrupt ();  /* returns 0 if hero was busy */

    int instantUpkeep ();

    void doGodMode ();

    int tryToTranslate (shCreature *c);

    void enterShop ();    
    void leaveShop ();
    void damagedShop (int x, int y);
    void pickedUpItem (shObject *obj);
    void usedUpItem (shObject *obj, int cnt, const char *action);
    void maybeSellItem (shObject *obj);
    void payShopkeeper ();

    void enterHospital ();    
    void leaveHospital ();
    void payDoctor (shMonster *doctor);

    void enterCompactor ();    
    void leaveCompactor ();

    void doDiagnostics ();

    void checkForFollowers (shMapLevel *level, int sx, int sy);
    int displace (shCreature *c);
    int doMove (shDirection dir);
    int tryToEscapeTrap ();
    void drop (shObject *obj);
    int kick (shDirection dir);
    void reorganizeInventory ();
    void listInventory ();
    void feel (int x, int y, int force = 0);
    void spotStuff ();
    void sensePeril ();

    void addSkillPoints (int points) { mSkillPoints += points; }
    void editSkills ();

    void quaffFromVat (shFeature *vat);
    int useKey (shObject *key, shFeature *door);

    shMutantPower getMutantPower (shMutantPower power = kNoMutantPower,
                                  int silent = 0);

    int useMutantPower ();

    /* fighting functions; see Fight.cpp */
    int meleeAttack (shObject *weapon, shDirection dir);

    void upkeep ();
    void takeTurn ();
};

int quaffGainAbility (shObject *can);

#endif




