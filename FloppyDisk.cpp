#include <ctype.h>
#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"

shVector <shObjectIlk *> FloppyDiskIlks;
shObjectIlk *BlankDisk;
shObjectIlk *SpamDisk;
shObjectIlk *BugDetectDisk;
/* all floppy programs return:
   0: successfully executed, maybe expire
   -1: don't expire
   1: eat the disk
*/


static int
doBlank (shObject *computer, shObject *disk)
{
    char buf[100];
    char buf2[100];
    char *s;
    int dc;
    int score;
    shObjectIlk *newilk;

    disk->setIlkKnown ();
    I->p ("This disk is blank!");
    I->getStr (buf, 80, "What program do you want to write on it?");
    s = &buf[0];
    while (isspace (*s)) ++s;
    if (0 == strncasecmp ("floppy disk of ", s, 15)) {
        strncpy (buf2, s, 80);
    } 
    snprintf (buf2, 100, "floppy disk of %s", s);
    buf2[99] = 0;
    newilk = findAnIlk (&FloppyDiskIlks, buf2);

    if (Hero.isConfused ()) {
        shObjectIlk *confilk;
        do {
            confilk = pickAnIlk (&FloppyDiskIlks);
        } while (confilk == disk->mIlk || confilk == newilk);
        newilk = confilk;
        if (Hero.isBlind ()) {
            I->p ("You mispronounce some of the commands in your confusion.");
        } else {
            I->p ("You make a few typos in your confusion.");
        }
        score = 100; /* always successful */
    } else {
        if (!newilk) {
            I->p ("There is no such program!");
            return -1;
        } else if (BlankDisk == newilk) {
            I->p ("Don't be ridiculous!");
            return -1;
        }
    
        score = sportingD20 () + Hero.getSkillModifier (kHacking);
        if (disk->isOptimized ()) { 
            score += 2;
        } else if (disk->isBuggy ()) {
            score -= 10;
        }
        if (newilk->mFlags & kIdentified) {
            score += 4;
        } else if (SoftwareEngineer != Hero.mProfession) {
            score -= 4;
        }
    }
    dc = 10 + newilk->mCost / 10;
    if (score >= dc) {
        disk = Hero.removeOneObjectFromInventory (disk);
        disk->mIlk = newilk;
        if (0 == Hero.addObjectToInventory (disk)) {
            I->p ("The disk slips from your hands!");
            Level->putObject (disk, Hero.mX, Hero.mY);
        }
    } else {
        I->p ("Your hacking attempt was a failure.");
        return 1;
    }
    return -1;
}


void
identifyObjects (int howmany)
{
    shObject *obj;
    int i;
    int didsomething = 0;
    const char *prompt = "What do you want to identify?";

    while (howmany) {
        shMenu idmenu (prompt, shMenu::kCategorizeObjects);
        shObjectVector v;

        selectObjectsByFunction (&v, Hero.mInventory, 
                                 &shObject::isUnidentified);
        if (0 == v.count ()) {
            I->p ("You have %s unidentified items.", 
                  didsomething ? "no more" : "no");
            return;
        }
        if (didsomething) {
            I->pause ();
        }
        for (i = 0; i < v.count (); i++) {
            obj = v.get (i);
            idmenu.addItem (obj->mLetter, obj->inv (), obj, obj->mCount);
        }
        idmenu.getResult ((const void **) &obj);
        if (obj) {
            ++didsomething;
            --howmany;
            obj->identify ();
            I->p ("%c - %s", obj->mLetter, obj->inv ());
            prompt = "What do you want to identify next?";
        }
    }
}


static int
doIdentify (shObject *computer, shObject *disk)
{
    int howmany = disk->isOptimized () ? RNG (1, 5) : 1;

    disk->setIlkKnown ();
    I->p ("This is an identify program.");
    I->pause ();
    identifyObjects (howmany);
    if (howmany > 1)
        disk->setBugginessKnown ();
    return 0;
}


static int
doSpam (shObject *computer, shObject *disk)
{
    static const char *spammsg [] = {
        "Warning!  Your warpspace connection is not optimized!",
        "Make thousands of buckazoids per cycle from the comfort of your own pod!",
        "Beautiful Arcturian babes are waiting to meet you!"
    };
    static const char *spamprompt [] = {
        "Transmit %d buckazoids to Meta-Net Systems for an upgrade?",
        "Upload $%d to Hyperpyramid Industries to buy business plan?",
        "Beam over your Arcturian bride for only %d buckazoids?"
    };

    int idx = RNG (3);
    int price = RNG (500) + 500;
    int sucker = 0;

    if (price > Hero.countMoney ()) {
        price = Hero.countMoney ();
    }
    I->p ("%s", spammsg[idx]);

    if (price < 50) {
        I->p ("Run this disk again when you've got some more buckazoids.");
        disk->maybeName ();
        return -1;
    }

    disk->setIlkKnown ();

    if (disk->isBuggy ()) {
        I->p ("Transmitting %d buckazoids through Zero-Click purchase plan...",
              price);
        disk->setBugginessKnown ();
        sucker = 1;
    } else {
        if (I->yn (spamprompt[idx], price)) {
            sucker = 1;
        } else {
            disk->setIlkKnown ();
        }
    }

    if (sucker) {
        Hero.loseMoney (price);
        I->p ("Thank you for your order.  "
              "Please wait 4-6 orbit cycles for delivery.");
    } else {
        I->p ("Perhaps you might be interested in some of our other products?");
    }
    return -1;
}


static int
doDetectObject (shObject *computer, shObject *disk)
{
    int x; int y;
    disk->setIlkKnown ();
    I->p ("You scan the area for objects.");

    if (Hero.isBlind ()) {
        I->pause ();
        I->p ("But you can't see the display!");
        return 0;
    }

    for (y = 0; y < MAPMAXROWS; y++) {
        for (x = 0; x < MAPMAXCOLUMNS; x++) {
            shObjectVector *v = Level->getObjects (x, y);
            if (NULL != v && v->count ()) {
                int i;
                int besttype = kMaxObjectType;
                for (i = 0; i < v->count (); i++) {
                    if (v->get (i) -> mIlk -> mType < besttype) {
                        besttype = v->get (i) -> mIlk -> mType;
                    }
                }
                Level->setMemory (x, y, ObjectGlyphs[besttype].mChar);
                Level->drawSq (x, y);
            }
        }
    }
    I->pauseXY (Hero.mX, Hero.mY);

    return 0;
}


/* no longer it's own program - it's the result of a confused detect life*/
static int
doDetectDroids (shObject *computer, shObject *disk)
{
    int x, y, cnt;
    cnt = 0;
    disk->setIlkKnown ();
    I->p ("You scan the area for droids.");
    if (disk->isBuggy ()) {
        I->pause ();
        I->p ("These aren't the droids you're looking for.");
        return 0;
    }
    for (y = 0; y < MAPMAXROWS; y++) {
        for (x = 0; x < MAPMAXCOLUMNS; x++) {
            if (Level->isOccupied (x, y)
                && Level->getCreature (x, y) -> isRobot ()) 
            {
                if (!Hero.isBlind ()) {
                    Level->drawSqCreature (x, y);
                }
                cnt++;
            }
        }
    }
    I->pauseXY (Hero.mX, Hero.mY);
    if (Hero.isBlind ()) {
        I->p ("But you can't see the display!");
    }
    I->p ("%d droid%sdetected.", cnt, cnt > 1 ? "s " : " ");
    return 0;
}


static int
doDetectLife (shObject *computer, shObject *disk)
{
    int x, y, cnt;
    cnt = 0;
    disk->setIlkKnown ();
    if (Hero.isConfused ()) {
        return doDetectDroids (computer, disk);
    }
    I->p ("You scan the area for lifeforms.");
    if (disk->isBuggy ()) { /* you detect yourself */
        I->p ("1 lifeform detected.");
        I->pauseXY (Hero.mX, Hero.mY);
        return 0;
    }

    for (y = 0; y < MAPMAXROWS; y++) {
        for (x = 0; x < MAPMAXCOLUMNS; x++) {
            if (Level->isOccupied (x, y)
                && Level->getCreature (x, y) -> isAlive ()) 
            {
                if (!Hero.isBlind ()) {
                    Level->drawSqCreature (x, y);
                }
                cnt++;
            }
        }
    }

    I->pauseXY (Hero.mX, Hero.mY);
    if (Hero.isBlind ()) {
        I->p ("But you can't see the display!");
    }
    I->p ("%d lifeform%sdetected.", cnt, cnt > 1 ? "s " : " ");
    return 0;
}


static int
doDetectTraps ()
{
    int i;
    int cnt = 0;

    I->p ("You scan the area for traps.");
    for (i = 0; i < Level->mFeatures.count (); i++) {
        shFeature *f = Level->mFeatures.get (i);
        if (f->isTrap ()) {
            if (f->isBerserkDoor ()) {
                continue; /* not a trap, a malfunction */
            }
            if (!Hero.isBlind ()) {
                f->mTrapUnknown = 0;
                Level->drawSqTerrain (f->mX, f->mY);
            }
            ++cnt;
        }
    }
    if (Hero.isBlind ()) {
        I->p ("But you can't see the display!");
    }
    I->p ("%d trap%sdetected.", cnt, cnt > 1 ? "s " : " ");
    return 0;
}


static int
doMapping (shObject *computer, shObject *disk)
{
    disk->setIlkKnown ();
    if (Hero.isConfused ()) {
        return doDetectTraps ();
    }
    I->p ("A map appears on your screen!");
    if (Hero.isBlind ()) {
        I->p ("But you can't see the display!");
        return 0;
    }
    Level->reveal ();
    return 0;
}


static int
doDetectBugs (shObject *computer, shObject *disk)
{
    shObject *obj;
    int btotal = 0;
    int i;

    disk->setIlkKnown ();
    if (Hero.isConfused ()) {
        int x, y;
        I->p ("You scan the area for bugs.");
        for (y = 0; y < MAPMAXROWS; y++) {
            for (x = 0; x < MAPMAXCOLUMNS; x++) {
                if (Level->isOccupied (x, y)
                    && Level->getCreature (x, y) -> isInsect ()) 
                {
                    if (!Hero.isBlind ()) {
                        Level->drawSqCreature (x, y);
                    }
                    btotal++;
                }
            }
        }
        I->pauseXY (Hero.mX, Hero.mY);
        if (Hero.isBlind ()) {
            I->p ("But you can't see the display!");
        }
    } else if (disk->isBuggy ()) {
        disk->setBugginessKnown ();
        I->p ("This disk is buggy!");
        return 0;
    } else {
        I->p ("Scanning inventory for bugs...");
        for (i = 0; i < Hero.mInventory->count (); i++) {
            obj = Hero.mInventory->get (i);
            if (obj->isBuggy ()) {
                obj->setBugginessKnown ();
                btotal++;
            } else if (disk->isOptimized ()) {
                obj->setBugginessKnown ();
            }
        }
    }
    I->p ("%d bugs found.", btotal);

    return 0;
}


static int
doDebug (shObject *computer, shObject *disk)
{
    shObject *obj;
    shObjectVector v;

    if (computer->isBuggy ()) {
        computer->setDebugged ();
        disk->setIlkKnown ();
        I->p ("%s seems to working much better now.", computer->your ());
        computer->setBugginessKnown ();
        return 0;
    }
    if (Hero.isConfused ()) {
        disk->setIlkKnown ();
        disk->resetBugginessKnown ();
        if (disk->isBuggy ()) {
            disk->setDebugged ();
            I->p ("You patch some bugs in %s.", disk->your ());
        } else if (disk->isOptimized ()) {
            int i;
            selectObjectsByFunction (&v, Hero.mInventory, &shObject::isBuggy);
            for (i = 0; i < v.count (); i++) {
                obj = v.get (i);
                obj->setDebugged ();
            }
            I->p ("%d objects debugged.", i);
        } else {
            disk->setOptimized ();
            I->p ("You optimize %s.", disk->your ());
        }
        disk->setBugginessKnown ();
        return 0;
    } else if (disk->isOptimized ()) {
        disk->setIlkKnown ();
        I->p ("You've found a floppy disk of debugging!");
        obj = Hero.quickPickItem (Hero.mInventory, "debug", 
                                  shMenu::kCategorizeObjects);
        if (NULL == obj) {
            return 0;
        }
        obj->resetBugginessKnown ();
        if (obj->isBuggy ()) {
            obj->setDebugged ();
            I->p ("You patch some bugs in %s.", obj->your ());
        } else if (obj->isOptimized ()) {
            I->p ("You verify that %s is fully optimized.", obj->your ());
        } else {
            obj->setOptimized ();
            I->p ("You optimize %s.", obj->your ());
        }
        obj->setBugginessKnown ();
        return 0;
    } else {
        shObjectVector v2;
        selectObjectsByFunction (&v, Hero.mInventory, &shObject::isWorn);
        selectObjectsByFunction (&v2, &v, &shObject::isBuggy);
        if (Hero.mWeapon && Hero.mWeapon->isBuggy ()) {
            v2.add (Hero.mWeapon);
        }
        if (v2.count ()) {
            obj = v2.get (RNG (v2.count ()));
            obj->setDebugged ();
            obj->resetBugginessKnown ();
            I->p ("Debugging %s.", obj->an ());
            obj->setBugginessKnown ();
            disk->setIlkKnown ();
        } else {
            disk->setIlkKnown ();
            I->p ("No showstopper bugs found.");
        }
        return 0;
    }       
}


static int
doEnhanceArmor (shObject *computer, shObject *disk)
{
    shObject *obj;
    shObjectVector v, w;

    selectObjects (&v, Hero.mInventory, kArmor);
    selectObjectsByFunction (&w, &v, &shObject::isWorn);
    v.reset ();
    selectObjectsByFunction (&v, &w, &shObject::isEnhanceable);
    if (0 == v.count ()) {
        I->p ("Your skin tingles for a moment.");
        disk->maybeName ();
        return 0;
    }
    obj = v.get (RNG (v.count ()));
    const char *your_armor = obj->your ();

    if (Hero.isConfused ()) {
        if (kNoEnergy != obj->vulnerability ()) {
            obj->setFooproof ();
            obj->setFooproofKnown ();
        }
        I->p ("%s vibrates.", your_armor);
        if (obj->mDamage) {
            obj->mDamage = 0;
            if (!Hero.isBlind ()) {
                I->p ("%s looks as good as new!", your_armor);
            }
        }
    } else if (disk->isBuggy ()) {
        I->p ("Blue smoke billows from %s.", your_armor);
        --obj->mEnhancement;
        obj->setBuggy ();
        obj->setBugginessKnown ();
        disk->setBugginessKnown ();
    } else if (disk->isOptimized ()) {
        int bonus = (6 - obj->mEnhancement) / 2;
        bonus = bonus <= 0 ? 0 : RNG (1, bonus);
        obj->mEnhancement += bonus;
        obj->setOptimized ();
        if (bonus > 1) {
            disk->setBugginessKnown ();
            obj->setBugginessKnown ();
        }
        I->p ("%s feels warm for %s.", your_armor, bonus > 1 ? "a while" : 
              bonus > 0 ? "a moment" : "an instant");
    } else {
        if (obj->mEnhancement < 3) {
            ++obj->mEnhancement;
            I->p ("%s feels warm for a moment.", your_armor);
        } else {
            I->p ("%s feels warm for an instant.", your_armor);
        }
        if (obj->isBuggy ()) {
            obj->setDebugged ();
        }
    }
    disk->setIlkKnown ();
    Hero.computeAC ();
    return 0;
}


static int
doEnhanceImplant (shObject *computer, shObject *disk)
{
    shObject *obj;
    shObjectVector v, w;

    selectObjects (&v, Hero.mInventory, kImplant);
    selectObjectsByFunction (&w, &v, &shObject::isWorn);
    v.reset ();
    selectObjectsByFunction (&v, &w, &shObject::isEnhanceable);
    if (0 == v.count ()) {
        I->p ("Your brain itches for a moment.");
        disk->maybeName ();
        return 0;
    }
    obj = v.get (RNG (v.count ()));
    
    if (disk->isBuggy ()) {
        I->p ("You feel a stinging sensation in your %s.", 
              describeImplantSite (obj->mImplantSite));
        --obj->mEnhancement;
        obj->setBuggy ();
        obj->setBugginessKnown ();
        disk->setBugginessKnown ();
    } else if (disk->isOptimized ()) {
        int bonus = (6 - obj->mEnhancement) / 2;
        bonus = bonus <= 0 ? 0 : RNG (1, bonus);
        obj->mEnhancement += bonus;
        I->p ("You feel a %s sensation in your %s.",
              bonus > 1 ? "buzzing" : 
              bonus > 0 ? "tingling" : "brief tingling",
              describeImplantSite (obj->mImplantSite));
    } else if (obj->mEnhancement < 3) {
        ++obj->mEnhancement;
        I->p ("You feel a tingling sensation in your %s.",
              describeImplantSite (obj->mImplantSite));
    } else {
        I->p ("You feel a brief tingling sensation in your %s.",
              describeImplantSite (obj->mImplantSite));
    }
    disk->setIlkKnown ();
    Hero.computeIntrinsics ();
    return 0;
}


static int
doEnhanceWeapon (shObject *computer, shObject *disk)
{
    shObject *obj;
    shObjectVector v;
    const char *your_weapon;

    if (NULL == Hero.mWeapon) {
        I->p ("Your hands sweat profusely.");
        return 0;
    }

    obj = Hero.mWeapon;
    your_weapon = obj->your ();
    if (!obj->isA (kWeapon)) {
        I->p ("Your hands sweat profusely.");
        return 0;
    }
    if (Hero.isConfused ()) {
        if (kNoEnergy != obj->vulnerability ()) {
            obj->setFooproof ();
            obj->setFooproofKnown ();
        }
        I->p ("%s vibrates", your_weapon);
        if (obj->mDamage) {
            obj->mDamage = 0;
            if (!Hero.isBlind ()) {
                I->p ("%s looks as good as new!", your_weapon);
            }
        }
    } else if (disk->isBuggy ()) {
        I->p ("%s shudders%s", your_weapon, 
              obj->isBuggy () ? "!" : " and welds to your hand!");
        --obj->mEnhancement;
        obj->setBuggy ();
        obj->setBugginessKnown ();
        disk->setBugginessKnown ();
    } else if (disk->isOptimized ()) {
        int bonus = (6 - obj->mEnhancement) / 2;
        bonus = bonus <= 0 ? 0 : RNG (1, bonus);
        obj->setOptimized ();
        obj->mEnhancement += bonus;
        I->p ("%s feels warm for %s.", your_weapon, 
              bonus > 1 ? "a while" : 
              bonus > 0 ? "a moment" : "an instant");
    } else {
        if (obj->mEnhancement < 3) {
            ++obj->mEnhancement;
            I->p ("%s feels warm for a moment.", your_weapon);
        } else {
            I->p ("%s feels warm for an instant.", your_weapon);
        }
        if (obj->isBuggy ()) {
            obj->setDebugged ();
        }
    }
    disk->setIlkKnown ();
    return 0;
}


static int
doHypnosis (shObject *computer, shObject *disk)
{
    if (Hero.isBlind ()) {
        I->p ("You listen to some excellent trance music.");
        disk->maybeName ();
        return 0;
    } else {
        I->p ("You are mesmerized by a hypnotic screensaver!");
    }

    if (disk->isBuggy ()) {
        Hero.makeAsleep (1000 * NDX (10, 12));
    } else if (disk->isOptimized ()) {
        Hero.makeAsleep (1000 * NDX (4, 6));
    } else {
        Hero.makeAsleep (1000 * NDX (6, 10));
    }

    disk->setIlkKnown ();
    return 0;
}


/* returns 0 on success
           1 if the creature dies
           -1 if otherwise failed
*/
int
shCreature::transport (int x, int y, int safe)
{
    int res;

    if (-1 == x) {
        mLevel->findUnoccupiedSquare (&x, &y);
    }
    if (isHero ()) {
        if (Level->noTransport ()) {
            if (Hero.isBlind ()) 
                I->p ("You shudder for a moment.");
            else 
                I->p ("You flicker for a moment.");
            return 0;
        }
        Hero.oldLocation (x, y, mLevel);
    }
    //if (!mLevel->removeCreature (this)) {
    //    return -1;
    //}
    mTrapped = 0;
    mDrowning = 0;
    while (safe--) {
        res = mLevel->moveCreature (this, x, y);
        if (-1 == res) {
            /* try again, random location */
            mLevel->findUnoccupiedSquare (&x, &y);
        } else {
            if (isHero ())
                Level->attractWarpMonsters (x, y);
            return res;
        }
    }
    die (kTransporterAccident);
    return 1;
}


static int
doTransport (shObject *computer, shObject *disk)
{
    int x = -1, y = -1;

    disk->setIlkKnown ();
    if (disk->isBuggy ()) {
        /* unsafe random local transport */
        Level->findSquare (&x, &y);
        Hero.transport (x, y, 5);
        return 0;
    } else if (disk->isOptimized () && !Hero.isConfused ()) {
        /* controlled local transport */
        if (I->getSquare ("Transport to what location?", &x, &y, -1)) {
            if (Level->isMainframe ()) {
                int tries = 50;
                int nx, ny;
                do {
                    nx = x + RNG (15) - 8;
                    ny = y + RNG (9) - 4;
                    Level->findNearbyUnoccupiedSquare (&nx, &ny);
                } while (nx == x && ny == y && tries--);
                I->p ("It's hard to transport accurately in here.");
                x = nx; y = ny;
            }
            Hero.transport (x, y, 100);
        }
        return 0;
    } else {
        /* random local transport */
        Level->findSquare (&x, &y);
        Hero.transport (x, y, 100);
        return 0;
    }
}


static const char *
energyDescription (shEnergyType t) {
    switch (t) {

    case kSlashing: return "slashing damage";
    case kPiercing: return "piercing damage";
    case kConcussive: return "concussive damage";
    case kBlinding: return "blinding";
    case kBurning: return "fire";
    case kConfusing: return "confusion";
    case kCorrosive: return "acid damage";
    case kElectrical: return "electrical damage";
    case kForce: return "force effects";
    case kLaser: return "laser damage";
    case kFreezing: return "cold damage";
    case kMagnetic: return "magnetic effects";
    case kParalyzing: return "paralysis";
    case kPoisonous: return "poison";
    case kRadiological: return "radiation";
    case kPsychic: return "psychic attacks";
    case kStunning: return "stunning";

    case kNoEnergy:
    case kBrainExtracting:
    case kBugging:
    case kChoking:
    case kCreditDraining:
    case kDisarming:
    case kDisintegrating:
    case kFaceHugging:
    case kHealing:
    case kHosing:
    case kRestoring:
    case kSickening:
    case kTimeWarping:
    case kTransporting:
    case kViolating:
    case kWebbing:
    case kMaxEnergyType:
        return NULL;
    }
    return NULL;
}


void
shHero::doDiagnostics ()
{
    int tmp;
    char buf[60];
    shMenu menu ("Personal diagnostics", shMenu::kNoPick);

    I->p ("Initiating body scan.");
    I->pauseXY (Hero.mX, Hero.mY);
    {
        int e;
        int resist;
        const char *description;

        snprintf (buf, 60, "Your Base Attack Bonus is %d.", mBAB);
        menu.addItem (' ', buf, NULL, 1);
        if (mToHitModifier) {
            snprintf (buf, 60, "Your To Hit Modifier is %d.", mToHitModifier);
            menu.addItem (' ', buf, NULL, 1);
        }
        if (mDamageModifier) {
            snprintf (buf, 60, "Your Damage Modifier is %d.", mDamageModifier);
            menu.addItem (' ', buf, NULL, 1);
        }

        for (e = kNoEnergy; e < kMaxEnergyType; e++) {
            resist = Hero.getResistance ((shEnergyType) e);
            description = energyDescription ((shEnergyType) e);
            if (resist && description) {
                snprintf (buf, 60, "You are %s to %s.", 
                                   resist < 0  ? "particularly vulnerable" :
                                   resist < 5  ? "somewhat resistant" :
                                   resist < 10 ? "resistant" :
                                   resist < 50 ? "extremely resistant" :
                                                 "nearly invulnerable",
                                   description);
                menu.addItem (' ', buf, NULL, 1);
            }
        }

        if (Hero.hasTelepathy ()) {
            menu.addItem (' ', "You are telepathic.", NULL, 1);
        }
        if (Hero.hasXRayVision ()) {
            menu.addItem (' ', "You have X-ray vision.", NULL, 1);
        }
        if (Hero.hasNightVision ()) {
            menu.addItem (' ', "You have night vision.", NULL, 1);
        }
        if (Hero.hasReflection ()) {
            menu.addItem (' ', "You have reflection.", NULL, 1);
        }
        if (Hero.hasTranslation ()) {
            menu.addItem (' ', "You understand alien languages.", NULL, 1);
        }
        if (Hero.hasNarcolepsy ()) {
            menu.addItem (' ', "You are narcoleptic.", NULL, 1);
        }
        if (Hero.hasHealthMonitoring ()) {
            menu.addItem (' ', "You monitor your health.", NULL, 1);
        }
        if (Hero.hasRadiationProcessing ()) {
            menu.addItem (' ', "Your body eliminates radiation.", NULL, 1);
        }
        if (Hero.hasShield ()) {
            menu.addItem (' ', "You are shielded.", NULL, 1);
        }
        if (Hero.hasBrainShield ()) {
            menu.addItem (' ', "Your have psionic protection.", NULL, 1);
        }
        if (Hero.hasAirSupply ()) {
            menu.addItem (' ', "You have an air supply.", NULL, 1);
        }
        if (Hero.hasCrazyIvan ()) {
            menu.addItem (' ', 
                          "Your brain's hemispheres are reversed.", NULL, 1);
        }
        if (Hero.hasAutoRegeneration ()) {
            menu.addItem (' ', "Your body regenerates quickly.", NULL, 1);
        }
        if (Hero.isLucky ()) {
            menu.addItem (' ', "You are lucky.", NULL, 1);
        }

        snprintf (buf, 60, "You've been exposed to %s radiation.",
                  Hero.mRad > 300 ? "lethal levels of" :
                  Hero.mRad > 200 ? "very dangerous levels of" :
                  Hero.mRad > 125 ? "dangerous levels of" :
                  Hero.mRad > 75 ? "significant levels of" :
                  Hero.mRad > 50 ? "moderate levels of" :
                  Hero.mRad > 25 ? "acceptable levels of" :
                  "minimal levels of");
        menu.addItem (' ', buf, NULL, 1);

        if ((tmp = Hero.getStoryFlag ("impregnation"))) {
            if (tmp > 15) {
                menu.addItem (' ', "The alien lifeform inside you"
                              " will soon kill you.", NULL, 1);
            } else {
                menu.addItem (' ' , "You've been impregnated with "
                              "a dangerous alien lifeform.", NULL, 1);
            }
        }
        if (Hero.getStoryFlag ("superglued tongue")) {
            menu.addItem (' ', "The canister of superglue will eventually"
                          " come off.", NULL, 1);
        }
    }
    
    menu.finish ();
}


static int
doDiagnostics (shObject *computer, shObject *disk)
{
    disk->setIlkKnown ();
    Hero.doDiagnostics ();
    return 0;
}


static int
doHacking (shObject *computer, shObject *disk)
{
    shObject *obj;
    shObjectVector v;
    int sk;
    int score;

    disk->setIlkKnown ();
    I->p ("You've found a floppy disk of hacking!");
    selectObjects (&v, Hero.mInventory, kFloppyDisk);
    obj = Hero.quickPickItem (&v, "hack", 0);
    if (NULL == obj) {
        return 0;
    }

    if (obj->isCracked ()) {
        obj->setCrackedKnown ();
        I->p ("This software has already been cracked!");
        return 0;
    }
    sk = Hero.getSkillModifier (kHacking);
    score = sportingD20 () + sk;
    if (disk->isOptimized ()) { 
        score += 2;
    } else if (disk->isBuggy ()) {
        score -= 10;
    }
    if (obj->isA ("floppy disk of hacking")) {
        score -= 50; /* this is nearly impossible */
    }
    if (obj->isOptimized () || obj->isBuggy ()) {
        /* debugged code is more straightforward to hack */
        score -= 2;
    }
    if (score >= 15) {
        I->p ("You remove the copy protection from %s.", obj->your ());
        obj->setCracked ();
        obj->setCrackedKnown ();
        Hero.exerciseSkill (kHacking, 3);
    } else {
        I->p ("Your hacking attempt was a failure!");
        obj->setCrackedKnown ();
        //obj->resetBugginessKnown ();
        if (score <= 10) {
            obj->setBuggy ();
        }
    }   
    return 0;
}


void
attractLawyers ()
{
    int i;
    int x, y;
    int cnt = 0;

    for (i = 0; i < Level->mCrList.count () ; i++) {
        shCreature *c = Level->mCrList.get (i);
        if (c->isLawyer ()) {
            if (c->canSee (&Hero)) {
                ((shMonster *) c) -> makeAngry ();
            } else if (cnt < 3) {
                x = Hero.mX;
                y = Hero.mY;
                if (0 == Level->findAdjacentUnoccupiedSquare (&x, &y)) {
                    c->transport (x, y, 1);
                    c->resetFleeing ();
                    cnt++;
                }
            }
        }
    }

    if (!cnt && !RNG (13)) {
        x = Hero.mX;
        y = Hero.mY;
        if (0 == Level->findAdjacentUnoccupiedSquare (&x, &y)) {
            shMonster *m = new shMonster (findAMonsterIlk ("lawyer"));
            if (0 == Level->putCreature (m, x, y)) {
                cnt++;
            }
        }
    }

    if (1 == cnt) {
        I->p ("You seem to have attracted the attention of a lawyer.");
    } else if (cnt > 1) {
        I->p ("You seem to have attracted the attention of some lawyers.");
    }

}


/* returns number of ms taken */
int
executeFloppyDisk (shObject *computer, shObject *disk)
{
    shFloppyDiskIlk *ilk = (shFloppyDiskIlk *) disk->mIlk;
    int result;
    int summonlawyer = 0;
    
    if (Hero.isBlind ()) {
        if (computer->isA ("mega computer")) {
            if (Hero.getStoryFlag ("superglued tongue")) {
                I->p ("But you're blind and mute!");
                return 0;
            } else {
                I->p ("You make use of your mega computer's voice interface.");
            }
        } else {
            I->p ("But you're blind, and this computer doesn't "
                  "have a voice interface.");
            return 0;
        }
    } else {
        /* I->p ("You insert the floppy disk into your computer."); */
    }

    disk->setAppearanceKnown ();

    if (Hero.isConfused () && !disk->isA (BlankDisk)) {
        if (Hero.isBlind ()) {
            I->p ("You mispronounce some of the commands in your confusion.");
        } else {
            I->p ("You make a few typos in your confusion.");
        }
    }

    if (Hero.isInShop ()) {
        Hero.usedUpItem (disk, 1, "use");
        disk->resetUnpaid ();
    }

    result = ilk->mUseFunc (computer, disk);

    if (-1 != result && disk->isCracked ()) {
        int danger = Hero.getStoryFlag ("software piracy");
        if (0 == danger) {
            Hero.setStoryFlag ("software piracy", 5);
        } else if (RNG (0, danger) && RNG (0, danger)) {
            summonlawyer = 1;
        }
    }
    
    if (1 == result) {
        I->p ("The floppy disk self destructs!");
        Hero.useUpOneObjectFromInventory (disk);
    } else if (0 == result &&
               !disk->isCracked () && 
               !RNG (3))
    {
        I->p ("Your license for this software has expired.");
        I->p ("The floppy disk self destructs!");
        Hero.useUpOneObjectFromInventory (disk);
    } else if (computer->isBuggy ()) {
        if (RNG (2)) {
            I->p ("Your computer eats the disk!");
            Hero.useUpOneObjectFromInventory (disk);
            computer->setBugginessKnown ();
        } else if (RNG (2)) {
            disk->setBuggy ();
        }
    } else if (disk->isCracked () && !disk->isCrackedKnown ()) {
        I->p ("The licence for this software has been cracked!");
        disk->setCrackedKnown ();
    }

    if (summonlawyer) attractLawyers ();

    return computer->isOptimized () ? FULLTURN : LONGTURN;
}


shFloppyDiskIlk *GenericFloppyDisk;

void
initializeFloppyDisks ()
{
    int n = 0;

    /* each price group should have one doozy */

    GenericFloppyDisk = 
        new shFloppyDiskIlk ("floppy disk", "floppy disk", NULL, 0, 0);
        
    BugDetectDisk = 
    new shFloppyDiskIlk ("floppy disk of bug detection", 
                         FloppyData[n++].mDesc, 
                         &doDetectBugs, 25, 50);
    new shFloppyDiskIlk ("floppy disk of identify", FloppyData[n++].mDesc, 
                         &doIdentify, 25, 150);
    SpamDisk =
    new shFloppyDiskIlk ("floppy disk of spam", FloppyData[n++].mDesc, 
                         &doSpam, 25, 150);
    

    BlankDisk = 
    new shFloppyDiskIlk ("blank floppy disk", "unlabeled floppy disk",
                         &doBlank, 50, 35);
    new shFloppyDiskIlk ("floppy disk of object detection", 
                         FloppyData[n++].mDesc, 
                         &doDetectObject, 50, 50);
    new shFloppyDiskIlk ("floppy disk of lifeform detection", 
                         FloppyData[n++].mDesc, &doDetectLife, 50, 50);
/* now the result of confused lifeform detection:
    new shFloppyDiskIlk ("floppy disk of droid detection", 
                         FloppyData[n++].mDesc, 
                         &doDetectDroids, 50, 40);
*/
    new shFloppyDiskIlk ("floppy disk of mapping", FloppyData[n++].mDesc, 
                         &doMapping, 50, 70);
    new shFloppyDiskIlk ("floppy disk of diagnostics", 
                         FloppyData[n++].mDesc, 
                         &doDiagnostics, 50, 20);

    new shFloppyDiskIlk ("floppy disk of debugging", FloppyData[n++].mDesc, 
                         &doDebug, 100, 100);
    new shFloppyDiskIlk ("floppy disk of enhance armor", 
                         FloppyData[n++].mDesc, 
                         &doEnhanceArmor, 100, 60);
    new shFloppyDiskIlk ("floppy disk of enhance weapon", 
                         FloppyData[n++].mDesc, 
                         &doEnhanceWeapon, 100, 70);
    new shFloppyDiskIlk ("floppy disk of enhance implant", 
                         FloppyData[n++].mDesc, 
                         &doEnhanceImplant, 100, 50);
    new shFloppyDiskIlk ("floppy disk of transportation", 
                         FloppyData[n++].mDesc, 
                         &doTransport, 100, 75);
    new shFloppyDiskIlk ("floppy disk of hypnosis", FloppyData[n++].mDesc, 
                         &doHypnosis, 100, 50);

/* TODO:
    new shFloppyDiskIlk ("floppy disk of gain level", FloppyData[n++].mDesc, 
                         &doGainLevel, 300, 20);
*/
    new shFloppyDiskIlk ("floppy disk of hacking", FloppyData[n++].mDesc, 
                         &doHacking, 300, 20);

}


shFloppyDiskIlk::shFloppyDiskIlk (const char *name, 
                                  const char *appearance, 
                                  shFloppyDiskFunc *usefunc,
                                  int cost,
                                  int prob)
{
    FloppyDiskIlks.add (this);

    mType = kFloppyDisk;
    mParent = GenericFloppyDisk;
    mName = name;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[kGray];
    mAppearance = appearance;
    mVagueName = "floppy disk";
    mMaterial = kPlastic;
    mFlags = kMergeable;
    mProbability = prob;
    mWeight = 18;
    mSize = kFine;
    mHardness = 5;
    mHP = 1;
    mCost = cost;
    mUseFunc = usefunc;
}
    

shObject *
createFloppyDisk (char *desc,
                  int count, int bugginess, int enhancement, int charges)
{
    shFloppyDiskIlk *ilk;
    shObject *floppy;

    ilk = (shFloppyDiskIlk *) (NULL == desc ? pickAnIlk (&FloppyDiskIlks) 
                                     : findAnIlk (&FloppyDiskIlks, desc));
    if (NULL == ilk) return NULL;

    if (count < 1) count = 1;

    if (-2 == bugginess) {
        int tmp = RNG (6);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
        if (SpamDisk == ilk && RNG (4)) {
            bugginess = -1;
        } else if (BugDetectDisk == ilk && RNG (2)) {
            bugginess = 1;
        }
    }

    floppy = new shObject ();
    floppy->mIlk = ilk;
    floppy->mCount = count;
    floppy->mBugginess = bugginess;
    floppy->mHP = ilk->mHP;
    floppy->setEnhancementKnown ();
    floppy->setCrackedKnown ();
    return floppy;
}



