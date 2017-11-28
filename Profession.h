#include "Global.h"
#include "Creature.h"
#include "Object.h"


/* professions are basically character classes.  I didn't want to use the
   term class and cause confusion with C++ classes...  -CADV 
*/

struct shProfession;
extern shVector <shProfession *> Professions;

#ifndef CLASS_H
#define CLASS_H


struct shSkill {
    shSkillCode mCode;
    shMutantPower mPower; /* only set for mCode == kMutantPower */

    char mRanks;
    char mBonus;     /* bonus from items, race, and misc effects */
    short mExercise; /* number of times the skill has been exercised */
    char mAccess;    /* number of ranks earned per 4 levels */

    shSkill (shSkillCode code, shMutantPower power = kNoMutantPower) {
        mCode = code;
        if (kMutantPower == code) {
            mPower = power;
        } else { 
            mPower = kNoMutantPower;
        }
        mAccess = 1;
        mRanks = 0;
        mBonus = 0;
        mExercise = 0;
    }

    const char *getName ();
    void getDesc (char *buf, int len);
};

typedef void (shHeroInitFunction) (shHero *);

struct shProfession
{
    int mId;
    const char *mName;        /* generic name, e.g. "janitor" */
    int mHitDieSize;
    const char *mTitles[10];

    int mNumPracticedSkills;  /* num skill points earned per level */
    int mBAB;                 /* BAB = level * mBAB / 4 */
    int mReflexSaveBonus;     /* Reflex Save = level * mRSB / 4 */
    int mWillSaveBonus;       /* Reflex Save = level * mWSB / 4 */

    struct InventorySpec {
        shObject *mObject;
        int mChance; /* chance in 100 */
    };

    shVector <InventorySpec> mStartingInventory;
    shHeroInitFunction *mInitFunction;

    //constructor:
    shProfession (const char *name, int hitdiesize, int numskills,
                  int bab, int reflex, int will,
                  shHeroInitFunction *f,
                  const char *t1,
                  const char *t2,
                  const char *t3,
                  const char *t4,
                  const char *t5,
                  const char *t6,
                  const char *t7,
                  const char *t8,
                  const char *t9,
                  const char *t10);

    void addStartingEquipment (shObject *, int chance);

};


void initializeProfessions ();

shProfession *chooseProfession ();

extern shProfession *SpaceMarine;
extern shProfession *Janitor;
extern shProfession *Quarterback;
extern shProfession *Psion;
extern shProfession *SoftwareEngineer;
extern shProfession *Cracker;

#endif
