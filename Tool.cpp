#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"

shVector <shObjectIlk *> ToolIlks;

shToolIlk *Computer;
shToolIlk *EnergyCell;
//shToolIlk *EnergyTank;
shToolIlk *PowerPlant;

shAttack ExplodingEnergyCellDamage = 
    shAttack (NULL, shAttack::kBlast, shAttack::kBurst, 0, kElectrical, 3, 6);


shAttack RestrainingBoltAttack = 
    shAttack (NULL, shAttack::kAttach, shAttack::kSingle, 0, kNoEnergy, 0, 1);


static int
useRestrainingBolt (shObject *bolt)
{
    shDirection dir;
    int x = Hero.mX;
    int y = Hero.mY;
    shMonster *c;
    char buf[80];
    int difficulty;

    dir = I->getDirection ();
    switch (dir) {
    case kNoDirection:
    case kUp:
    case kDown:
        return 0;
    default:
        if (!Level->moveForward (dir, &x, &y)) {
            return 0;
        }
        c = (shMonster *) Level->getCreature (x, y);
        if (!c || c->isAlive ()) {
            I->p ("Restraining bolts only work on bots and droids.");
            return 0;
        }
        c->the (buf, 80);
        if (c->isPet ()) {
            I->p ("You've already affixed a restraining bolt to %s.", buf);
            return 0;
        }

        /* low-level bots should be automatic, warbots should be quite  
           difficult to tame! */
        difficulty = bolt->isOptimized () ? 6 : 
                     bolt->isBuggy ()     ? 2 :
                                            3;

        if (c->mCLevel > difficulty && RNG (c->mCLevel) > difficulty) {
            I->p ("You miss %s.", buf);
            c->newEnemy (&Hero);
        } else {
            Hero.useUpOneObjectFromInventory (bolt);
            I->p ("You attach the restraining bolt to %s.", buf);
            if (c->isA ("clerkbot")) {
                if (1) { /* nice try */
                    I->p ("The restraining bolt is vaporized by %s's "
                          "anti-shoplifting circuits!", buf);
                }
                c->newEnemy (&Hero);
            } else {
                c->makePet ();
            }
        }
        return FULLTURN;
    }
}


int
makeRepair (shObject *tool)
{
    shDirection dir;
    int x = Hero.mX;
    int y = Hero.mY;
    shFeature *f;
    shMonster *c;
    shObjectVector *v;
    int score;
    char buf[80];

    score = sportingD20 () + Hero.getSkillModifier (kRepair);
    if (tool->isA ("roll of duct tape")) {
        /* at first I had this as a penalty, but duct-tape holds the universe 
           together, right?  Plus this might make for better game balance.  I think
           it's no fun if the one-time use items suck compared to reusable ones...
         */
        score += 2;
    } else if (tool->isA ("canister of super glue")) {
        /* eh... no modifier */
        tool->setIlkKnown ();
    }

    dir = I->getDirection ();
    switch (dir) {
    case kNoDirection:
        return 0;
    case kUp:
        I->p ("There's nothing to repair on the ceiling.");
        return 0;
    default:
        if (!Level->moveForward (dir, &x, &y)) {
            I->p ("There's nothing there to repair.");
            return 0;
        }
        if ((c = (shMonster *) Level->getCreature (x, y))) {
            /* repair a pet droid */
            c->the (buf, 80);
            if (c->isAlive ()) {
                I->p ("You can't make repairs to %s!", buf);
                return 0;
            } else if (c->mHP == c->mMaxHP) {
                I->p ("%s doesn't appear to be damaged.", buf);
                return 0;
            } else if (!c->isPet ()) {
                I->p ("%s won't cooperate with your attempt to repair it.",
                      buf);
                return FULLTURN;
            } else if (score < 15) {
                I->p ("Your repair attempt is a failure.");
            } else {
                c->mHP += score - 15 + RNG (1, 3);
                if (c->mHP >= c->mMaxHP) {
                    c->mHP = c->mMaxHP;
                    I->p ("Now %s looks as good as new!", buf);
                    Hero.exerciseSkill (kRepair, 3);
                } else {
                    I->p ("You repair some of the damage on %s.", buf);
                    Hero.exerciseSkill (kRepair, 1);
                }
            }
            goto done;
        }
        if ((v = Level->getObjects (x, y))) {
            int i;
            shObject *obj;
            int nx = x;
            int ny = y;

            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);
                if (obj->isA (&WreckIlk)) {
                    obj->the (buf, 80);

                    if (nx == Hero.mX && ny == Hero.mY &&
                        -1 == Level->findNearbyUnoccupiedSquare (&nx, &ny))
                    {
                        obj->an (buf, 80);
                        I->p ("There is %s here, "
                              "but you need more room to repair it.", buf);
                        break;
                    }
                    if (I->yn ("Repair %s?", buf)) {
                        if (score < 20) {
                            I->p ("Your repair attempt is a failure.");
                        } else {
                            shMonster *bot = new shMonster (obj->mCorpseIlk);

                            Hero.exerciseSkill (kRepair, 4);
                            Level->putCreature (bot, nx, ny);
                            I->p ("You bring %s back on-line!", buf);
                            v->remove (obj);
                            delete obj;
                        }
                        goto done;
                    }
                }
            }
        }

        if ((f = Level->getFeature (x, y))) {
            if (shFeature::kDoorOpen == f->mType &&
                shFeature::kBerserk & f->mDoor &&
                !f->mTrapUnknown)
            {
                if (score >= 12) {
                    Hero.exerciseSkill (kRepair, 2);
                    f->mDoor &= ~shFeature::kBerserk;
                    I->p ("You repair the malfunctioning door.");
                } else {
                    I->p ("You repair attempt is a failure.");
                }
                goto done;
            } else {
                I->p ("It ain't broke!");
                return 0;
            }
        } 
        I->p ("There's nothing there to repair.");
        return 0;
    }
    done:
    if (tool->isA ("roll of duct tape")) {
        /* superglue will get used up, too, but that's taken care of by
           the useCanister() function
         */
        Hero.useUpOneObjectFromInventory (tool);
    }
    /* yeah, realistically this should take longer, but that wouldn't
       be very fun for the player I think. */
    return FULLTURN; 
}


static int
useKey (shObject *key)
{
    shDirection dir;
    int x = Hero.mX;
    int y = Hero.mY;
    shFeature *f;

    int elapsed = 0;

    dir = I->getDirection ();
    switch (dir) {
    case kNoDirection:
        return 0;
    case kUp:
        I->p ("There's no lock on the ceiling.");
        return 0;
    default:
        if (!Level->moveForward (dir, &x, &y)) {
            I->p ("There's no lock there.");
            return 0;
        }
        if ((f = Level->getFeature (x, y)) && f->isDoor ()) {
            int locked = f->isLockedDoor ();
            shObjectIlk *cardneeded = f->keyNeededForDoor ();

            if (f->isLockBrokenDoor ()) {
                I->p ("The lock on this door is broken.");
                return QUICKTURN;
            }
            if (!cardneeded) { 
                I->p ("There is no lock on this door.");
                return QUICKTURN;
            }
            if (f->isOpenDoor ()) {
                I->p ("You have to close it first.");
                return 0;
            }
            if (!I->yn ("%s it?", locked ? "Unlock" : "Lock")) {
                return 0;
            }
            if (key->isA ("master keycard") ||
                key->isA (cardneeded))
            {
                I->p ("You swipe your keycard and the door %s.",
                      locked ? "unlocks" : "locks");
                elapsed = HALFTURN;
            } else if (key->isA ("lock pick")) {
                int score = sportingD20 () + Hero.getSkillModifier (kOpenLock);
                if (score >= 20) {
                    Hero.exerciseSkill (kOpenLock, 2);
                    I->p ("You run a bypass on the locking mechanism.");
                    elapsed = FULLTURN;
                } else {
                    I->p ("You fail to defeat the lock.");
                    return FULLTURN;
                }
            } else {
                I->p ("You need a %s to %s this door.", cardneeded->mName,
                      locked ? "unlock" : "lock");
                return QUICKTURN;
            }
            /* TODO: dramatic failure could result in a berserk door? */
            if (locked) {
                f->unlockDoor ();
            } else {
                f->lockDoor ();
            }
            return elapsed;
        }
        I->p ("There's no lock there.");
        return 0;
    }
}


static int
useComputer (shObject *computer)
{
    shObjectVector v;
    shObject *obj;

    selectObjects (&v, Hero.mInventory, kFloppyDisk);
    obj = Hero.quickPickItem (&v, "execute", 0);
    if (NULL == obj) {
        return 0;
    }
    
    return executeFloppyDisk (computer, obj);
}


static int
useTricorder (shObject *tricorder)
{
    shDirection dir;
    int x = Hero.mX;
    int y = Hero.mY;
    shMonster *c;
    char buf[80];

    dir = I->getDirection ();
    switch (dir) {
    case kNoDirection:
    case kUp:
    case kDown:
        return 0;
    default:
        if (!Level->moveForward (dir, &x, &y)) {
            return 0;
        }
        c = (shMonster *) Level->getCreature (x, y);
        if (!c) {
            I->p ("You detect no creature there.");
            return 0;
        };
        c->getDescription (buf, 80);
        c->mHidden = 0;

        I->p ("%s HP:%d/%d AC: %d", buf, c->mHP, c->mMaxHP, c->getAC ());
    }
    return 0; /* takes no time at all! (otherwise it would be pretty useless) */
}


/* returns ms elapsed */
static int
usePortableHole (shObject *obj)
{
    shDirection dir;
    int x = Hero.mX;
    int y = Hero.mY;
    shFeature *f;
    shCreature *c;

    dir = I->getDirection ();
    switch (dir) {
    case kNoDirection:
        return 0;
    case kUp:
        I->p ("You can't reach the ceiling.");
        return 0;
    case kDown:
    default:
        if (!Level->moveForward (dir, &x, &y)) {
            return 0;
        }
        if (!Level->isFloor (x, y)) {
            /* TODO: holes in walls, etc. */
            I->p ("Sorry, but this hole only works on floors.");
            return 0;
        }
        f = (shFeature *) Level->getFeature (x, y);
        if (f) {
            /* FIX: there might be a hidden feature there.  This "not enough
                    room" explanation is unsatisfying... */
            I->p ("There isn't enough room to lay it flat %s.",
                  x == Hero.mX && y == Hero.mY ? "here" : "there");
            return 0;
        }
        Hero.useUpOneObjectFromInventory (obj);
        Level->addTrap (x, y, shFeature::kHole);
        c = Level->getCreature (x, y);
        if (c && !c->isHero ()) {
            c->newEnemy (&Hero);
        }
        Level->checkTraps (x, y, 100);
    }
    return FULLTURN;
}



/* returns ms elapsed */
static int
useOnOffTool (shObject *gc)
{
    char buf[64];
    gc->your (buf, 64);
    if (gc->isActive ()) {
        I->p ("You turn off %s.", buf);
        gc->resetActive ();
    } else if (Hero.countEnergy () <= 0) {
        I->p ("You're out of juice!");
    } else {
        I->p ("You turn on %s.", buf);
        gc->setActive ();
    }
    Hero.computeIntrinsics ();
    return HALFTURN;
}


/* MODIFIES: may use up the tool and delete it.
   RETURNS: time elapsed*/
int
useTool (shObject *tool) 
{
    shToolIlk *ilk;

    assert (tool->isA (kTool));
    ilk = (shToolIlk *) tool->mIlk;
    return ilk->mUseFunc (tool);
}


void
initializeTools ()
{
    EnergyCell =
    new shToolIlk ("energy cell", "energy cell", "energy cell", 
                   kYellow, NULL, 1, kSteel,
                   kIdentified | kMergeable | kBugProof, 20, kTiny, 10, 1, 
                   0, NULL, 140);
/*
    EnergyTank =
    new shToolIlk ("energy tank", "energy tank", "energy tank",
                   kBrightYellow, NULL, 400, kSteel,
                   kIdentified | kBugProof | kChargeable, 1500, kSmall, 12, 12,
                   0, NULL, 20);
*/
    PowerPlant =
    new shToolIlk ("power plant", "power plant", "power plant",
                   kBrightRed, NULL, 1000, kPlasteel, 
                   kChargeable, 5000, kSmall, 12, 12, 
                   5000, NULL, 0);

    new shToolIlk ("fission power plant", "power plant", "power plant",
                   kBrightRed, PowerPlant, 1000, kPlasteel, 
                   kChargeable, 5000, kSmall, 12, 12, 
                   5000, NULL, 20);

    new shToolIlk ("fusion power plant", "power plant", "power plant",
                   kBrightRed, PowerPlant, 1000, kPlasteel, 
                   kChargeable, 5000, kSmall, 12, 12, 
                   2500, NULL, 20);
    
    new shToolIlk ("roll of duct tape", "roll of duct tape", "roll of duct tape",
                   kGray, NULL, 5, kPlastic,
                   kIdentified | kMergeable, 100, kFine, 5, 2,
                   0, makeRepair, 70);

    new shToolIlk ("monkey wrench", "wrench", "wrench", kBlue, NULL, 10, kSteel,
                   kIdentified, 250, kTiny, 10, 1, 
                   0, makeRepair, 30);
/*
    new shToolIlk ("pencil", "yellow pointy thing", "yellow pointy thing", 
                   kYellow, NULL, 1, kWood,
                   0, 20, kTiny, 1, 1, 
                   0, NULL, 5);
*/
    shToolIlk *flashlight = 
    new shToolIlk ("flashlight", "flashlight", "flashlight", 
                   kRed, NULL, 10, kPlastic,
                   kIdentified, 200, kTiny, 2, 2, 
                   180000, useOnOffTool, 50);
    flashlight->mInUseIntrinsics |= kLightSource;

    shToolIlk *geigercounter = 
    new shToolIlk ("geiger counter", "electronic gizmo", "electronic gizmo",
                   kBlue, NULL, 300, kSteel,
                   0, 700, kTiny, 10, 10,
                   20000, useOnOffTool, 50);
    geigercounter->mInUseIntrinsics |= kRadiationDetection;

    shToolIlk *motiontracker =
    new shToolIlk ("motion tracker", "electronic gizmo", "electronic gizmo",
                   kBlue, NULL, 400, kSteel,
                   0, 700, kTiny, 10, 10,
                   10000, useOnOffTool, 25);
    motiontracker->mInUseIntrinsics |= kMotionDetection;
/*
    new shToolIlk ("communicator", "communicator", "communicator", 
                   kWhite, NULL, 100, kSilicon,
                   kIdentified, 100, kTiny, 10, 4, 
                   0, NULL, 25);

    new shToolIlk ("business card", "business card", "business card",
                   kYellow, NULL, 1, kPaper,
                   kIdentified, 5, kFine, 1, 1, 
                   0, NULL, 100);
*/
    new shToolIlk ("red keycard", "keycard", "red keycard", 
                   kRed, NULL, 20, kPlastic,
                   0, 5, kFine, 5, 1,
                   0, useKey, 15);

    new shToolIlk ("green keycard", "keycard", "green keycard", 
                   kGreen, NULL, 20, kPlastic,
                   0, 5, kFine, 5, 1,
                   0, useKey, 15);

    new shToolIlk ("blue keycard", "keycard", "blue keycard",
                   kBlue, NULL, 20, kPlastic,
                   kIdentified, 5, kFine, 5, 1,
                   0, useKey, 15);

    new shToolIlk ("orange keycard", "keycard", "orange keycard",
                   kBrightRed, NULL, 20, 
                   kPlastic,
                   0, 5, kFine, 5, 1,
                   0, useKey, 15);

    new shToolIlk ("master keycard", "keycard", "purple keycard",
                   kBrightMagenta, 
                   NULL, 100, kPlastic,
                   0, 5, kFine, 5, 1,
                   0, useKey, 15);

    new shToolIlk ("lock pick", "lock pick", "lock pick",
                   kBlue, 
                   NULL, 200, kPlastic,
                   kIdentified, 60, kFine, 5, 5, 
                   0, useKey, 20);
                   
    new shToolIlk ("restraining bolt", "magnetic bolt", "magnetic bolt",
                   kBlue, NULL, 200, kSteel,
                   kIdentified, 50, kFine, 5, 1,
                   0, useRestrainingBolt, 60);
/*
    new shToolIlk ("scalpel", "scalpel", "scalpel", kCyan, NULL, 10, kSteel,
                   kIdentified, 50, kTiny, 10, 10,
                   0, NULL, 15);

    new shToolIlk ("mop", "mop", "mop", kGreen, NULL, 10, kSteel,
                   kIdentified, 600, kLarge, 10, 10,
                   0, NULL, 25);
*/
    Computer = 
    new shToolIlk ("computer", "computer", "computer", kGray, NULL, 0, kSilicon,
                       0, 0, kSmall, 10, 10, 0, NULL, 0);

    new shToolIlk ("mega computer", 
                   "mega computer", "mega computer",
                   kWhite,
                   Computer, 1000, kTitanium,
                   kIdentified, 1200, kSmall, 10, 6, 
                   0, useComputer, 20);

    new shToolIlk ("mini computer", "mini computer", 
                   "mini computer",
                   kWhite,
                   Computer, 500, kPlastic,
                   kIdentified, 1200, kSmall, 10, 4, 
                   0, useComputer, 40);

    new shToolIlk ("tricorder", "tricorder", "tricorder",
                   kWhite,
                   NULL, 200, kPlastic,
                   kIdentified, 150, kTiny, 10, 4,
                   0, useTricorder, 20);

    new shToolIlk ("portable hole", "portable hole", "portable hole",
                   kBlack,
                   NULL, 200, kWarpish,
                   kIdentified | kMergeable, 0, kSmall, 100, 4,
                   0, usePortableHole, 75);

    shToolIlk *rabbitfoot =
    new shToolIlk ("rabbit's foot", "rabbit's foot", "rabbit's foot",
                   kBlue, NULL, 400, kSteel,
                   kIdentified, 50, kTiny, 10, 10,
                   0, NULL, 0);
    rabbitfoot->mCarriedIntrinsics |= kLucky;


    new shToolIlk ("heap of space junk", "heap of space junk", "heap of space junk",
                   kBrown, NULL, 5, kPlasteel,
                   0, 50000, kLarge, 10, 50,
                   0, NULL, 285);

}


shToolIlk::shToolIlk (char *name, 
                      char *vaguename,
                      char *appearance, 
                      shColor color,
                      shToolIlk *parent,
                      int cost,
                      shMaterialType material,
                      int flags,
                      int weight,
                      shThingSize size,
                      int hardness, 
                      int hp,
                      int energyuse,
                      shToolFunc *usefunc,
                      int prob)
{
    ToolIlks.add (this);

    mType = kTool;
    mParent = parent;
    mName = name;
    mAppearance = appearance;
    mVagueName = vaguename;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[color];
    mMaterial = material;
    mCost = cost;
    mFlags = flags;
    mProbability = prob;
    mWeight = weight;
    mSize = size;
    mHardness = hardness;
    mHP = hp;
    mEnergyUse = energyuse;
    mUseFunc = usefunc;
}


shObject *
createTool (char *desc,
            int count, int bugginess, int enhancement, int charges)
{
    shToolIlk *ilk;
    shObject *tool;

    ilk = (shToolIlk *) (NULL == desc ? pickAnIlk (&ToolIlks) 
                                        : findAnIlk (&ToolIlks, desc));
    if (NULL == ilk) return NULL;


    if (-2 == bugginess) {
        int tmp = RNG (8);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
    }

    tool = new shObject ();

    tool->mIlk = ilk;
    if (EnergyCell == ilk) {
        bugginess = 0;
        tool->identify ();
    } else if (tool->isA (PowerPlant)) {
        tool->mCharges = RNG (1, 100);
        tool->setChargeKnown ();
    }
   
    tool->mCount = -22 == count ? EnergyCell == ilk ? NDX (15, 10) : 1 : count;
    tool->mBugginess = bugginess;
    tool->mHP = ilk->mHP;

    if (!tool->isChargeable ()) {
        tool->setChargeKnown ();
    }
    tool->setFooproofKnown ();
    
    return tool;
}
