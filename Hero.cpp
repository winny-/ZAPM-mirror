#include <math.h>
#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Profession.h"
#include "Creature.h"
#include "Interface.h"
#include "Game.h"

#include <ctype.h>

const char *
shHero::getDescription () 
{
    return "hero";
}


const char *
shHero::the () 
{
    return "you";
}


const char *
shHero::an () 
{
    return "you";
}


const char *
shHero::your () 
{
    return "you";
}


void
shHero::gainAbility ()
{
    shAbilityIndex choice;
    int menuresult = 0;
    
    while (!menuresult) {
        shMenu menu ("You may increase an ability:", 0);

        menu.addItem ('a', "Strength", (void *) kStr, 1);
        menu.addItem ('b', "Constitution", (void *) kCon, 1);
        menu.addItem ('c', "Agility", (void *) kAgi, 1);
        menu.addItem ('d', "Dexterity", (void *) kDex, 1);
        menu.addItem ('e', "Intelligence", (void *) kInt, 1);
        menu.addItem ('f', "Wisdom", (void *) kWis, 1);
        menu.addItem ('g', "Charisma", (void *) kCha, 1);

        menuresult = menu.getResult ((const void **) &choice, NULL);
    } 
    
    int oldmodifier = ABILITY_MODIFIER (mAbil.getByIndex (choice));
    mMaxAbil.changeByIndex (choice, 1);
    mAbil.changeByIndex (choice, 1);
    if (kCon == choice) {
        if (ABILITY_MODIFIER (mAbil.getByIndex (choice)) > oldmodifier) {
            mMaxHP += mCLevel;
            mHP += mCLevel;
        }
    }

    computeIntrinsics ();
    computeAC ();
    I->drawSideWin ();

}


void
shHero::earnScore (int points)
{
    mScore += points;
}


void
shHero::earnXP (int challenge)
{
    int old = mXP / 1000;
    int amount = challenge - mCLevel;

    if (-1 == challenge) {
        mXP = old * 1000 + 1000;
        levelUp ();
        return;
    }

    amount = (int) (125.0 * pow (2.0, (double) amount / 2.0));

    earnScore (amount);
    mXP += amount;
}

static int pickedupitem;


void
shHero::oldLocation (int newX, int newY, shMapLevel *newLevel)
{
    if (GameOver) 
        return;

    shRoom *oldroom = Level->getRoom (mX, mY); 
    shRoom *newroom = Level->getRoom (newX, newY); 

    if (oldroom == newroom) 
        return;

    if (mLevel->isInShop (mX, mY)) {
        leaveShop ();
    }
    if (Level->isInHospital (mX, mY)) {
        leaveHospital ();
    }
    if (Level->isInGarbageCompactor (mX, mY)) {
        leaveCompactor ();
    }
}


void
shHero::newLocation ()
{
    if (!mLastLevel) mLastLevel = Level;
    shRoom *oldroom = mLastLevel->getRoom (mLastX, mLastY); 
    shRoom *newroom = Level->getRoom (mX, mY); 

//    if (mLastLevel != Level || distance (this, mLastX, mLastY) > 10) 
    Level->computeVisibility ();

    I->drawScreen ();
    
    if (sewerSmells () && !isSickened ()) {
        I->p ("What a terrible smell!");
        makeSickened (2000);
    }

    if (newroom != oldroom) { 
        if (Level->isInShop (mX, mY)) {
            enterShop ();
        }
        if (Level->isInHospital (mX, mY)) {
            enterHospital ();
        }
        if (Level->isInGarbageCompactor (mX, mY)) {
            enterCompactor ();
        }
        
        if (shRoom::kNest == newroom->mType) {
            I->p ("You enter an alien nest!");
        }
    }

    pickedupitem = 0;
    if (Flags.mAutopickup) {
        shObjectVector *v = Level->getObjects (mX, mY);
        int i;
        int n;

        if (v) {
            n = v->count ();
            for (i = 0; i < n; i++) {
                shObject *obj = v->get (i);
                if (Flags.mAutopickupTypes[obj->mIlk->mType]) {
                    interrupt ();
                    if (!isBlind ()) obj->setAppearanceKnown ();
                    /* HACK: as discussed below (see case shInterface::kPickup:),
                       remove obj from floor before attempting to add to 
                       inventory: */
                    v->remove (obj); 
                    if (addObjectToInventory (obj)) {
                        --i;
                        --n;
                        pickedupitem++;
                        if (Level->isInShop (mX, mY)) {
                            pickedUpItem (obj);
                        }
                    } else {
                        /* END HACK: add the object back to the end of the 
                           vector, and don't look at it again: */
                        v->add (obj);
                        --n;
                    }
                }
            }
        }
    }
}


void
shHero::lookAtFloor (int menuok /* = 0 */ )
{
    int objcnt = Level->countObjects (mX, mY);
    shObjectVector *v;
    int i;
    shFeature *f = Level->getFeature (mX, mY);
    int feelobjs = 0;

    feel (mX, mY);

    if (0 == objcnt && !menuok && !pickedupitem) {
        return;
    }
    if (kSewage == Level->getSquare (mX, mY) -> mTerr) {
        if (0 == mZ && 0 == objcnt) {
            I->p ("You are knee deep in sewage."); 
        } 
    }
    if (f) {
        switch (f->mType) {
        case shFeature::kStairsUp:
            I->p ("There is a staircase up here."); break;
        case shFeature::kStairsDown:
            I->p ("There is a staircase down here."); break;
        case shFeature::kDoorOpen:
            I->p ("You are standing in an open doorway."); break;
        case shFeature::kVat:
            I->p ("There is a vat of sludge here."); break;
        case shFeature::kPit:
            if (isTrapped ()) { 
                I->p ("You are in a pit."); 
            } else {
                I->p ("There is a pit here.");
            }
            break;
        case shFeature::kAcidPit:
            if (isTrapped ()) { 
                I->p ("You are in an acid pit."); 
            } else {
                I->p ("There is an acid pit here.");
            }
            break;
        case shFeature::kSewagePit:
            if (isTrapped ()) { 
                I->p ("You are at the bottom of a pit of sewage."); 
            } else {
                I->p ("There is a pit of sewage here.");
            }
            break;
        case shFeature::kTrapDoor:
            I->p ("There is a trap door here."); break;
        case shFeature::kHole:
            I->p ("There is a hole here."); break;
        case shFeature::kWeb:
            if (isTrapped ()) 
                I->p ("You are stuck in a web.");
            else 
                I->p ("There is a web here.");
            break;
        case shFeature::kRadTrap:
            I->p ("There is a radiation trap here."); break;
        default:
            I->p ("There is a strange feature here.");
        }
        interrupt ();
    }
    if (0 == objcnt) return;
    if (Level->isWatery (mX, mY)) {
        if (kSewage == Level->getSquare (mX, mY) -> mTerr) {
            I->p ("You feel around in the sewage...");
            feelobjs = 1;
        } 
    } else if (isBlind ()) {
        if (isFlying ()) return;
        I->p ("You feel around the floor...");
        feelobjs = 1;
    } 
    interrupt ();
    v = Level->getObjects (mX, mY);

    for (i = 0; i < v->count (); i++) {
        shObject *obj = v->get (i);
        if (hasBugSensing ()) 
            obj->setBugginessKnown ();
        if (!feelobjs) 
            obj->setAppearanceKnown ();
    }

    if (objcnt > 4) {
        if (!menuok) {
            I->p ("There are several objects here.");
            return;
        }
        else {
            shMenu menu ("Things that are here:", 
                         shMenu::kNoPick | shMenu::kCategorizeObjects);
            shObject *obj;
            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);
                menu.addItem (' ', obj->an (), obj, obj->mCount);
            }
            menu.finish ();
            return;
        }
    }
    if (1 == objcnt) {
        I->p ("You %s %s.", feelobjs ? "find" : "see here", 
              v->get (0) -> an ());
        return;
    }
    I->p ("Things that are here:");
    for (i = 0; i < objcnt; i++) {
        I->p (v->get (i) -> an ());
    }
}


int
shHero::tryToTranslate (shCreature *c)
{
#define TALKERIDLEN 20
    char talkerid[TALKERIDLEN];
    const char *talker = c->the ();
    int i;

    snprintf (talkerid, TALKERIDLEN, "translate %p", c);

    if (hasTranslation ()) {
        assert (mImplants[shImplantIlk::kLeftEar]);

        if (1 != getStoryFlag (talkerid)) {
            I->p ("%s translates %s's %s:", 
                  mImplants[shImplantIlk::kLeftEar]->your (), talker, 
                  c->isRobot () ? "beeps and chirps" : "alien language");
            setStoryFlag (talkerid, 1);
            mImplants[shImplantIlk::kLeftEar]->setIlkKnown ();
        }
        return 1;
    }
    
    for (i = 0; i < mPets.count (); i++) {
        shMonster *m = (shMonster *) mPets.get (i);

        if (m->isA ("protocol droid") && 
            distance (m, c) < 100 && 
            distance (this, m) < 100) 
        {
            if (2 != getStoryFlag (talkerid)) {
                I->p ("%s translates %s's %s:", m->your (), talker,
                      c->isRobot () ? "beeps and chirps" : "alien language");
                setStoryFlag (talkerid, 2);
            }
            return 1;
        }
    }
    return 0;
}


int 
shHero::looksLikeJanitor ()
{
    return (Hero.mWeapon && Hero.mWeapon->isA ("mop") &&
            Hero.mJumpsuit && Hero.mJumpsuit->isA ("janitor uniform") &&
            (Janitor == Hero.mProfession || !Hero.mBodyArmor));
}


void
shHero::sensePeril ()
{
    int oldperil = mGoggles->isToggled ();
    int newperil = 0;
    shCreature *c;
    shFeature *f;
    int i;

    if (oldperil) {
        mGoggles->resetToggled ();
        computeIntrinsics ();
    }        

    for (i = 0; i < Level->mCrList.count (); i++) {
        c = Level->mCrList.get (i);
        if (c == this || !c->isHostile () || !canSee (c->mX, c->mY)) {
            continue;
        }
        newperil++;
        goto done;
    }

    for (i = 0; i < Level->mFeatures.count (); i++) {
        f = Level->mFeatures.get (i);
        if (f->isTrap () && canSee (f->mX, f->mY)) {
            newperil++;
            goto done;
        }
    }

done:
    const char *your_goggles = mGoggles->your ();
    if (!oldperil && newperil) {
        mGoggles->setToggled ();
        interrupt ();
        computeIntrinsics ();
        if (hasXRayVision ()) {
            I->p ("%s have darkened a bit.", your_goggles);
        } else {
            I->p ("%s have turned black!", your_goggles);
        }
    } else if (oldperil && !newperil) {
        mGoggles->resetToggled ();
        computeIntrinsics ();
        I->p ("%s have turned transparent.", your_goggles);
    } else if (oldperil) {
        mGoggles->setToggled ();
        computeIntrinsics ();
    }
}


void
shHero::spotStuff ()
{
    shCreature *c;
    shFeature *f;
    int i;
    int sk = getSkillModifier (kSpot);
    int score;

    if (hasPerilSensing ()) {
        sensePeril ();
    }
    
    for (i = 0; i < Level->mCrList.count (); i++) {
        c = Level->mCrList.get (i);
        if (c == this || !c->isHostile () || !canSee (c->mX, c->mY)) {
            continue;
        }
        if (canSee (c->mX, c->mY) &&
            c->mHidden > 0 && c->mSpotAttempted + 160000 < Clock &&
            distance (this, c->mX, c->mY) < 30) 
        {
            c->mSpotAttempted = Clock;
            score = sportingD20 () + sk;
            I->debug ("spot check: %d", score);
            if (score > c->mHidden) {
                I->p ("You spot %s!", c->an ());
                c->mHidden *= -1;
                Level->drawSq (c->mX, c->mY, 1);
                I->pauseXY (c->mX, c->mY);
                interrupt ();
            }
        }
    }

    for (i = 0; i < Level->mFeatures.count (); i++) {
        f = Level->mFeatures.get (i);
        if (canSee (f->mX, f->mY) && 
            (f->mSpotAttempted + 160000 < Clock) &&
            (distance (this, f->mX, f->mY) < 30)) 
        {
            f->mSpotAttempted = Clock;
            score = sportingD20 () + sk;
            I->debug ("spot check: %d", score);
            if ((shFeature::kDoorHiddenHoriz == f->mType ||
                 shFeature::kDoorHiddenVert == f->mType) &&
                score >= 24) 
            {
                I->p ("You spot a secret door!");
                f->mType = shFeature::kDoorClosed;
                f->mSportingChance = 0;
                Level->drawSqTerrain (f->mX, f->mY);
                I->pauseXY (f->mX, f->mY);
                interrupt ();
            } else if (f->isTrap () && f->mTrapUnknown && score >= 24) {
                if (f->isBerserkDoor ()) {
                    I->p ("You spot a malfunctioning door!");
                } else {
                    I->p ("You spot %s!", f->an ());
                }
                f->mTrapUnknown = 0;
                f->mSportingChance = 0;
                Level->drawSqTerrain (f->mX, f->mY);
                I->pauseXY (f->mX, f->mY);
                interrupt ();
            }
        }
    }
}


/* returns 1 if interrupted, 0 if not busy anyway */
int
shHero::interrupt ()
{
    Level->computeVisibility ();
    I->drawScreen ();
    if (mBusy) {
        mBusy = 0;
        return 1;
    } 
    return 0;
}


/* returns 1 if successful*/
int
shHero::wield (shObject *obj, int quiet /* = 0 */ )
{
    shObject *oldweapon = mWeapon;
    if (mWeapon) {
        if (mWeapon == obj) {
            if (0 == quiet) {
                I->p ("You're wielding that already!");
            }
            return 0;
        }
        if (mWeapon->isWeldedWeapon ()) {
            I->p ("%s is welded to your hand!", YOUR(mWeapon));
            mWeapon->setBugginessKnown ();
            return 0;
        }
        if (0 == unwield (mWeapon, quiet)) {
            return 0;
        }
    }
    if (TheNothingObject == obj) {
        if (NULL == oldweapon) { 
            if (0 == quiet) {
                I->p ("You're already unarmed.");
            }
            return 0;
        } else {
            if (0 == quiet) {
                I->p ("You are unarmed.");
            }
            resetStoryFlag ("strange weapon message");
            mWeapon = NULL;
            return 1;
        }
    } else if (obj->isWorn ()) {
        I->p ("You can't wield that because you're wearing it!");
        return 0;
    } else {
        mWeapon = obj;
        if (0 == quiet) {
            I->p ("You now wield %s.", AN(obj));
        }
        resetStoryFlag ("strange weapon message");
        obj->setWielded ();
        computeIntrinsics ();
        return 1;
    }
}


int
shHero::unwield (shObject *obj, int quiet /* = 0 */ )
{
    assert (mWeapon == obj);
    mWeapon = NULL;
    obj->resetWielded ();
    resetStoryFlag ("strange weapon message");
    computeIntrinsics ();
    return 1;
}


void
shHero::drop (shObject *obj)
{
    shFeature *f = Level->getFeature (mX, mY);
    int iscan = obj->isA (kCanister);

    const char *an_obj = AN (obj);
    
    if (f && shFeature::kVat == f->mType &&
        I->yn ("%s %s into the vat?", iscan ? "Pour" : "Drop", an_obj))
    {
        if (obj->isA ("canister of antimatter")) {
            I->p ("The vat is annihilated!");
            Level->removeFeature (f);
            obj->setIlkKnown ();
        } else if (obj->isA ("canister of liquid nitrogen")) {
            I->p ("The sludge freezes!");
            obj->setIlkKnown ();
        } else if (obj->isA ("canister of napalm")) {
            I->p ("The sludge boils!");
            obj->setIlkKnown ();
        } else if (obj->isA ("canister of beer")) {
            I->p ("The sludge fizzes.");
            obj->maybeName ();
        } else if (obj->isA ("canister of super glue")) {
            I->p ("The sludge thickens.");
            obj->maybeName ();
        } else if (obj->isA ("canister of nano cola")) {
            I->p ("The sludge fizzes.");
            obj->maybeName ();
        } else if (obj->isA ("canister of universal solvent")) {
            I->p ("The sludge thins.");
            obj->maybeName ();
        } else if (obj->isA ("canister of mutagen")) {
            I->p ("The sludge changes color.");
            f->mVat.mHealthy -= 2 * obj->mCount;
            f->mVat.mRadioactive++;
            obj->maybeName ();
        } else if (obj->isA ("canister of poison")) {
            I->p ("The sludge seems less nutritous now.");
            f->mVat.mHealthy -= 2 * obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("canister of spice")) {
            /* TODO: teleport the vat? */
            I->p ("The sludge smells like coffee now.");
            obj->maybeName ();
        } else if (obj->isA ("canister of healing")) {
            I->p ("The sludge seems more nutritious now.");
            f->mVat.mHealthy += obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("canister of full healing")) {
            I->p ("The sludge seems more nutritious now.");
            f->mVat.mHealthy += 2 * obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("canister of restoration")) {
            I->p ("You've improved the sludge recipe.");
            f->mVat.mHealthy += 2 * obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("canister of gain ability")) {
            I->p ("You've improved the sludge recipe.");
            f->mVat.mHealthy += 2 * obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("brain cylinder")) {
            I->p ("You drain %s.", 
                  obj->mCount > 1 ? "some brains" : "a brain");
            f->mVat.mHealthy += 2 * obj->mCount;
            obj->maybeName ();
        } else if (obj->isA ("canister of Rad-Away")) {
            I->p ("The sludge seems purified.");
            f->mVat.mHealthy += obj->mCount;
            f->mVat.mRadioactive = 0;
            obj->maybeName ();
        } else if (obj->isA ("canister of speed")) {
            I->p ("The sludge is churning more rapidly now.");
            obj->maybeName ();
        } else if (obj->isA ("canister of water")) {
            I->p ("The sludge thins.");
            obj->maybeName ();
        }

        else if (obj->mCount > 1) {
            I->p ("They sink under the sludge.");
        } else {
            I->p ("It sinks under the sludge.");
        }
        delete obj;
    } else {
        I->p ("You drop %s.", an_obj);
        Level->putObject (obj, mX, mY);
        if (Level->isInShop (mX, mY)) {
            maybeSellItem (obj);
        }
    }
}


void
shHero::listInventory () 
{
    if (0 == mInventory->count ()) {
        I->p ("You aren't carrying anything!");
    }
    else {
        int i;
        shObject *obj;
        shMenu menu ("Inventory", 
                     shMenu::kNoPick | shMenu::kCategorizeObjects);
        for (i = 0; i < mInventory->count (); i++) {
            obj = mInventory->get (i);
            menu.addItem (obj->mLetter, obj->inv (), obj, obj->mCount);
        }
        menu.finish ();
    }
}


/* returns non-zero if the hero dies */
int
shHero::instantUpkeep ()
{
    shObject *obj;
    int i;

    for (i = mInventory->count () - 1; i >= 0; --i) {
        obj = mInventory->get (i);
        if (obj->isActive () || obj->isA (PowerPlant)) {
            if (MAXTIME == obj->mLastEnergyBill) {
                /* unnecessary, should be handled by setActive () */
                obj->mLastEnergyBill = Clock; 
            } else {
                while (Clock - obj->mLastEnergyBill > obj->mIlk->mEnergyUse) {
                    if (obj->isA (PowerPlant)) {
                        gainEnergy (1);
                    } else if (0 == loseEnergy (1)) {
                        obj->resetActive ();
                        I->p ("%s has shut itself off.", YOUR(obj));
                    }
                    obj->mLastEnergyBill += obj->mIlk->mEnergyUse;
                }
            }
        }
    }
    if (hasAutoRegeneration ()) {
        while (Clock - mLastRegen > 1500) {
            if (mHP < mMaxHP) ++mHP;
            mLastRegen += 1500;
        }
    }
    if (checkTimeOuts ()) {
        return -1;
    }
    computeIntrinsics ();
    return 0;
}


void
shHero::feel (int x, int y, int force)
{
    if (!force && 
        (!isBlind () || !isAdjacent (x, y)))
    {
        return;
    }
    Level->feelSq (x, y);
}


int
shHero::tryToEscapeTrap ()
{              
    if (--mTrapped <= 0) {
        if (Level->isObstacle (mX, mY)) {
            ++mTrapped;
        } else {
            mZ = 0;
            shFeature *f = Level->getFeature (mX, mY);
            if (!f) {
                I->p ("You can move again.");
            } else switch (f->mType) {
                case shFeature::kPit:
                    I->p ("You climb out of the pit."); break;
                case shFeature::kAcidPit:
                    I->p ("You climb out of the acid filled pit."); break;
                case shFeature::kSewagePit:
                    I->p ("You swim back to shallow sewage."); break;
                case shFeature::kWeb:
                    I->p ("You free yourself from the web.");
                    break;
                default:
                    I->p ("You can move again."); break;
                }
        }
    }
    return FULLTURN;
}



/* try to move the hero one square in the given direction
   RETURNS: AP expended 
 */

int
shHero::doMove (shDirection dir)
{
    int x = mX;
    int y = mY;
    int tx, ty;
    static int leftsq, rightsq; /* this static crap is not tasty */
    shFeature *f;
    int speed;

    speed = isDiagonal (dir) ? DIAGTURN: FULLTURN;

    if (!Level->moveForward (dir, &x, &y)) {
        //I->p ("You can't move there!");
        return 0;
    }

    mDir = dir;

    if (1 == mBusy) {
        tx = x; ty = y;
        if (!Level->moveForward (leftTurn (dir), &tx, &ty)) {
            leftsq = 0;
        } else {
            leftsq = Level->appearsToBeFloor (tx, ty);
        }
        tx = x; ty = y;
        if (!Level->moveForward (rightTurn (dir), &tx, &ty)) {
            rightsq = 0;
        } else {
            rightsq = Level->appearsToBeFloor (tx, ty);
        }
        ++mBusy;
    } else if (mBusy) {
        if (isStunned ()) {
            interrupt ();
            return 0;
        }
        feel (x, y);
        if (Level->isObstacle (x, y)) {
            interrupt ();
            return 0;
        }
        if ((f = Level->getFeature (x, y))) {
            if (!f->mTrapUnknown || f->isObstacle ()) {
                interrupt ();
                return 0;
            }
        }
        if (isBlind ()) {
            tx = x;     ty = y;
            if (Level->moveForward (leftTurn (dir), &tx, &ty)) {
                feel (tx, ty);
            }
            if (Level->moveForward (rightTurn (dir), &tx, &ty)) {
                feel (tx, ty);
            }
            goto trymove;
        }

        tx = x; ty = y;
        if (!Level->moveForward (leftTurn (dir), &tx, &ty) ||
            leftsq != Level->appearsToBeFloor (tx, ty) ||
            (Level->countObjects (tx, ty) && !Level->isWatery (tx, ty)) ||
            Level->getKnownFeature (tx, ty))
        {
            interrupt (); 
            //return 0;
        }
        tx = x; ty = y;
        if (!Level->moveForward (rightTurn (dir), &tx, &ty) ||
            rightsq != Level->appearsToBeFloor (tx, ty) ||
            (Level->countObjects (tx, ty) && !Level->isWatery (tx, ty)) ||
            Level->getKnownFeature (tx, ty))
        {
            interrupt (); 
            //return 0;
        }
    }
trymove:

    if (Level->isObstacle (x, y)) {
        feel (x, y);
        if (!interrupt ()) {
            if (isStunned () || isConfused () || isBlind ()) {
                I->p ("You bump into %s!", Level->the (x, y));
                return speed;
            }
        }
    } else if (Level->isOccupied (x, y)) {
        feel (x, y);
        if (!interrupt ()) {
            if (isStunned () || isConfused () || isBlind ()) {
                I->p ("You bump into %s!", 
                      THE(Level->getCreature (x, y)));
                return speed;
            }
        }
    } else {
        Level->moveCreature (this, x, y);
        return speed;
    }
 
    return 0;
}


/* called every 10 seconds */
void
shHero::upkeep ()
{
    if (!isSickened ()) {
        /* recover hit points */
        if (mHP < mMaxHP) {
            mHP += 1 + mCLevel / 3;
            if (mHP > mMaxHP) {
                mHP = mMaxHP;
            }
        }
        
        /* recover a cha drain point */
        if (mChaDrain > 0) {
            --mChaDrain;
            ++mAbil.mCha;
        } else if (mChaDrain < 0) {
            ++mChaDrain;
            --mAbil.mCha;
        }
    }

    /* superglued tongue */
    {
        int glue = getStoryFlag ("superglued tongue");
        if (glue > 0) {
            glue++;
            if (glue > 100) {
                I->p ("The canister of super glue has finally fallen off!");
                resetStoryFlag ("superglued tongue");
                mAbil.setByIndex (kCha, 4 + mAbil.getByIndex (kCha));
            } else {
                setStoryFlag ("superglued tongue", glue);
            }
        }
    }

    /* reduce police awareness of your software piracy crimes */
    {
        int danger = getStoryFlag ("software piracy");
        if (danger > 0) {
            setStoryFlag ("software piracy", --danger);
        }
    }

    /* check for alien impregnation */
    {
        int preggers = getStoryFlag ("impregnation");

        if (preggers > 0) {
            preggers++;
            if (preggers > 40) {
                int x = Hero.mX;
                int y = Hero.mY;
                int queen = !RNG (0, 17);
                shMonster *baby = 
                    new shMonster (findAMonsterIlk (queen ? "alien princess"
                                                          : "chestburster"));
                Level->findNearbyUnoccupiedSquare (&x, &y);
                if (baby) {
                    if (Level->putCreature (baby, x, y)) {
                        /* FIXME: something went wrong */
                    } else {
                        I->drawScreen ();
                    }
                }
                I->p ("The alien creature explodes from your chest!");
                die (kMisc, "Died during childbirth");
            } else if (38 == preggers) {
                I->p ("You are violently ill!");
                makeStunned (35000);
                interrupt ();
            } else if (35 == preggers) {
                I->p ("You vomit!");
                makeSickened (100000);
                interrupt ();
            } else if (20 == preggers) {
                I->p ("You feel something moving inside you!");
                interrupt ();
            } else if (10 == preggers) {
                I->p ("You feel a little queasy.");
                interrupt ();
            }
            setStoryFlag ("impregnation", preggers);
        }
    }

    /* check radiation */
    {
        int r = checkRadiation ();
        int d = 0;
        while (r--) {
            d += RNG (1, 3);
        }
        d -= getResistance (kRadiological);
        if (d < 0) d = 0;

        mRad += d;
    }

    if (mRad > 300) {
        mRad -= RNG (1, 10);
    }

    if (hasRadiationProcessing ()) {
        mRad -= 15;
    }

    if (mRad < 0) {
        mRad = 0;
    }

    if (2 == (Clock / 10000) % 6) {
        if (mRad > 1000) {
            if (0 == RNG (100)) {
                /* mutation */
            }
        }
        if (mRad > 150) {
            int level = getStoryFlag ("radsymptom");
            if (RNG (20) + ABILITY_MODIFIER (getCon ()) <
                mRad / 12) 
            {
                if (sufferAbilityDamage (kCon, RNG (1,2))) {
                    I->p ("You are overcome by your illness.");
                    die (kSlain, "radiation sickness");
                } else {
                    if (level < 7 - getCon ()) {
                        level = 7 - getCon ();
                    }
                    switch (level) {
                    case 0:
                        I->p ("You feel tired."); break;
                    case 1:
                        I->p (RNG (2) ? "You feel weak." : 
                              "You feel feverish."); break;
                    case 2:
                        I->p ("You feel nauseated."); break;
                    case 3:
                        I->p ("You vomit!"); break;
                    case 4:
                        I->p ("Your gums are bleeding!"); break;
                    case 5:
                        I->p ("Your hair is falling out!"); break;
                    case 6:
                        I->p ("Your skin is melting!"); break;
                    default:
                        I->p ("Your body is wracked with convulsions.");
                        makeStunned (9000);
                    }
                    level++;
                    setStoryFlag ("radsymptom", level);
                }
            } else if (0 == level) {
                I->p ("You have a slight headache.");
                setStoryFlag ("radsymptom", 1);
            } else if (RNG (2)) {
                I->p ("You feel weary.");
            } else {
                I->p ("You feel quite sick.");
            }
        } 
        else {
            if (getStoryFlag ("radsymptom")) {
                I->p ("You're beginning to feel better.");
                setStoryFlag ("radsymptom", 0);
            }
        }
        mRad -= 5; /* slowly recover */
        if (mRad < 0) {
            mRad = 0;
        }
    }

    if (hasNarcolepsy () && 
        !isAsleep () && 
        !RNG (10)) 
    {
        I->p ("You suddenly fall asleep!");
        makeAsleep (1000 * RNG (20, 100));
    }
}


void
shHero::takeTurn ()
{
    shInterface::Command cmd;
    int dx;
    int dy;
    int glidemode = 0;
    int elapsed = 0;

    if (instantUpkeep ()) { /* hero died */
        return;
    }
    if (1 || !mBusy) {
        Level->computeVisibility ();
        I->drawScreen ();
    }

    if (isParalyzed () || isAsleep ()) {
        /* lose a turn */
        mAP -= 1000;
        return;
    }

 getcmd:

    if (hasRadiationDetection ()) {
        int rl = checkRadiation ();
        int ol = getStoryFlag ("geiger counter message");

        if (rl != ol) {
            int i;
            shObject *obj = NULL;
            const char *what = NULL;

            for (i = 0; i < mInventory->count (); i++) {
                obj = mInventory->get (i);
                if (obj->isA (findAnIlk (&ToolIlks, "geiger counter", 1))) {
                    what = YOUR (obj);
                    break;
                }
            }
            if (0 == ol) {
                interrupt ();
                I->p ("%s is making a clicking noise.", what);
                if (obj) obj->setToggled ();
            } else if (0 == rl) {
                interrupt ();
                I->p ("%s has stopped clicking.", what);
                if (obj) obj->resetToggled ();
            } else if (ol < rl) {
                interrupt ();
                I->p ("%s is clicking more rapidly.", what);
            } else {
                interrupt ();
                I->p ("%s is clicking more slowly.", what);
            }
        }
        setStoryFlag ("geiger counter message", rl);
    }

    


    if (mBusy && !isTrapped () && !isStunned () && !isConfused ()) {
        elapsed = doMove (mDir);
        if (elapsed) {
            mAP -= elapsed;
            return;
        }
    }

    mBusy = 0;
    cmd = I->getCommand ();

    dx = 0;
    dy = 0;
    //I->debug ("CLOCK %d -----------------------------", Clock);
    if ((isStunned () || 
         (isConfused () && !RNG (3))) && 
        cmd >= shInterface::kMoveN && cmd <= shInterface::kMoveNW)
    {
        cmd = (shInterface::Command) (shInterface::kMoveN + RNG (8));
    }
    
    switch (cmd) {
    case shInterface::kNoCommand:
        I->p ("Unknown command.  Type '?' for help.");
        goto getcmd;
    case shInterface::kHelp:
        I->showHelp ();
        goto getcmd;
    case shInterface::kHistory:
        I->showHistory ();
        goto getcmd;
    case shInterface::kRest:
        /* rest for 1000 ms */
        elapsed = FULLTURN;
        break;
    case shInterface::kGlide:
    {
        if (isConfused () || isStunned () || isTrapped ()) {
            goto getcmd;
        }
        int dz = 0;
        shDirection d;
        d = I->getDirection (&dx, &dy, &dz, 1);
        if (kNoDirection == d || kOrigin == d || dz) {
            goto getcmd;
        }
        glidemode = 1;
        mBusy = 1;
        I->pageLog ();
            
        goto domove;
    }
    case shInterface::kGlideN:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveN:
        --dy;  goto domove;
    case shInterface::kGlideNE:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveNE:
        ++dx; --dy; goto domove;
    case shInterface::kGlideE:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveE:
        ++dx; goto domove;
    case shInterface::kGlideSE:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveSE:
        ++dx; ++dy; goto domove;
    case shInterface::kGlideS:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveS:
        ++dy; goto domove;
    case shInterface::kGlideSW:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveSW:
        ++dy; --dx; goto domove;
    case shInterface::kGlideW:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveW:
        --dx; goto domove;
    case shInterface::kGlideNW:
        glidemode = 1; mBusy = 1; /* fall through */
    case shInterface::kMoveNW:
        --dx; --dy;

    domove:
        {
            int x = mX + dx;
            int y = mY + dy;
            shCreature *c;

            if (!Level->isInBounds (x, y))
                goto getcmd;

            c = Level->getCreature (x, y);

            if (Level->rememberedCreature (x, y) || 
                (c && hasMotionDetection () && c->isMoving ()) ||
                (c && hasTelepathy() && c->hasMind ()) ||
                (c && !isBlind ())) 
            { /* this must be an attack */
                if (glidemode) {
                    glidemode = 0;
                    goto getcmd;
                }
                
                if (c && c->mZ < 0 && mLevel->isObstacle (x, y)) {
                    I->p ("You can't attack there!");
                    goto getcmd;
                } 
                if (mZ < 0 && mLevel->isObstacle (mX, mY)) {
                    if (isTrapped ())
                        goto untrap;
                    else
                        goto getcmd;
                }
                if (c && c->isPet () && !isStunned () && !isConfused () 
                    && !isStunned ()) 
                {
                    if (isTrapped ()) {
                        goto untrap;
                    }
                    elapsed = displace (c);
                } else {
                    if (c && isFrightened () && !isConfused () &&
                             !isStunned () && !isBlind ()) 
                    {
                        I->p ("You are too afraid to attack %s!", THE (c));
                        goto getcmd;
                    }
                    if (c && !c->isHostile () && !c->isA ("monolith") &&
                        !isBlind () && !isConfused () && !isStunned ()) 
                    {
                        if (!I->yn ("Really attack %s?", THE (c))) {
                            I->nevermind ();
                            goto getcmd;
                        }
                    }
                    elapsed = meleeAttack (mWeapon, I->moveToDirection (cmd));
                }
                if (!elapsed) {
                    goto getcmd;
                }
            } else if (isTrapped ()) {
            untrap:
                elapsed = tryToEscapeTrap ();
            } else {
                elapsed = doMove (vectorDirection (dx, dy));
            }
        }
        break;

    {
        shDirection dir;
        shAttack *atk;

    case shInterface::kFireWeapon:
        dir = kNoDirection;
        goto checkweapon;

    case shInterface::kFireN:   dir = kNorth; goto checkweapon;
    case shInterface::kFireNE:  dir = kNorthEast; goto checkweapon;
    case shInterface::kFireE:   dir = kEast; goto checkweapon;
    case shInterface::kFireSE:  dir = kSouthEast; goto checkweapon;
    case shInterface::kFireS:   dir = kSouth; goto checkweapon;
    case shInterface::kFireSW:  dir = kSouthWest; goto checkweapon;
    case shInterface::kFireW:   dir = kWest; goto checkweapon;
    case shInterface::kFireNW:  dir = kNorthWest; goto checkweapon;
//    case shInterface::kFireDown:      dir = kDown; goto checkweapon;
//    case shInterface::kFireUp:        dir = kUp; goto checkweapon;

    checkweapon:


        if (NULL == mWeapon || 
            ! (mWeapon->isA (kWeapon) || mWeapon->isA (kRayGun))) 
        {
            I->p ("You're not wielding a ranged weapon.");
            goto getcmd;
        } 
        atk = & ((shWeaponIlk *) mWeapon->mIlk) -> mAttack;
        if (atk->isMeleeAttack ()) {
            I->p ("You're not wielding a ranged weapon.");
            goto getcmd;
        }       

        if (isFrightened ()) {
            I->p ("You are too afraid to %s!", 
                  mWeapon->isThrownWeapon () ? "throw your weapon"
                                             : "fire your weapon");
            goto getcmd;
        }

        if (kNoDirection == dir) {
            dir = I->getDirection ();
            if (kNoDirection == dir) {
                goto getcmd;
            }
        }
        resetStoryFlag ("strange weapon message");

        if (mWeapon->isThrownWeapon ()) {
            shObject *obj = removeOneObjectFromInventory (mWeapon);
            elapsed = throwObject (obj, dir);
        } else {
            elapsed = shootWeapon (mWeapon, dir);
        }
        break;
    }

    case shInterface::kThrow:
    {
        shObject *obj;
        shObjectVector v;
        shDirection dir;

        if (isFrightened ()) {
            I->p ("You are too afraid to throw anything!");
            goto getcmd;
        }

        selectObjectsByFunction (&v, mInventory, &shObject::isThrownWeapon);
        obj = quickPickItem (&v, "throw", shMenu::kAnythingAllowed | 
                             shMenu::kCategorizeObjects);
        if (NULL == obj) {
            goto getcmd;
        }
        if (obj->isWeldedWeapon ()) {
            obj->setBugginessKnown ();
            I->p ("Your weapon is welded to your hands!");
            elapsed = HALFTURN;
        } else if (obj->isWorn ()) {
            I->p ("You can't throw that because you're wearing it!");
            goto getcmd;
        } else {
            dir = I->getDirection ();
            if (kNoDirection == dir) {
                goto getcmd;
            }
        
            obj = removeOneObjectFromInventory (obj);
            elapsed = throwObject (obj, dir);
        }
        break;
    }

    case shInterface::kKick:
    {
        shDirection dir;
        
        dir = I->getDirection ();
        if (kNoDirection == dir) {
            goto getcmd;
        } else if (kOrigin == dir) {
            I->p ("Kicking yourself isn't going to make things any better.");
            goto getcmd;
        }

        elapsed = kick (dir);
        break;
    }

    case shInterface::kZapRayGun:
    {
        shObject *obj;
        shDirection dir;
        shObjectVector v;

        if (isFrightened ()) {
            I->p ("You are too afraid to use a raygun!"); 
            goto getcmd;
        }

        selectObjects (&v, mInventory, kRayGun);
        obj = quickPickItem (&v, "zap", 0);
        if (NULL == obj) {
            goto getcmd;
        }
        dir = I->getDirection ();
        if (kNoDirection == dir) {
            goto getcmd;
        }
        elapsed = shootWeapon (obj, dir);
        break;
    }


    case shInterface::kMoveUp:
    {
        int x, y;
        shMapLevel *oldlevel = Level;
        shFeature *stairs = Level->getFeature (mX, mY);

        if (mTrapped) {
            elapsed = tryToEscapeTrap ();
            break;
        }
        
        if (NULL == stairs || shFeature::kStairsUp != stairs->mType) {
            I->p ("You can't go up here.");
            goto getcmd;
        }
        Level->removeCreature (this);
        Level = Maze.get (stairs->mDest.mLevel);

        if (-1 == stairs->mDest.mX) {
            Level->findUnoccupiedSquare (&x, &y);
        } else {
            x = stairs->mDest.mX;
            y = stairs->mDest.mY;
            if (Level->isOccupied (x, y)) {
                Level->findNearbyUnoccupiedSquare (&x, &y);
            }
        }
        Level->putCreature (this, x, y);
        checkForFollowers (oldlevel, stairs->mX, stairs->mY); 
        elapsed = FULLTURN;
        break;
    }
    case shInterface::kMoveDown:
    {
        int x, y;
        shMapLevel *oldlevel = Level;
        shFeature *stairs = Level->getFeature (mX, mY);

        if (NULL == stairs) {
            I->p ("You can't go down here.");
            goto getcmd;
        }
        if (-1 == Hero.mZ) {
            I->p ("You're already at the bottom.");
            goto getcmd;
        }

        if (isFlying ()) {
            I->p ("You are flying high above %s.", THE (stairs));
            goto getcmd;
        }
        switch (stairs->mType) {
        case shFeature::kHole:
            I->p ("You jump into a hole!");
            break;
        case shFeature::kTrapDoor:
            I->p ("You jump into a trap door!");
            break;
        case shFeature::kPit:
            I->p ("You climb down into the pit.");
            mZ = -1;
            mTrapped = NDX (1, 6);
            elapsed = FULLTURN;
            break;
        case shFeature::kAcidPit:
            I->p ("You climb down into the pit.");
            mZ = -1;
            mTrapped = NDX (1, 6);
            if (sufferDamage (&AcidPitTrapDamage)) {
                die (kMisc, "Dissolved in acid");
                return;
            }
            setTimeOut (TRAPPED, 1000, 0);
            elapsed = FULLTURN;
            break;
        case shFeature::kSewagePit:
            I->p ("You dive into the sewage.");
            Hero.mZ = -1;
            Hero.mTrapped = NDX (1, 6);
            if (!hasAirSupply () && !isBreathless ()) {
                I->p ("You're holding your breath!");
                mDrowning = getCon ();
            }
            setTimeOut (TRAPPED, 1000, 0);
            elapsed = FULLTURN;
            break;
        case shFeature::kStairsDown:
            break;
        default:
            I->p ("You can't go down here.");
            goto getcmd;
        }

        if (elapsed)
            break;

        /* o/w we are descending a level */
        Level->removeCreature (this);
        Level = Maze.get (stairs->mDest.mLevel);
        if (NULL == Level) {
            Level = oldlevel->getLevelBelow ();
        }
        if (-1 == stairs->mDest.mX) {
            Level->findUnoccupiedSquare (&x, &y);
        } else {
            x = stairs->mDest.mX;
            y = stairs->mDest.mY;
            if (Level->isOccupied (x, y)) {
                Level->findNearbyUnoccupiedSquare (&x, &y);
            }
        }
        Level->putCreature (this, x, y);
        checkForFollowers (oldlevel, stairs->mX, stairs->mY); 
        elapsed = FULLTURN;
        break;
    }
    case shInterface::kOpen:
    case shInterface::kClose:
    {
        int dz;
        int x, y, z;
        shFeature *f;

        if (kNoDirection == I->getDirection (&dx, &dy, &dz)) {
            goto getcmd;
        }
        x = dx + mX;
        y = dy + mY;
        z = dz;
        if (!Level->isInBounds (x, y)) {
            I->p ("There is nothing to %s there.", 
                  shInterface::kOpen == cmd ? "open" : "close");
            goto getcmd;
        }
        feel (x, y);
        f = Level->getFeature (x, y);
        if (shInterface::kOpen == cmd) {
            if (f && shFeature::kDoorClosed == f->mType) {
                if (0 == openDoor (x, y)) {
                    //I->p ("You can't open it.");
                }
                feel (x, y, 1);
                elapsed = FULLTURN;
            } else if (f && shFeature::kDoorOpen == f->mType) {
                I->p ("It's already open.");
                goto getcmd;
            } else {
                I->p ("There is nothing there to open.");
                goto getcmd;
            }
        } else {
            if (f && shFeature::kDoorOpen == f->mType) {
                if (0 != Level->countObjects (x, y)) {
                    I->p ("There's something obstructing it.");
                } else if (Level->isOccupied (x, y)) {
                    I->p ("There's a monster in the way!");
                } else if (0 == closeDoor (x, y)) {
                    I->p ("You fail to close the door.");
                }
                feel (x, y, 1);
                elapsed = HALFTURN; /* it takes less time to slam a door shut */
            } else if (f && shFeature::kDoorClosed == f->mType) {
                I->p ("It's already closed.");
                goto getcmd;
            } else {
                I->p ("There is nothing there to close.");
                goto getcmd;
            }
        }
        break;
    }

    case shInterface::kPickup:
        {
            int objcnt;
            shObjectVector *v = Level->getObjects (mX, mY);
            shObject *obj;

            objcnt = v ? v->count () : 0;
            if (0 == objcnt) {
                I->p ("There is nothing on the ground to pick up.");
                goto getcmd;
            }
            if (isFlying ()) {
                I->p ("You are flying high above the floor.");
                goto getcmd;
            }
            if (1 == objcnt) {
                if (!isBlind ()) 
                    v->get (0) -> setAppearanceKnown ();
                /* HACK: if the object is merged into an inventory object, 
                   the the floor will momentarily contain a deleted object
                   which would be dereferenced during the screen redraw that
                   occurs while printing the "you picked up an object" 
                   message.  So, we preemptively null out the floor objs: */
                Level->setObjects (mX, mY, NULL);
                if (1 == addObjectToInventory (v->get (0))) {
                    if (Level->isInShop (mX, mY)) {
                        pickedUpItem (v->get (0));
                    }
                    delete v;
                    v = NULL;
                }
                /* END HACK: Restore the floor. */
                Level->setObjects (mX, mY, v);
            }
            else {
                int i, cnt;
                shMenu menu ("Pick up what?", 
                             shMenu::kMultiPick | shMenu::kCountAllowed |
                             shMenu::kCategorizeObjects);

                v->sort (&compareObjects);
                for (i = 0; i < v->count (); i++) {
                    obj = v->get (i);
                    int let = i % 52;
                    menu.addItem (let < 26 ? let + 'a' : let - 26 + 'A', 
                                  AN (obj), obj, obj->mCount);
                }
                objcnt = 0;
                while (menu.getResult ((const void **) &obj, &cnt)) {
                    assert (cnt);
                    ++objcnt;
                    if (!isBlind()) 
                        obj->setAppearanceKnown ();
                    if (cnt != obj->mCount) {
                        obj = obj->split (cnt);
                    } else {
                        /* HACK: as above, need to pre-emptively remove the
                           object from the floor vector even though we might 
                           not actually pick it up: */
                        v->remove (obj);
                    }
                    if (0 == addObjectToInventory (obj)) {
                        /* END HACK: put it back. */
                        Level->putObject (obj, mX, mY);
                    } else {
                        if (Level->isInShop (mX, mY)) {
                            pickedUpItem (obj);
                        }
                    }
                }               
            }
            if (0 == objcnt) { /* didn't pick anything up */
                goto getcmd;
            }
            feel (mX, mY);
            elapsed = FULLTURN;
            break;
        }
    case shInterface::kListInventory:
    {
        reorganizeInventory ();
        listInventory ();
        goto getcmd;
    }
    case shInterface::kAdjust:
    {
        shObject *obj;
        shObject *obj2;
        char c;

        obj = quickPickItem (mInventory, "adjust", shMenu::kCategorizeObjects);
        if (NULL == obj) {
            goto getcmd;
        }
        I->p ("Adjust to what letter?");
        c = I->getChar (I->logWin());
        if (isalpha (c) && c != obj->mLetter) {
            int i;
            for (i = 0; i < mInventory->count (); i++) {
                obj2 = mInventory->get (i);
                if (obj2->mLetter == c) {
                    obj2->mLetter = obj->mLetter;
                    obj->mLetter = c;
                    I->p ("%c - %s", obj->mLetter, obj->inv ());
                    I->p ("%c - %s", obj2->mLetter, obj2->inv ());
                    goto getcmd;
                }
            }
            obj->mLetter = c;
            I->p ("%c - %s", obj->mLetter, obj->inv ());
            goto getcmd;
        }
        I->nevermind ();
        goto getcmd;
    }
    case shInterface::kDrop:
    {
        shObject *obj;
        int cnt = 1;
        
        if (0 == mInventory->count ()) {
            I->p ("You aren't carrying anything!");
            goto getcmd;
        }
        reorganizeInventory ();

        obj = quickPickItem (mInventory, "drop", 
                             shMenu::kCountAllowed | 
                             shMenu::kCategorizeObjects, &cnt);
        if (NULL == obj) {
            goto getcmd;
        }
        if (obj == mWeapon) {
            if (obj->isWeldedWeapon ()) {
                I->p ("Your weapon is welded to your hands");
                obj->setBugginessKnown ();
                goto getcmd;
            }
            if (0 == unwield (obj)) {
                goto getcmd;
            }
        }
        if (obj->isWorn ()) {
            I->p ("You can't drop that because you're wearing it!");
            goto getcmd;
        }
        
        obj = removeSomeObjectsFromInventory (obj, cnt);

        drop (obj);
        elapsed = HALFTURN;
        break;
    }

    case shInterface::kDropMany:
    {
        int i;
        shObject *obj;
        int cnt;
        int ndropped = 0;

        reorganizeInventory ();

        if (0 == mInventory->count ()) {
            I->p ("You aren't carrying anything!");
            goto donedropping;
        }
        else {
            shMenu menu ("Drop what?", 
                         shMenu::kMultiPick | shMenu::kCountAllowed |
                         shMenu::kCategorizeObjects);
            for (i = 0; i < mInventory->count (); i++) {
                obj = mInventory->get (i);
                menu.addItem (obj->mLetter, obj->inv (), obj, obj->mCount);
            }
            
            while (menu.getResult ((const void **) &obj, &cnt)) {
                if (0 == cnt) {
                    abort ();
                }
                if (obj == mWeapon) {
                    if (obj->isWeldedWeapon ()) {
                        I->p ("Your weapon is welded to your hands");
                        obj->setBugginessKnown ();
                        continue;
                    }
                    if (0 == unwield (obj)) {
                        continue;
                    }
                }
                if (obj->isWorn ()) {
                    I->p ("You can't drop %s because you're wearing it.", 
                          YOUR (obj));
                    continue;
                }
                obj = removeSomeObjectsFromInventory (obj, cnt);
                drop (obj);
                ++ndropped;
            }
        }
    donedropping:
        if (0 == ndropped) { /* didn't drop anything, no time penalty */
            goto getcmd;
        } else if (1 == ndropped) {
            elapsed = HALFTURN;
        } else {
            elapsed = FULLTURN;
        }
        break;
    }

    case shInterface::kLookHere:
        lookAtFloor (1);
        if (0 == Level->countObjects (mX, mY)) {
            I->p ("There are no objects here.");
        }
        goto getcmd;

    case shInterface::kLookThere:
    {
        int x = -1, y = -1;
        while (I->getSquare ("Look at what? (select a location, '?' for help)",
                             &x, &y, -1, 1))
        {
            shCreature *c = Level->getCreature (x, y);
            shFeature *f = Level->getFeature (x, y);
            int seen = 0;
            I->pageLog ();

            //FIXME: should remember things not in LOS

            if (Level->rememberedCreature (x, y)) {
                I->p ("You remember an unseen monster there.");
                seen++;
            } else if (c) {
                const char *pcf = "";
                const char *desc = NULL;
                if (c->isHostile ()) {
                    desc = AN (c);
                } else {
                    pcf = "a peaceful ";
                    desc = c->getDescription ();
                }
                if (canSee (c) && c->mHidden <= 0) {
                    const char *wielding = "";
                    const char *weapon = "";
                    if (c->mWeapon) {
                        wielding = ", wielding ";
                        weapon = AN (c->mWeapon);
                    }
                    I->p ("You see %s%s%s%s.", pcf, desc, wielding, weapon);
                    seen++;
                } else if (hasTelepathy () && c->hasMind () && 
                           distance (this, x, y) < 5 * (mCLevel + 20))
                {
                    I->p ("You sense %s.", pcf, desc);
                    seen++;
                } else if (hasMotionDetection () && 
                           c->isMoving () &&
                           distance (this, x, y) < 50) 
                {
                    I->p ("You see a blip on your motion tracker.");
                    seen++;
                } else if (canSee(c) && c->mHidden > 0) {
                    switch (c->mMimic) {
                    case shCreature::kObject:
                        I->p ("You see %s.", c->mMimickedObject->mVagueName);
                        seen++;
                        break;
                    default:
                        /* nothing seen */
                        break;
                    }
                }
            } 
            if (!canSee (x, y)) {
                if (!seen)
                    I->p ("You can't see that location from here.");
                continue;
            } 

            if (Level->countObjects (x, y) && !Level->isWatery (x, y)) {
                shObjectVector *objs = Level->getObjects (x, y);
                int i;
                int besttype = kMaxObjectType;
                shObject *bestobj = NULL;
                for (i = 0; i < objs->count (); i++) {
                    if (objs->get (i) -> mIlk -> mType < besttype) {
                        besttype = objs->get (i) -> mIlk -> mType;
                        bestobj = objs->get (i);
                    }
                }
                I->p ("You see %s.", AN (bestobj));
                seen++;
            } 
            if (f && !(f->isTrap () && f->mTrapUnknown)) {
                switch (f->mType) {
                case shFeature::kDoorHiddenVert:
                case shFeature::kDoorHiddenHoriz:
                case shFeature::kMovingHWall:
                    I->p ("You see a wall."); break;
                case shFeature::kStairsUp:
                    I->p ("You see a staircase leading upstairs."); break;
                case shFeature::kStairsDown:
                    I->p ("You see a staircase leading downstairs."); break;
                case shFeature::kDoorClosed:
                case shFeature::kDoorOpen:
                    I->p ("You see a%s%s%s door.",
                          f->isBerserkDoor () && !f->mTrapUnknown ? 
                             " malfunctioning" : "",
                          f->isOpenDoor () ? " open" : " closed",
                          f->isAutomaticDoor () ? " automatic" : "");
                    break;
                case shFeature::kVat:
                    I->p ("You see a vat of sludge."); break;
                case shFeature::kPit:
                    I->p ("You see a pit."); break;
                case shFeature::kAcidPit:
                    I->p ("You see an acid-filled pit."); break;
                case shFeature::kTrapDoor:
                    I->p ("You see a trap door."); break;
                case shFeature::kHole:
                    I->p ("You see a hole."); break;
                case shFeature::kWeb:
                    I->p ("You see a web."); break;
                case shFeature::kRadTrap:
                    I->p ("You see a radiation trap."); break;
                case shFeature::kSewagePit:
                    I->p ("You see a dangerously deep pool of sewage."); break;
                case shFeature::kMachinery:
                    I->p ("You see pistons and machinery.");
                }
            } else {
                if (Level->isFloor (x, y)) {
                    if (kSewage == Level->getSquare(x,y)->mTerr) {
                        I->p ("You see a pool of sewage."); 
                    } else if (!seen) {
                        I->p ("You see the floor.");
                    }
                } else if (kVoid == Level->getSquare (x, y) ->mTerr) {
                    I->p ("You see a yawning void.");
                } else {
                    I->p ("You see a wall.");
                }
            }
        }
        I->p ("Done.");
        goto getcmd;
    }

    case shInterface::kPay:
    {
        payShopkeeper ();
        elapsed = 0;
        break;
    }

    case shInterface::kSearch:
    {
        int x, y, sk;

        sk = getSkillModifier (kSearch);
        for (x = mX - 1; x <= mX + 1; x++) {
            for (y = mY - 1; y <= mY + 1; y++) {
                if (Level->isInBounds (x, y)) {
                    int score = sportingD20 () + sk;
                    shFeature *f = Level->getFeature (x, y);
                    feel (x, y);
                    if (f) {
                        score += f->mSportingChance++;
                        if ((shFeature::kDoorHiddenHoriz == f->mType ||
                             shFeature::kDoorHiddenVert == f->mType) &&
                            score >= 20)
                        {
                            exerciseSkill (kSearch, 2);
                            f->mType = shFeature::kDoorClosed;
                            f->mSportingChance = 0;
                            I->p ("You find a secret door!");
                            Level->drawSqTerrain (f->mX, f->mY);
                            I->pauseXY (f->mX, f->mY);
                        } else if ((shFeature::kDoorClosed == f->mType ||
                                    shFeature::kDoorOpen == f->mType) &&
                                   shFeature::kBerserk & f->mDoor &&
                                   f->mTrapUnknown && 
                                   score >= 12)
                        {
                            exerciseSkill (kSearch, 1);
                            I->p ("You find a malfunction in the door.");
                            f->mTrapUnknown = 0;
                            f->mSportingChance = 0;
                            Level->drawSqTerrain (f->mX, f->mY);
                            I->pauseXY (f->mX, f->mY);
                        } else if (f->isTrap () &&
                                   f->mTrapUnknown &&
                                   score >= 20)
                        {
                            I->p ("You find %s!", AN (f));
                            f->mTrapUnknown = 0;
                            f->mSportingChance = 0;
                            Level->drawSqTerrain (f->mX, f->mY);
                            I->pauseXY (f->mX, f->mY);
                        }
                    }
                }
            }
        }

        elapsed = FULLTURN;
        break;
    }
    case shInterface::kQuaff:
    {
        shObject *obj;
        shObjectVector v;
        shFeature *f;

        if (getStoryFlag ("superglued tongue")) {
            I->p ("You can't drink anything with this stupid canister "
                  "glued to your mouth!");
            goto getcmd;
        }

        if (NULL != (f = Level->getFeature (mX, mY)) &&
            (shFeature::kVat == f->mType) && 
            !isFlying () &&
            I->yn ("Drink from the vat?")) 
        {
            quaffFromVat (f);
        } else {
            selectObjects (&v, mInventory, kCanister);
            obj = quickPickItem (&v, "quaff", 0);
            if (NULL == obj) {
                goto getcmd;
            }
            elapsed  = quaffCanister (obj);
        }
        elapsed = FULLTURN;
        break;
    }
    case shInterface::kUse:
    {
        shObject *obj;
        shObjectVector v;

        selectObjectsByFunction (&v, mInventory, &shObject::isUseable);
        obj = quickPickItem (&v, "use", shMenu::kCategorizeObjects);
        if (NULL == obj) {
            goto getcmd;
        }
        if (obj->isA (kTool)) {
            elapsed = useTool (obj);
        } else if (obj->isA (kCanister)) {
            elapsed = useCanister (obj);
        } else if (obj->isA ("empty ray gun")) {
            elapsed = loadRayGun (obj);
        } else if (obj->isWielded () && obj->isSelectiveFireWeapon ()) {
            elapsed = selectWeaponFireMode (obj);
        } else {
            abort ();
        }
        break;
    }
    case shInterface::kEditOptions:
    {
        I->editOptions ();
        I->crazyIvan (mIntrinsics & kCrazyIvan);
        goto getcmd;
    }
    case shInterface::kEditSkills:
    {
        editSkills ();
        goto getcmd;
    }
    case shInterface::kExecute:
    {
        shObject *computer;
        shObject *floppy;
        shObjectVector v;

        selectObjects (&v, mInventory, Computer);
        if (0 == v.count ()) {
            I->p ("You don't have a computer!");
            goto getcmd;
        } else if (1 == v.count ()) {
            computer = v.get (0);
        } else {
            computer = quickPickItem (&v, "compute with", 0);
            if (!computer) {
                goto getcmd;
            }
        }
        v.reset ();
        selectObjects (&v, mInventory, kFloppyDisk);
        if (0 == v.count ()) {
            I->p ("But you don't have any floppy disks.");
            goto getcmd;
        }
        floppy = quickPickItem (&v, "execute", 0);
        if (!floppy) {
            goto getcmd;
        }
        elapsed = executeFloppyDisk (computer, floppy);
        break;
    }
    case shInterface::kWield:
    {
        shObject *obj;
        shObjectVector v;
        
        selectObjects (&v, mInventory, kWeapon);
        obj = quickPickItem (&v, "wield", shMenu::kNothingAllowed |
                                          shMenu::kAnythingAllowed |
                                          shMenu::kCategorizeObjects);
        if (NULL == obj) {
            goto getcmd;
        }
        else if (0 == wield (obj)) {
            goto getcmd;
        }
        elapsed = HALFTURN;
        break;
    }
    case shInterface::kWear:
    {
        shObject *obj;
        shObjectVector v1, v2;

        selectObjects (&v1, mInventory, kArmor);
        unselectObjectsByFunction (&v2, &v1,& shObject::isWorn);
        obj = quickPickItem (&v2, "wear", 0);
        if (NULL == obj) {
            goto getcmd;
        }
        if (obj->isA (shBodyArmor) && NULL != mBodyArmor) {
            I->p ("You are already wearing armor.");
            goto getcmd;
        } else if (obj->isA (shJumpsuit) && NULL != mJumpsuit) {
            I->p ("You are already wearing a jumpsuit.");
            goto getcmd;
        } else if (obj->isA (shJumpsuit) && NULL != mBodyArmor) {
            I->p ("You'll have to take off your armor first.");
            goto getcmd;
        } else if (obj->isA (shHelmet) && NULL != mHelmet) {
            I->p ("You are already wearing headgear.");
            goto getcmd;
        } else if (obj->isA (shGoggles) && mGoggles) {
            I->p ("You are already wearing eye protection.");
            goto getcmd;
        } else if (obj->isA (shBelt) && NULL != mBelt) {
            I->p ("You are already wearing a belt.");
            goto getcmd;
        } else {
            if (obj->isWielded ()) {
                unwield (obj);
            }
            don (obj);
            elapsed = FULLTURN;
            break;
        }
    }
    case shInterface::kTakeOff:
    {
        shObject *obj;
        shObjectVector v;

        if (mBodyArmor)       v.add (mBodyArmor);
        else if (mJumpsuit)   v.add (mJumpsuit);
        if (mHelmet)          v.add (mHelmet);
        if (mGoggles)         v.add (mGoggles);
        if (mBelt)            v.add (mBelt);
        if (mBoots)           v.add (mBoots);

        obj = quickPickItem (&v, "take off", 0);
        if (NULL == obj) {
            goto getcmd;
        }
        if (obj == mJumpsuit && mBodyArmor) {
            I->p ("You'll have to take your armor off first.");
            goto getcmd;
        } else if (obj->isBuggy ()) {
            I->p ("You can't seem to take it off.  It must be buggy!");
            obj->setBugginessKnown ();
        } else {
            I->p ("You take off %s.", YOUR (obj));
            doff (obj);
        }
        elapsed = FULLTURN;
        break;
    }


    case shInterface::kInstall:
    {
        shObject *obj;
        shObjectVector v1, v2;

        selectObjects (&v1, mInventory, kImplant);
        unselectObjectsByFunction (&v2, &v1,& shObject::isWorn);
        obj = quickPickItem (&v2, "install", 0);
        if (NULL == obj) {
            goto getcmd;
        }
        if (mHelmet && mHelmet->isBuggy ()) {
            /* FIX: but what about in vacuum/poison gas cloud? 
                    (before, it was required that the hero remove helmet on 
                    his own, but I think that's kind of a pain...) CADV */
            I->p ("You can't remove your helmet to install the implant!");
            mHelmet->setBugginessKnown ();
            goto getcmd;
        } 

        if (obj->isWielded ()) {
            unwield (obj);
        }

        don (obj);
        elapsed = FULLTURN;
        break;
    }
    case shInterface::kUninstall:
    {
        shObject *obj;
        shObjectVector v;
        int i;

        for (i = 0; i < shImplantIlk::kMaxSite; i++) {
            if (mImplants[i]) {
                v.add (mImplants[i]);
            }
        }

        obj = quickPickItem (&v, "uninstall", 0);
        if (NULL == obj) {
            goto getcmd;
        }
        if (mHelmet && mHelmet->isBuggy ()) {
            /* FIX: but what about in vacuum, etc?  (see "install" above) */
            I->p ("You can't remove your helmet!");
            mHelmet->setBugginessKnown ();
            goto getcmd;
        } 
        if (obj->isBuggy ()) {
            I->p ("It won't come out!  It must be buggy!");
            obj->setBugginessKnown ();
        } else {
            I->p ("You uninstall %s.", YOUR (obj)); 
            doff (obj);
        }
        elapsed = FULLTURN;
        break;
    }

    case shInterface::kMutantPower:
    {
        elapsed = useMutantPower ();
        break;
    }

    case shInterface::kName:
    {
        shObject *obj;
        int ilk;

        ilk = ! I->yn ("Name an individual object?");
        obj = quickPickItem (mInventory, "name", 
                             shMenu::kCategorizeObjects);
        if (obj) {
            if (ilk) {
                obj->nameIlk ();
            } else {
                obj->name ();
            }
        }
        goto getcmd;
    }

    case shInterface::kToggleAutopickup:
        Flags.mAutopickup = !Flags.mAutopickup;
        I->p ("Autopickup is now %s.", Flags.mAutopickup ? "ON" : "OFF");
        goto getcmd;


    case shInterface::kShowArmor:
    {
        unsigned int i;
        shObject *objs[] = {mJumpsuit, mBodyArmor, mHelmet, 
                            mBoots, mGoggles, mBelt };
        int cnt = 0;

        shMenu menu ("Worn Armor", 
                     shMenu::kNoPick);
        for (i = 0; i < sizeof(objs) / sizeof (shObject*); i++) {
            if (objs[i]) {
                menu.addItem (objs[i]->mLetter, objs[i]->inv (),
                              objs[i], objs[i]->mCount);
                cnt++;
            }
        }
        if (cnt) {
            menu.finish ();
        } else {
            I->p ("You aren't wearing any armor.");
        }
        goto getcmd;
    }
    case shInterface::kShowImplants:
    {
        int i;
        int cnt = 0;

        shMenu menu ("Installed Implants", 
                     shMenu::kNoPick);
        for (i = 0; i < shImplantIlk::kMaxSite; i++) {
            if (mImplants[i]) {
                menu.addItem (mImplants[i]->mLetter, mImplants[i]->inv (),
                              mImplants[i], mImplants[i]->mCount);
                cnt++;
            }
        }
        if (cnt) {
            menu.finish ();
        } else {
            I->p ("You don't have any implants installed.");
        }
        goto getcmd;
    }
    case shInterface::kShowWeapons:
        if (!mWeapon) {
            I->p ("You are empty handed.");
        } else {
            I->p ("%c - %s", mWeapon->mLetter,  mWeapon->inv ());
        }
        goto getcmd;

    case shInterface::kVersion:
        I->showVersion ();
        goto getcmd;

    case shInterface::kSaveGame:
        if (I->yn ("Really save?")) {
            if (0 == saveGame ()) {
                GameOver = 1;
                I->pause ();
                return;
            } else {
                I->p ("Didn't save!");
            }
        } else {
            I->nevermind ();
        }
        goto getcmd;
    case shInterface::kQuit:
        if (I->yn ("Really quit?")) {
            die (kQuitGame, "quit");
            return;
        } else {
            I->nevermind ();
            goto getcmd;
        }

/* debugging commands */

    case shInterface::kGodMode:
        if (!GodMode) {
            I->p ("This command is only available in God Mode."); goto getcmd;
        }
        doGodMode ();
        goto getcmd;

    default:
//      I->p ("Impossible command.  The game is corrupt!");
        goto getcmd;
    }

    if (0 == elapsed && !GameOver) {
        goto getcmd;
    } else if (elapsed > 0) {
        mAP -= elapsed;
    }

    if (mXP / 1000 >= mCLevel) {
        if (mXP / 1000 > mCLevel) {
            /* don't allow multiple level gain */
            mXP = mCLevel * 1000;
        }
        levelUp ();
    }

    return;
}


int
shHero::die (shCauseOfDeath how, shCreature *killer)
{
    resetBlind ();
    Level->setLit (killer->mX, killer->mY, 1, 1, 1, 1);
    if (this == killer)
        return die (kKilled, her ("own weapon"));
    return die (how, AN (killer));
}


int
shHero::die (shCauseOfDeath how, const char *killer)
{
    int won = 0;
    int died = 1;
    char message[200];

    epitaph (message, 200, how, killer);

    Level->computeVisibility ();
    I->drawScreen ();
    switch (how) {
    case kSlain:
    case kKilled:
    case kMisc:
    case kSuicide:
        I->pause ();
        I->p ("You die."); break;
    case kBrainJarred:
        I->p ("You are now a brain in a jar."); 
        break;
    case kDrowned:
        I->p ("You drown."); break;
    case kAnnihilated:
        I->pause ();
        I->p ("You are annihilated."); break;
    case kEmbarassment:
        I->pause ();
        I->p ("You die of embarassment."); break;
    case kSuddenDecompression:
        I->pause ();
        switch (RNG (3)) { /* I just couldn't decide on one message */
        case 0:
            I->p ("Your lungs explode painfully."); break;
        case 1:
            I->p ("Your blood boils, enveloping your body in a crimson mist."); 
            break;
        case 2:
            I->p ("Your internal organs rupture and your eyeballs pop."); break;
        }
        break;
    case kTransporterAccident:
        I->pause ();
        I->p ("Your molecules are scrambled into a pile of gray goo."); 
        break;
    case kQuitGame:
        died = 0;
        goto nopause;
        break;
    case kWonGame:
        died = 0;
        won = 1;
        I->pause ();
        I->p ("Congratulations, you are the baddest motherfucker "
              "in the galaxy now!");
        break;
    }

    I->smallPause ();
nopause:
    if (!won && SpaceMarine == mProfession) { /* YAFM */
        I->p ("Game over, man!  Game over!");
        //I->smallPause ();
    }
    Level->clearSpecialEffects ();

    if (I->yn ("Do you want your possessions identified?")) {
        int i;

        for (i = 0; i < mInventory->count (); i++) {
            mInventory->get (i) -> identify ();
        }
        listInventory ();
    }
    if (I->yn ("Would you like to see the console message history?")) {
        I->showHistory ();
    }

    tallyScore ();

    if (kWonGame == how || kQuitGame == how) {

    } else {
        tomb (message);
    }

    logGame (message);

    I->p ("Goodbye...");

    I->debug ("Hero is slain!");
    mState = kDead;
    GameOver = 1;
    return 1;
}

//REQUIRES: monster types initialized already

void
shHero::init (const char *name, shProfession *profession)
{
    int x = 1;
    int y = 1;
    
    mIlk = findAMonsterIlk ("earthling");
    strncpy (mName, name, HERO_NAME_LENGTH);
    mName[HERO_NAME_LENGTH] = 0;
    mCLevel = 1;
    mBAB = 0;
    mInateResistances[kMagnetic] = 100;

    profession->mInitFunction (this);

    mAP = -1000;
    mHP = mMaxHP;

    computeIntrinsics ();
    computeAC ();

    mScore = 0;
    mSkillPoints = 0;
    mXP = 0;

    initGlyph (&mGlyph, kSymHero, kWhite, 0);
    
    while (1) {
        Level->findUnoccupiedSquare (&x, &y);
        if (!Level->isInShop (x, y) &&
            Level->isInRoom (x, y)) 
        {
            break;
        }
    }
    Level->putCreature (this, x, y);

}


shHero Hero;




