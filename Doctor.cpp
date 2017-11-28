#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Creature.h"
#include "Interface.h"

/******************************************************

shopping code

******************************************************/


/* called when the hero enters a shop room */

void
shHero::enterHospital ()
{
    shMonster *doctor = mLevel->getDoctor (mX, mY);
    
    if (doctor) {
        if (doctor->isHostile ()) {
            return;
        }
        if (tryToTranslate (doctor)) {
            I->p ("\"Welcome to Ol' Sawbot's Precision Surgery Center!\"");
        } else {
            I->p ("%s beeps and bloops pleasantly.", doctor->the ());
        }
        I->p ("Press 'p' to see a menu of available services.");
    } else {
        I->p ("Hmm...  This store appears to be deserted.");
    }
}


void
shHero::leaveHospital ()
{
    shMonster *doc = mLevel->getDoctor (mX, mY);
    shObjectVector v;

    if (!doc) {
        return;
    }
    if (doc->isHostile ()) {
        return;
    } else if (tryToTranslate (doc)) {
        I->p ("\"Come again soon!\"");
    } else {
        I->p ("%s twitters cheerfully.", doc->the ());
    }
}



const char * MedicalProcedureNames[] = 
{ "Wound Treatment",
  "Restoration Treatment",
  "Intestinal Examination",
  "Systems Diagnostics",
  "Radiation Purge",
  "Caesarean Section"
};


shAttack CaesareanDamage = 
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0,
              kSlashing, 6, 6);


void
shHero::payDoctor (shMonster *doctor)
{
    char buf[200];
    shMenu menu ("Medical Services Menu", shMenu::kNothingAllowed);
    int i, j, serv;
    const int TREATMENT_COST = 200;

    if (tryToTranslate (doctor)) {
        /* make all services known */
        for (i = 0; i < kMedMaxService; i++) {
            MedicalProcedureData[i].mNameKnown = 1;
        }
    }

    char letter = 'a';
    for (i = 0; i < kMedMaxService; i++) {
        serv = doctor->mDoctor.mPermute[i];

        /* try not to offer unneeded services */
        switch (serv) {
        case kMedHealing:
            if (Hero.mHP != Hero.mMaxHP ||
                Hero.isViolated () ||
                Hero.isConfused () ||
                Hero.isStunned ())
            {
                goto addservice;
            }
            continue;
        case kMedRestoration:
            for (j = 1; j <= 7; j++) {
                int abil = Hero.mAbil.getByIndex (j);
                if (kCha == j) {
                    abil += Hero.mChaDrain;
                }
                if (abil < Hero.mMaxAbil.getByIndex (j))
                    goto addservice;
            }
            continue;
        case kMedRectalExam:
        case kMedDiagnostics:
        case kMedRadPurification:
            /* these services are always available: */
            goto addservice;
        case kMedCaesareanSection:
            if (Hero.getStoryFlag ("impregnation"))
                goto addservice;
        default:
            continue;
        }

    addservice:

        snprintf (buf, sizeof(buf), "%s ($%d)",
                  MedicalProcedureData[serv].mNameKnown 
                      ? MedicalProcedureNames[serv]
                      : MedicalProcedureData[serv].mDesc,
                 TREATMENT_COST);
        menu.addItem (letter++, buf, (void *) serv);
    }

    int choice;

    if (!menu.getResult  ((const void **) &choice))
        return; /* nothing picked */ 

    int price = TREATMENT_COST;

    if (countMoney () < price) {
        I->p ("You don't have enough money for that.");
        return;
    }
    
    loseMoney (price);
    doctor->gainMoney (price);

    const char *who = doctor->the ();

    switch (choice) {
    case kMedHealing:
        I->p ("%s injects you with a %s serum!", who,
              findAnIlk (&RayGunIlks, "healing ray gun") ->getRayGunColor ());
        if (Hero.healing (0))
            MedicalProcedureData[choice].mNameKnown = 1;
        break;
    case kMedRestoration:
        I->p ("%s injects you with a %s serum!", who,
              findAnIlk (&RayGunIlks, "restoration ray gun") ->getRayGunColor ());
        if (Hero.restoration ())
            MedicalProcedureData[choice].mNameKnown = 1;
        break;
    case kMedRectalExam:
        MedicalProcedureData[choice].mNameKnown = 1;
        I->p ("%s probes you!", who);
        Hero.makeViolated (1000 * NDX (20, 20));
        break;
    case kMedDiagnostics:
        MedicalProcedureData[choice].mNameKnown = 1;
        I->p ("%s probes you!", who);
        Hero.doDiagnostics ();
        break;
    case kMedRadPurification:
        MedicalProcedureData[choice].mNameKnown = 1;
        I->p ("%s injects you with a bubbly serum!", who);
        Hero.mRad -= RNG (1, 200);
        Hero.mRad = maxi (0, Hero.mRad);
        if (!Hero.mRad)
            I->p ("You feel purified.");
        else 
            I->p ("You feel less contaminated.");
        break;
    case kMedCaesareanSection:
    {
        if (!Hero.getStoryFlag ("impregnation"))
            break;

        Hero.setStoryFlag ("impregnation", 0);

        int x = Hero.mX;
        int y = Hero.mY;
        int queen = !RNG (0, 17);
        int colicky = !RNG (0, 5);
        shMonster *baby = 
            new shMonster (findAMonsterIlk (queen ? "alien princess"
                                                  : "chestburster"));
        if (!colicky) {
            baby->mDisposition = shMonster::kIndifferent;
        }
        Level->findNearbyUnoccupiedSquare (&x, &y);
        if (!baby) {
            I->p ("Unfortunately, your baby was stillborn.");
        } else {
            I->p ("It's a %s!", queen ? "girl" : "boy");
            if (Level->putCreature (baby, x, y)) {
                /* FIXME: something went wrong */
            } else {
                I->drawScreen ();
            }
        }

        I->p ("You lose a lot of blood during the operation.");
        if (Hero.sufferDamage (&CaesareanDamage)) {
            Hero.die (kKilled, "complications in childbirth");
        }
        break;
    }
    default:
        break;
    }
}

/* doctor strategy: 
     wait around for hero to request services;
     occasionally advertise
                   
   returns ms elapsed, -2 if the monster dies
*/
int
shMonster::doDoctor ()
{    
    I->debug ("  doctor strategy");
    int elapsed;
    int res = -1;
    int retry = 3;
    
    while (-1 == res) {
        if (!retry--) {
            return 200;
        }

        switch (mTactic) {

        case kNewEnemy:
            mStrategy = kWander;
            mTactic = kNewEnemy;
            return doWander ();
        case kMoveTo:
            res = doMoveTo ();
            continue;
        case kReady:    
            if (Level->getRoomID (mX, mY) != mDoctor.mRoomID) {
                /* somehow, we're not in our hospital! */
                mDestX = mDoctor.mHomeX;
                mDestY = mDoctor.mHomeY;
                
                if (setupPath ()) {
                    mTactic = kMoveTo;
                    continue;
                } else {
                    return 2000;
                }
            }

            if (canSee (&Hero) && 
                mDoctor.mRoomID == Level->getRoomID (Hero.mX, Hero.mY))
            {
                if (0 == RNG (12)) {
                    /* move to an empty square near the home square */
                    mDestX = mDoctor.mHomeX;
                    mDestY = mDoctor.mHomeY;
                    if (!mLevel -> 
                        findAdjacentUnoccupiedSquare (&mDestX, &mDestY)) 
                    {
                        elapsed = doQuickMoveTo ();
                        if (-1 == elapsed) elapsed = 800;
                        return elapsed;
                    }
                    return RNG (300, 1000); /* nowhere to go, let's wait... */
                } else if (0 == RNG (50)) {
                    const unsigned int NUMQUIPS = 3;
                    const char *quips[NUMQUIPS] = {
                        "\"Dammit, %s! I'm a docbot, not %s!\"",
                        "\"Your second amputation is half off!\"",
                        "\"Galaxy-class health care services!\"",
                    };
                    const unsigned int NUMNOUNS = 10;
                    const char *nouns[NUMNOUNS] = {
                        "a magician",
                        "a psychiatrist",
                        "a bartender",
                        "a bricklayer",
                        "a mechanic",
                        "a priest",
                        "a lawyer",
                        "a prostitute",
                        "a toaster oven",
                        "a warbot"
                    };
                    if (Hero.tryToTranslate (this)) {
                        I->p (quips[RNG(NUMQUIPS)], 
                              Hero.mName, nouns[RNG(NUMNOUNS)]);
                        Hero.interrupt ();
                    } 
                    return RNG (500, 1000);
                }
            } else {
                return RNG (800, 1600);
            }
        case kFight:
        case kShoot:
            mTactic = kReady;
            I->debug ("Unexped doctor tactic!");
        }
    }

    return RNG (300, 1000); /* keep on lurking */
}




