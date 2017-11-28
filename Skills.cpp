/*******************************

Skill Management Code

each character has access to a variety of skills

practice needed to advance a skill:

ranks   exercise  (x-class)
  1         0         10
  2        10         20
  3        20         40
  4        30         60
  5        40         70
  6        50         80
 etc.

********************************/

#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"
#include "Interface.h"

const char *
shSkill::getName ()
{
    switch (mCode) {
    case kConcentration: return "Concentration";

    case kHide: return "Hide";
    case kMoveSilently: return "Move Silently";
    case kMeleeWeapon: return "Basic Melee Weapons";
    case kSword: return "Swords";
    case kUnarmedCombat: return "Unarmed Combat";

    case kOpenLock: return "Pick Locks";
    case kRepair: return "Repair";

    case kGrenade: return "Thrown Weapons";
    case kHandgun: return "Handguns";
    case kLightGun: return "Light Guns";
    case kHeavyGun: return "Heavy Guns";

    case kSearch: return "Search";
    case kHacking: return "Programming";
    
    case kListen: return "Listen";
    case kSpot: return "Spot";

    case kMutantPower: 
        return getMutantPowerName (mPower);

    default:
        return "Unknown!";
    }
}


#if 0
static int
compareSkills (shSkill **a, shSkill **b)
{
    shSkill *s1 = * (shSkill **) a;
    shSkill *s2 = * (shSkill **) b;

    if (s1->mCode < s2->mCode) {
        return -1;
    } else if (s1->mCode > s2->mCode) {
        return 1;
    } else if (s1->mPower < s2->mPower) {
        return -1;
    } else {
        return 1;
    }
}
#endif


void
shSkill::getDesc (char *buf, int len)
{
    const char *abil = NULL;

    if (kMutantPower == mCode) {
        abil = "Cha";
    } else {
        switch (kSkillAbilityMask & mCode) {
        case kStrSkill: abil = "Str"; break;
        case kConSkill: abil = "Con"; break;
        case kAgiSkill: abil = "Agi"; break;
        case kDexSkill: abil = "Dex"; break;
        case kIntSkill: abil = "Int"; break;
        case kWisSkill: abil = "Wis"; break;
        case kChaSkill: abil = "Cha"; break;
        }
    }

    snprintf (buf, len, "%s (%s)                          ", getName (), abil);
    snprintf (&buf[28], len-28, "%d (%d)      ", mRanks,
              mAccess*(Hero.mCLevel+1)/4);
}


void
shHero::editSkills ()
{
    char prompt[50];
    int i;
    int flags = 0;
    int navail = 0;
    char buf[70];
    shSkill *skill;
    shSkillCode lastcode = kWeaponSkill;

//    mSkills.sort (&compareSkills);

    do {
        if (mSkillPoints > 1) {
            snprintf (prompt, 50, "You may make %d skill advancements:", 
                      mSkillPoints);
            flags |= shMenu::kMultiPick;
        } else if (1 == mSkillPoints) {
            snprintf (prompt, 50, "You may advance a skill:");
            flags |= shMenu::kMultiPick;
        } else {
            snprintf (prompt, 50, "Skills");
            flags |= shMenu::kNoPick;
        }

        shMenu menu (prompt, flags);
        char letter = 'a';

#if 0
        if (mSkillPoints) {
            menu.addHeader ("       Skill                     Ranks");
        } else {
            menu.addHeader ("Skill                    Ranks");
        }
#else 
        menu.addHeader ("Combat Skills");
#endif

        for (i = 0; i < mSkills.count (); i++) {
            skill = mSkills.get (i);
            skill->getDesc (buf, 70);

            if (kMutantPower == skill->mCode && !mMutantPowers[skill->mPower])
                continue; /* can't advance skills til you learn the power */
            
            if (kWeaponSkill & lastcode && !(kWeaponSkill & skill->mCode)) {
                menu.addHeader ("Adventuring Skills");
            } else if (kMutantPower != lastcode && 
                       kMutantPower == skill->mCode) 
            {
                menu.addHeader ("Mutant Power Skills");
            }
            
            lastcode = skill->mCode;

            if (mSkillPoints && 
                skill->mAccess*(mCLevel+1)/4 > skill->mRanks) 
            {
                menu.addItem (letter++, buf, skill);
                navail++;
            } else {
                menu.addItem (0, buf, skill);
            }
        }
        if (mSkillPoints && !navail) { 
            return;
        }
        int advanced = 0;
        do {
            i = menu.getResult ((const void **) &skill, NULL);
            if (skill) {
                --mSkillPoints;
                skill->mRanks++;
                advanced++;
            }
        } while (i && mSkillPoints);
        if (!advanced) 
            return;
    } while (mSkillPoints);

}

