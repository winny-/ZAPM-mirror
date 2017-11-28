#include "Global.h"
#include "Util.h"
#include "Creature.h"
#include "Profession.h"
#include "Object.h"
#include "Creature.h"
#include "Hero.h"

shVector <shProfession *> Professions;
shProfession *Janitor;
shProfession *SoftwareEngineer;
shProfession *SpaceMarine;
shProfession *Quarterback;
shProfession *Psion;
shProfession *Cracker;

shProfession::shProfession (const char *name,
                            int hitdiesize,
                            int numskills,
                            int bab,
                            int reflex,
                            int will,
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
                            const char *t10)
{
    mId = Professions.add (this);
    mName = name;
    mHitDieSize = hitdiesize;
    mNumPracticedSkills = numskills;
    mBAB = bab;
    mReflexSaveBonus = reflex;
    mWillSaveBonus = will;
    mInitFunction = f;
    mTitles[0] = t1;
    mTitles[1] = t2;
    mTitles[2] = t3;
    mTitles[3] = t4;
    mTitles[4] = t5;
    mTitles[5] = t6;
    mTitles[6] = t7;
    mTitles[7] = t8;
    mTitles[8] = t9;
    mTitles[9] = t10;
}


void
shProfession::addStartingEquipment (shObject *object, int chance)
{
    InventorySpec spec;

    spec.mObject = object;
    spec.mChance = chance;
    mStartingInventory.add (spec);
}
                                    

static void
initSoftwareEngineer (shHero *hero)
{
    hero->mProfession = SoftwareEngineer;
    hero->rollAbilityScores (8, 8, 8, 12, 13, 12, 6);

    hero->mMaxHP = SoftwareEngineer->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());

    hero->mInateIntrinsics |= kBugSensing;

    hero->addSkill (kGrenade, 2);
    hero->addSkill (kHandgun, 3);
    hero->addSkill (kLightGun, 1);
    hero->addSkill (kHeavyGun, 1);
    hero->addSkill (kUnarmedCombat, 0);
    hero->addSkill (kMeleeWeapon, 1);
    hero->addSkill (kSword, 1);

    hero->addSkill (kConcentration, 2);
    //hero->addSkill (kHide, 1);
    //hero->addSkill (kMoveSilently, 1);
    hero->addSkill (kOpenLock, 4);
    hero->addSkill (kRepair, 4);
    hero->addSkill (kSearch, 4);
    hero->addSkill (kHacking, 3);
    //hero->addSkill (kListen, 3);
    hero->addSkill (kSpot, 2);
    
    hero->gainRank (kHacking, 2);

    hero->wield (createObject ("debugged +0 pea shooter", 0), 1);
    hero->mWeapon->identify ();
    hero->addObjectToInventory (hero->mWeapon, 1);

    hero->don (createObject ("debugged +0 ordinary jumpsuit", 0), 1);
    hero->mJumpsuit->identify ();
    hero->addObjectToInventory (hero->mJumpsuit, 1);

    shObject *computer = createObject ("debugged mini computer", 0);
    computer->identify ();
    hero->addObjectToInventory (computer, 1);

    shObject *disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);

    
    shObject *nc = createObject ("debugged canister of nano cola", 0);
    nc->identify ();
    nc->mCount = RNG (1, 2);
    hero->addObjectToInventory (nc, 1);

    shObject *dt = createObject ("debugged roll of duct tape", 0);
    dt->identify ();
    dt->mCount = RNG (1, 2);
    hero->addObjectToInventory (dt, 1);

    shObject *bolt = createObject ("debugged restraining bolt", 0);
    bolt->identify ();    
    hero->addObjectToInventory (bolt, 1);

    hero->addObjectToInventory (createObject ("200 energy cells", 0), 1);

    hero->addObjectToInventory (createMoney (RNG (1, 100) + RNG (1, 100)), 1);
}


static void
initCracker (shHero *hero)
{
    hero->mProfession = Cracker;
    hero->rollAbilityScores (8, 8, 8, 10, 13, 10, 10);

    hero->mMaxHP = Cracker->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());

    hero->addSkill (kGrenade, 1);
    hero->addSkill (kHandgun, 3);
    hero->addSkill (kLightGun, 1);
    hero->addSkill (kHeavyGun, 0);
    hero->addSkill (kUnarmedCombat, 4);
    hero->addSkill (kMeleeWeapon, 1);
    hero->addSkill (kSword, 0);

    hero->addSkill (kConcentration, 4);
    //hero->addSkill (kHide, 0);
    //hero->addSkill (kMoveSilently, 0);
    hero->addSkill (kOpenLock, 4);
    hero->addSkill (kRepair, 4);
    hero->addSkill (kSearch, 4);
    hero->addSkill (kHacking, 4);
    //hero->addSkill (kListen, 2);
    hero->addSkill (kSpot, 2);
    
    hero->gainRank (kHacking, 2);

    hero->wield (createObject ("debugged +0 pea shooter", 0), 1);
    hero->mWeapon->identify ();
    hero->addObjectToInventory (hero->mWeapon, 1);

    hero->don (createObject ("debugged +0 ordinary jumpsuit", 0), 1);
    hero->mJumpsuit->identify ();
    hero->addObjectToInventory (hero->mJumpsuit, 1);

    shObject *computer = createObject ("debugged mini computer", 0);
    computer->identify ();
    hero->addObjectToInventory (computer, 1);

    shObject *disk = createFloppyDisk ();
    disk->identify ();
    disk->setCracked ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);
    disk = createFloppyDisk ();
    disk->identify ();
    hero->addObjectToInventory (disk, 1);

    shObject *nc = createObject ("debugged canister of nano cola", 0);
    nc->identify ();
    nc->mCount = RNG (1, 2);
    hero->addObjectToInventory (nc, 1);

    shObject *dt = createObject ("debugged roll of duct tape", 0);
    dt->identify ();
    dt->mCount = RNG (1, 2);
    hero->addObjectToInventory (dt, 1);

    shObject *bolt = createObject ("debugged lock pick", 0);
    bolt->identify ();    
    hero->addObjectToInventory (bolt, 1);

    hero->addObjectToInventory (createObject ("200 energy cells", 0), 1);

    hero->addObjectToInventory (createMoney (RNG (1, 100) + RNG (1, 100)), 1);
}


static void
initSpaceMarine (shHero *hero)
{
    hero->mProfession = SpaceMarine;
    hero->rollAbilityScores (13, 12, 9, 11, 7, 8, 8);
    if (hero->mAbil.mStr < 10) { 
        hero->mAbil.mStr = 10;
        hero->mMaxAbil.mStr = 10;
    }

    hero->mMaxHP = SpaceMarine->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());
    
    hero->mBAB = 1;
    hero->addSkill (kGrenade, 3);
    hero->addSkill (kHandgun, 4);
    hero->addSkill (kLightGun, 4);
    hero->addSkill (kHeavyGun, 4);
    hero->addSkill (kUnarmedCombat, 3);
    hero->addSkill (kMeleeWeapon, 3);
    hero->addSkill (kSword, 1);

    hero->addSkill (kConcentration, 0);
    //hero->addSkill (kHide, 0);
    //hero->addSkill (kMoveSilently, 0);
    hero->addSkill (kOpenLock, 2);
    hero->addSkill (kRepair, 2);
    hero->addSkill (kSearch, 2);
    hero->addSkill (kHacking, 0);
    //hero->addSkill (kListen, 2);
    hero->addSkill (kSpot, 2);

    hero->gainRank (kHandgun, 2);
    hero->gainRank (kGrenade, 2);
    hero->gainRank (kLightGun, 2);
    hero->gainRank (kHeavyGun, 2);

    hero->don (createObject ("debugged +1 flak jacket", 0), 1);
    hero->mBodyArmor->identify ();
    hero->addObjectToInventory (hero->mBodyArmor, 1);

    shObject *pistol = createObject ("debugged +0 pistol", 0);
    pistol->identify ();
    hero->addObjectToInventory (pistol, 1);

    hero->wield (createObject ("debugged +0 pulse rifle", 0), 1);
    hero->mWeapon->identify ();
    hero->addObjectToInventory (hero->mWeapon, 1);

    shObject *bul = createObject ("99 debugged +0 bullets", 0);
    bul->identify ();
    hero->addObjectToInventory (bul, 1);

    hero->addObjectToInventory (createObject ("100 energy cells", 0), 1);

    shObject *mt = createObject ("debugged motion tracker", 0);
    mt->identify ();
    hero->addObjectToInventory (mt, 1);

    hero->addObjectToInventory (createMoney (RNG (1, 10) + RNG (1, 10)), 1);
}


void
initQuarterback (shHero *hero)
{
    hero->mProfession = Quarterback;
    hero->rollAbilityScores (12, 10, 13, 12, 7, 7, 10);

    hero->mMaxHP = Quarterback->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());

    hero->addSkill (kGrenade, 4);
    hero->addSkill (kHandgun, 2);
    hero->addSkill (kLightGun, 2);
    hero->addSkill (kHeavyGun, 2);
    hero->addSkill (kUnarmedCombat, 4);
    hero->addSkill (kMeleeWeapon, 3);
    hero->addSkill (kSword, 2);

    hero->addSkill (kConcentration, 2);
    //hero->addSkill (kHide, 0);
    //hero->addSkill (kMoveSilently, 0);
    hero->addSkill (kOpenLock, 0);
    hero->addSkill (kRepair, 0);
    hero->addSkill (kSearch, 2);
    hero->addSkill (kHacking, 0);
    //hero->addSkill (kListen, 2);
    hero->addSkill (kSpot, 4);

    hero->gainRank (kGrenade, 2);
    hero->gainRank (kSpot, 2);

    hero->don (createObject ("debugged +0 set of football pads", 0), 1);
    hero->mBodyArmor->identify ();
    hero->addObjectToInventory (hero->mBodyArmor, 1);

    hero->don (createObject ("debugged +0 football helmet", 0), 1);
    hero->mHelmet->identify ();
    hero->addObjectToInventory (hero->mHelmet, 1);

    hero->wield (createObject ("1 debugged +3 football", 0), 1);
    hero->mWeapon->identify ();
    hero->addObjectToInventory (hero->mWeapon, 1);

    hero->addObjectToInventory (createObject ("200 energy cells", 0), 1);

    hero->addObjectToInventory (createMoney (NDX (3, 100)), 1);

    shObject *beer = createObject ("6 debugged canisters of beer", 0);
    beer->identify ();
    hero->addObjectToInventory (beer, 1);
}


static void
initPsion (shHero *hero)
{
    shMutantPower pwr;

    hero->mProfession = Psion;
    hero->rollAbilityScores (7, 7, 9, 8, 12, 12, 15);
    if (hero->mAbil.mCha < 14) { 
        hero->mAbil.mCha = 14;
        hero->mMaxAbil.mCha = 14;
    }

    hero->mMaxHP = Psion->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());

    hero->addSkill (kGrenade, 1);
    hero->addSkill (kHandgun, 2);
    hero->addSkill (kLightGun, 1);
    hero->addSkill (kHeavyGun, 1);
    hero->addSkill (kUnarmedCombat, 1);
    hero->addSkill (kMeleeWeapon, 1);
    hero->addSkill (kSword, 4);

    hero->addSkill (kConcentration, 4);
    //hero->addSkill (kHide, 2);
    //hero->addSkill (kMoveSilently, 2);
    hero->addSkill (kOpenLock, 1);
    hero->addSkill (kRepair, 1);
    hero->addSkill (kSearch, 3);
    hero->addSkill (kHacking, 0);
    //hero->addSkill (kListen, 2);
    hero->addSkill (kSpot, 2);

    hero->addSkill (kMutantPower, 4, kIllumination);
    hero->addSkill (kMutantPower, 4, kDigestion);
    hero->addSkill (kMutantPower, 4, kHypnosis);
    hero->addSkill (kMutantPower, 4, kRegeneration);
    hero->addSkill (kMutantPower, 4, kOpticBlast);
    hero->addSkill (kMutantPower, 4, kHaste);
    hero->addSkill (kMutantPower, 4, kTelepathyPower);
    hero->addSkill (kMutantPower, 4, kShootWebs);
    hero->addSkill (kMutantPower, 4, kMentalBlast);
    hero->addSkill (kMutantPower, 4, kPyrokinesis);
    hero->addSkill (kMutantPower, 4, kRestoration);
    hero->addSkill (kMutantPower, 4, kAdrenalineControl);
    hero->addSkill (kMutantPower, 4, kXRayVisionPower);
    hero->addSkill (kMutantPower, 4, kTelekinesis);
    hero->addSkill (kMutantPower, 4, kInvisibility);
    hero->addSkill (kMutantPower, 4, kCharm);
    hero->addSkill (kMutantPower, 4, kTeleport);


    while (kNoMutantPower == (pwr = hero->getMutantPower (kNoMutantPower, 1)));
    hero->gainRank (kMutantPower, 2, pwr);
    while (kNoMutantPower == (pwr = hero->getMutantPower (kNoMutantPower, 1)));
    hero->gainRank (kMutantPower, 2, pwr);
    
    shObject *lp = createObject ("debugged +1 laser pistol", 0);
    lp->identify ();
    hero->addObjectToInventory (lp, 1);
    hero->wield (lp, 1);

    hero->don (createObject ("debugged +1 ordinary jumpsuit", 0), 1);
    hero->mJumpsuit->identify ();
    hero->addObjectToInventory (hero->mJumpsuit, 1);

    hero->don (createObject ("debugged +0 pair of sunglasses", 0), 1);
    hero->mGoggles->identify ();
    hero->addObjectToInventory (hero->mGoggles, 1);

    hero->addObjectToInventory (createObject ("200 energy cells", 0), 1);

    shObject *nc = createObject ("2 debugged canisters of nano cola", 0);
    nc->identify ();
    hero->addObjectToInventory (nc, 1);

    shObject *hc = createObject ("1 debugged canister of healing", 0);
    hc->identify ();
    hero->addObjectToInventory (hc, 1);

    shObject *rac = createObject ("1 debugged canister of Rad-Away", 0);
    rac->identify ();
    hero->addObjectToInventory (rac, 1);

    hero->addObjectToInventory (createMoney (RNG (1, 10) + RNG (1, 10)), 1);

}


static void
initJanitor (shHero *hero)
{
    hero->mProfession = Janitor;
    hero->rollAbilityScores (10, 12, 10, 12, 8, 10, 8);

    hero->mMaxHP = Janitor->mHitDieSize + 
        ABILITY_MODIFIER (hero->getCon ());

    //hero->mInateResistances[kSickening] = 122; 

    hero->addSkill (kGrenade, 1);
    hero->addSkill (kHandgun, 3);
    hero->addSkill (kLightGun, 1);
    hero->addSkill (kHeavyGun, 1);
    hero->addSkill (kUnarmedCombat, 2);
    hero->addSkill (kMeleeWeapon, 3);
    hero->addSkill (kSword, 1);

    hero->addSkill (kConcentration, 2);
    //hero->addSkill (kHide, 2);
    //hero->addSkill (kMoveSilently, 2);
    hero->addSkill (kOpenLock, 4);
    hero->addSkill (kRepair, 4);
    hero->addSkill (kSearch, 4);
    hero->addSkill (kHacking, 0);
    //hero->addSkill (kListen, 3);
    hero->addSkill (kSpot, 4);
    
    hero->gainRank (kSpot, 2);
    hero->gainRank (kSearch, 2);
    hero->gainRank (kRepair, 2);


    hero->wield (createObject ("debugged +1 mop", 0), 1);
    hero->mWeapon->identify ();
    hero->addObjectToInventory (hero->mWeapon, 1);

    hero->don (createObject ("debugged +1 janitor uniform", 0), 1);
    hero->mJumpsuit->identify ();
    hero->addObjectToInventory (hero->mJumpsuit, 1);
    
    shObject *sg = createObject ("debugged canister of super glue", 0);
    sg->identify ();
    hero->addObjectToInventory (sg, 1);

    shObject *mw = createObject ("optimized monkey wrench", 0);
    mw->identify ();
    hero->addObjectToInventory (mw, 1);

    shObject *kc = createObject ("debugged master keycard", 0);
    kc->identify ();
    hero->addObjectToInventory (kc, 1);

    shObject *bolt = createObject ("debugged restraining bolt", 0);
    bolt->identify ();    
    hero->addObjectToInventory (bolt, 1);

    hero->addObjectToInventory (createObject ("75 energy cells", 0), 1);

    hero->addObjectToInventory (createMoney (RNG (1, 100)), 1);
}



void
initializeProfessions ()
{

    Psion = new shProfession ("psion", 8, 3, 3, 2, 2, initPsion,
                              "Odd Ball",
                              "Weirdo",
                              "Mind Reader",
                              "Spoon Bender",
                              "Freakazoid",
                              "Mutant",
                              "Empath",
                              "Psion",
                              "Psyker",
                              "Farseer");

    SoftwareEngineer = new shProfession ("software engineer", 8, 3, 3, 1, 2,
                                         initSoftwareEngineer,
                                         "Summer Intern",
                                         "Q/A Tester",
                                         "Web Designer",
                                         "Help Desk Jockey",
                                         "Jr. Programmer",
                                         "Sysadmin",
                                         "Programmer",
                                         "Lead Programmer",
                                         "VP Engineering",
                                         "High Programmer");

    Cracker = new shProfession ("cracker", 8, 4, 2, 1, 2,
                                initCracker,
                                "Savescummer",
                                "File Sharer",
                                "W@r3z d00d",
                                "Script Kiddie",
                                "h@x0r",
                                "1337 h@x0r",
                                "Decker",
                                "Sneaker",
                                "Phreaker",
                                "One");

    Janitor = new shProfession ("janitor", 8, 3, 3, 2, 2, 
                                initJanitor,
                                "Toilet Scrubber",
                                "Mop Boy",
                                "Janitor",
                                "Housekeeper",
                                "Custodian",
                                "Maintenance Man",
                                "Sanitation Engineer",
                                "Superintendent",
                                "Property Manager",
                                "Landlord");
/* decker?  */

    SpaceMarine = new shProfession ("space marine", 10, 2, 4, 2, 1,
                                    initSpaceMarine,
                                    "Private",
                                    "Corporal",
                                    "Sergeant",
                                    "Cadet",
                                    "Lieutentant",
                                    "Captain",
                                    "Major",
                                    "Lt. Colonel",
                                    "Colonel",
                                    "General");

    Quarterback = new shProfession ("quarterback", 10, 2, 3, 2, 1,
                                    initQuarterback,
                                    "Towel Boy",
                                    "Rookie",
                                    "Bench Warmer",
                                    "Starter",
                                    "Jock",
                                    "Star Player",
                                    "Team Captain",
                                    "MVP",
                                    "Pro Bowler",
                                    "Hall Of Famer");

}


shProfession *
chooseProfession ()
{
    const void *result = NULL;
    shMenu menu ("Choose your profession", 0);

    menu.addItem ('j', "Janitor", Janitor);
    menu.addItem ('m', "Space Marine", SpaceMarine);
    menu.addItem ('p', "Psion", Psion);
    menu.addItem ('q', "Quarterback", Quarterback);
    menu.addItem ('s', "Software Engineer", SoftwareEngineer);

    menu.getResult (&result);
    return (shProfession *) result;
}
