#include <math.h>
#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Profession.h"
#include "Creature.h"
#include "Interface.h"
#include "Game.h"

#include <ctype.h>

char *
shHero::getDescription (char *buff, int buflen) 
{
    snprintf (buff, buflen, "hero");
    return buff;
}


char *
shHero::the (char *buff, int buflen) 
{
    snprintf (buff, buflen, "you");
    return buff;
}


char *
shHero::an (char *buff, int buflen) 
{
    snprintf (buff, buflen, "you");
    return buff;
}


char *
shHero::your (char *buff, int buflen) 
{
    snprintf (buff, buflen, "you");
    return buff;
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

        menuresult = menu.getResult ((void **) &choice, NULL);
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
shHero::newLocation ()
{
    shRoom *oldroom = Level->getRoom (mLastX, mLastY); 
    shRoom *newroom = Level->getRoom (mX, mY); 

    pickedupitem = 0;
    if (Flags.mAutopickup) {
        shObjectVector *v = Level->getObjects (mX, mY);
        int i;

        if (v) for (i = 0; i < v->count (); i++) {
            shObject *obj = v->get (i);
            if (Flags.mAutopickupTypes[obj->mIlk->mType]) {
                interrupt ();
                if (!isBlind ()) obj->setAppearanceKnown ();
                if (addObjectToInventory (obj)) {
                    v->remove (obj); 
                    --i;
                    pickedupitem++;
                    if (Level->isInShop (mX, mY)) {
                        pickedUpItem (obj);
                    }
                }
            }
        }
    }

    if (newroom == oldroom) return;
    
    if (Level->isInShop (mX, mY)) {
        enterShop ();
    }
    if (mLastLevel && mLastLevel->isInShop (mLastX, mLastY)) {
        leaveShop ();
    }
    if (shRoom::kNest == newroom->mType) {
        I->p ("You enter an alien nest!");
    }
}


void
shHero::lookAtFloor (int menuok /* = 0 */ )
{
    int objcnt = Level->countObjects (mX, mY);
    shObjectVector *v;
    int i;
    char buf[50];
    shFeature *f = Level->getFeature (mX, mY);
    
    feel (mX, mY);

    if (0 == objcnt && !menuok && !pickedupitem) {
        return;
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
        case shFeature::kTrapDoor:
            I->p ("There is a trap door here."); break;
        case shFeature::kHole:
            I->p ("There is a hole here."); break;
        case shFeature::kRadTrap:
            I->p ("There is a radiation trap here."); break;
        default:
            I->p ("There is a strange feature here.");
        }
        interrupt ();
    }
    if (0 == objcnt) return;
    if (isBlind ()) {
        if (isFlying ()) return;
        I->p ("You feel around the floor...");
    } 
    interrupt ();
    if (objcnt > 4) {
        if (!menuok) {
            I->p ("There are several objects here.");
            return;
        }
        else {
            v = Level->getObjects (mX, mY);
            shMenu menu ("Things that are here:", 
                         shMenu::kNoPick | shMenu::kCategorizeObjects);
            shObject *obj;
            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);
                char buf[80];
                if (!isBlind ()) obj->setAppearanceKnown ();
                obj->an (buf, 80);
                menu.addItem (i < 26 ? i + 'a' : i + 'A', 
                              buf, obj, obj->mCount);
            }
            menu.finish ();
            return;
        }
    }
    v = Level->getObjects (mX, mY);
    if (1 == objcnt) {
        if (!isBlind ()) v->get (0) -> setAppearanceKnown ();
        v->get (0) -> an (buf, 50);
        I->p ("You %s %s.", isBlind () ? "find" : "see here", buf);
        return;
    }
    I->p ("Things that are here:");
    for (i = 0; i < objcnt; i++) {
        if (!isBlind ()) v->get (i) -> setAppearanceKnown ();
        v->get (i) -> an (buf, 50);
        I->p ("%s", buf);
    }
}


int
shHero::tryToTranslate (shCreature *c)
{
    char talker[50];
    char talkerid[20];
    char buf[50];
    int i;

    c->the (talker, 50);
    snprintf (talkerid, 50, "translate %p", c);

    if (hasTranslation ()) {
        assert (mImplants[shImplantIlk::kLeftEar]);

        if (1 != getStoryFlag (talkerid)) {
            mImplants[shImplantIlk::kLeftEar]->your (buf, 50);
            I->p ("%s translates %s's %s:", buf, talker, 
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
                m->your (buf, 50);
                I->p ("%s translates %s's %s:", buf, talker,
                      c->isRobot () ? "beeps and chirps" : "alien language");
                setStoryFlag (talkerid, 2);
            }
            return 1;
        }
    }
    return 0;
}


void
shHero::spotStuff ()
{
    shCreature *c;
    shFeature *f;
    int i;
    char buf[50];
    int peril = 0;
    int sk = getSkillModifier (kSpot);
    int score;


    if (isBlind ()) {
        goto done;
    }
    for (i = 0; i < Level->mCrList.count (); i++) {
        c = Level->mCrList.get (i);
        if (c == this || !c->isHostile () || !Level->isInLOS (c->mX, c->mY)) {
            continue;
        }
        if (hasPerilSensing ()) {
            peril = 1;
            break;
        } 
        if (c->mHidden > 0 && c->mSpotAttempted + 160000 < Clock &&
            distance (this, c->mX, c->mY) < 30) 
        { /* note that peril-sensitive sunglasses go black before 
             the poor hero gets a chance to notice a hidden monster */
            c->mSpotAttempted = Clock;
            score = sportingD20 () + sk;
            I->debug ("spot check: %d", score);
            if (score > c->mHidden) {
                I->p ("You spot %s!", c->an (buf, 50));
                c->mHidden *= -1;
                Level->drawSq (c->mX, c->mY, 1);
                I->pauseXY (c->mX, c->mY);
                interrupt ();
            }
        }
    }

    for (i = 0; i < Level->mFeatures.count (); i++) {
        f = Level->mFeatures.get (i);
        if (f->isTrap () && Level->isInLOS (f->mX, f->mY)) {
            peril = 1;
        }
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
                    f->getDescription (buf, 40);
                    I->p ("You spot a %s!", buf);
                }
                f->mTrapUnknown = 0;
                f->mSportingChance = 0;
                Level->drawSqTerrain (f->mX, f->mY);
                I->pauseXY (f->mX, f->mY);
                interrupt ();
            }
        }
    }

done:

    if (hasPerilSensing ()) {
        if (peril) {
            if (!mGoggles->isToggled ()) {
                mGoggles->your (buf, 50);
                mGoggles->setToggled ();
                interrupt ();
                computeIntrinsics ();
                if (hasXRayVision ()) {
                    I->p ("%s have darkened a bit.", buf);
                } else {
                    I->p ("%s have turned black!", buf);
                }
            }
        } else if (mGoggles->isToggled ()) {
            mGoggles->your (buf, 50);
            mGoggles->resetToggled ();
            computeIntrinsics ();
            I->p ("%s have turned transparent.", buf);
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
            char buf[80];
            mWeapon->your (buf, 80);
            I->p ("%s is welded to your hand!", buf);
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
        char buf[80];

        mWeapon = obj;
        if (0 == quiet) {
            obj->an (buf, 80);
            I->p ("You now wield %s.", buf);
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
    char buf[80];
    shFeature *f = Level->getFeature (mX, mY);
    int iscan = obj->isA (kCanister);

    obj->an (buf, 80);
    
    if (f && shFeature::kVat == f->mType &&
        I->yn ("%s %s into the vat?", iscan ? "Pour" : "Drop", buf))
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
        I->p ("You drop %s.", buf);
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
            char buf[80];
            obj->inv (buf, 80);
            menu.addItem (obj->mLetter, buf, obj, obj->mCount);
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
    char buf[64];

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
                        obj->your (buf, 64);
                        I->p ("%s has shut itself off.", buf);
                    }
                    obj->mLastEnergyBill += obj->mIlk->mEnergyUse;
                }
            }
        }
    }
    if (hasAutoRegeneration ()) {
        while (Clock - mLastRegen > 2500) {
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
    char buf[80];
    int speed;

    speed = isDiagonal (dir) ? DIAGTURN: FULLTURN;

    if (!Level->moveForward (dir, &x, &y)) {
        I->p ("You can't move there!");
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
            Level->countObjects (tx, ty) ||
            Level->getKnownFeature (tx, ty))
        {
            interrupt (); 
            //return 0;
        }
        tx = x; ty = y;
        if (!Level->moveForward (rightTurn (dir), &tx, &ty) ||
            rightsq != Level->appearsToBeFloor (tx, ty) ||
            Level->countObjects (tx, ty) ||
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
                I->p ("You bump into %s", Level->the (buf, 80, x, y));
                return speed;
            }
        }
    } else if (Level->isOccupied (x, y)) {
        feel (x, y);
        if (!interrupt ()) {
            if (isStunned () || isConfused () || isBlind ()) {
                I->p ("You bump into %s", 
                      Level->getCreature (x, y) -> the (buf, 80));
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

    /* check for alien impregnation */
    {
        int preggers = getStoryFlag ("impregnation");

        if (preggers > 0) {
            preggers++;
            if (preggers > 20) {
                I->p ("The alien creature explodes from your chest!");
                /* died in childbirth? */
                die (kSlain, "alien impregnation");
            } else if (18 == preggers) {
                I->p ("You are violently ill!");
                makeStunned (35000);
                interrupt ();
            } else if (15 == preggers) {
                I->p ("You vomit!");
                interrupt ();
            } else if (10 == preggers) {
                I->p ("You feel nauseated.");
                interrupt ();
            } else if (5 == preggers) {
                I->p ("You feel something moving inside you!");
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
            d += RNG (1, 10);
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
                mRad / 10) 
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
                        I->p (RNG (2) ? "You have a headache." : 
                              "You feel tired."); break;
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
            char buf[80];
            int i;
            shObject *obj = NULL;

            for (i = 0; i < mInventory->count (); i++) {
                obj = mInventory->get (i);
                if (obj->isA (findAnIlk (&ToolIlks, "geiger counter", 1))) {
                    obj->your (buf, 80);
                    break;
                }
            }
            if (0 == ol) {
                I->p ("%s is making a clicking noise.", buf);
                if (obj) obj->setToggled ();
            } else if (0 == rl) {
                I->p ("%s has stopped clicking.", buf);
                if (obj) obj->resetToggled ();
            } else if (ol < rl) {
                I->p ("%s is clicking more rapidly.", buf);
            } else {
                I->p ("%s is clicking more slowly.", buf);
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
    case shInterface::kMoveN:
        --dy;  goto domove;
    case shInterface::kMoveNE:
        ++dx; --dy; goto domove;
    case shInterface::kMoveE:
        ++dx; goto domove;
    case shInterface::kMoveSE:
        ++dx; /* fall through */
    case shInterface::kMoveS:
        ++dy; goto domove;
    case shInterface::kMoveSW:
        ++dy; /* fall through */
    case shInterface::kMoveW:
        --dx; goto domove;
    case shInterface::kMoveNW:
        --dx; --dy;

    domove:
        {
            int x = mX + dx;
            int y = mY + dy;
            char buf[50];
            shCreature *c;

            c = Level->getCreature (x, y);

            if (Level->rememberedCreature (x, y) || 
                (c && !isBlind ())) 
            { /* this must be an attack */
                if (glidemode) {
                    goto getcmd;
                }
                
                if (c && c->isPet () && !isStunned () && !isConfused ()) {
                    c->your (buf, 50);
                    elapsed = displace (c);
                } else {
                    if (c && isFrightened () && 
                             !isConfused () && !isStunned () && !isBlind ()) 
                    {
                        c->the (buf, 50);
                        I->p ("You are too afraid to attack %s!", buf);
                        goto getcmd;
                    }
                    if (c && !c->isHostile () && !c->isA ("monolith") &&
                        !isBlind () && !isConfused ()) 
                    {
                        c->the (buf, 50);
                        if (!I->yn ("Really attack %s?", buf)) {
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
                if (--mTrapped <= 0) {
                    I->p ("You free yourself from the trap.");
                }
                elapsed = FULLTURN;
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
            I->p ("Kicking yourself isn't going to make things any better");
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
        char buf[20];
        shMapLevel *oldlevel = Level;
        shFeature *stairs = Level->getFeature (mX, mY);

        if (NULL == stairs || 
            !(shFeature::kStairsDown == stairs->mType ||
              shFeature::kTrapDoor == stairs->mType || 
              shFeature::kHole == stairs->mType)) 
        {
            I->p ("You can't go down here.");
            goto getcmd;
        }
        stairs->the (buf, 20);
        if (isFlying ()) {
            I->p ("You are flying high above %s.", buf);
            goto getcmd;
        }
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
                if (!isBlind ()) v->get (0) -> setAppearanceKnown ();
                if (1 == addObjectToInventory (v->get (0))) {
                    if (Level->isInShop (mX, mY)) {
                        pickedUpItem (v->get (0));
                    }
                    delete v;
                    Level->setObjects (mX, mY, NULL);
                }
            }
            else {
                int i, cnt;
                shMenu menu ("Pick up what?", 
                             shMenu::kMultiPick | shMenu::kCountAllowed |
                             shMenu::kCategorizeObjects);

                v->sort (&compareObjects);
                for (i = 0; i < v->count (); i++) {
                    obj = v->get (i);
                    char buf[80];
                    if (!isBlind ()) obj->setAppearanceKnown ();
                    obj->an (buf, 80);
                    menu.addItem (i < 26 ? i + 'a' : i + 'A', 
                                  buf, obj, obj->mCount);
                }
                objcnt = 0;
                while (menu.getResult ((void **) &obj, &cnt)) {
                    assert (cnt);
                    ++objcnt;
                    if (cnt != obj->mCount) {
                        obj = obj->split (cnt);
                    }
                    else {
                        v->remove (obj);
                    }
                    if (0 == addObjectToInventory (obj)) {
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
        char buf[80];

        obj = quickPickItem (mInventory, "adjust", shMenu::kCategorizeObjects);
        if (NULL == obj) {
            goto getcmd;
        }
        I->p ("Adjust to what letter?");
        c = I->getChar ();
        if (isalpha (c) && c != obj->mLetter) {
            int i;
            for (i = 0; i < mInventory->count (); i++) {
                obj2 = mInventory->get (i);
                if (obj2->mLetter == c) {
                    obj2->mLetter = obj->mLetter;
                    obj->mLetter = c;
                    obj->inv (buf, 80);
                    I->p ("%c - %s", obj->mLetter, buf);
                    obj2->inv (buf, 80);
                    I->p ("%c - %s", obj2->mLetter, buf);
                    goto getcmd;
                }
            }
            obj->mLetter = c;
            obj->inv (buf, 80);
            I->p ("%c - %s", obj->mLetter, buf);
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
                char buf[80];
                obj->inv (buf, 80);
                menu.addItem (obj->mLetter, buf, obj, obj->mCount);
            }
            
            while (menu.getResult ((void **) &obj, &cnt)) {
                char buf[80];
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
                    obj->your (buf, 80);
                    I->p ("You can't drop %s because you're wearing it.", buf);
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
        char buf[80];
        int ofs = 0;
        while (I->getSquare ("Look at what? (select a location, '?' for help)",
                             &x, &y, -1))
        {
            shCreature *c = Level->getCreature (x, y);
            shFeature *f = Level->getFeature (x, y);
            int seen = 0;

            if (c) {
                ofs = 0;
                if (c->isHostile ()) {
                    ofs += strlen (c->an (&buf[ofs], 80));
                } else {
                    ofs += snprintf (&buf[ofs], 80 - ofs, "a peaceful ");
                    ofs += strlen (c->getDescription (&buf[ofs], 80));
                }
                if (canSee (c) && c->mHidden <= 0) {
                    if (c->mWeapon) {
                        ofs += snprintf (&buf[ofs], 80 - ofs, ", wielding ");
                        ofs += c->mWeapon->an (&buf[ofs], 80 - ofs);
                    }
                    I->p ("You see %s.", buf);
                    seen++;
                } else if (hasTelepathy () && c->hasMind () && 
                           distance (this, x, y) < 5 * (mCLevel + 20))
                {
                    I->p ("You sense %s.", buf);
                    seen++;
                } else if (hasMotionDetection () && 
                           c->isMoving () &&
                           distance (this, x, y) < 50) 
                {
                    I->p ("You see a blip on your motion tracker.", buf);
                    seen++;
                } else if (c->mHidden) {
                    switch (c->mMimic) {
                    case shCreature::kObject:
                        /* TEMP FIX FIX FIX 
                           right now only used by creeping credits */
                        c->mInventory->get (0) -> an (buf, 80);
                        I->p ("You see %s.", buf);
                        seen++;
                        break;
                    default:
                        /* nothing seen */
                        break;
                    }
                }
            } else if (!canSee (x, y)) {
                I->p ("You can't see that location from here.");
                continue;
            } 

            if (Level->countObjects (x, y)) {
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
                bestobj->an (buf, 80);
                I->p ("You see %s.", buf);
            } 
            if (f && !(f->isTrap () && f->mTrapUnknown)) {
                switch (f->mType) {
                case shFeature::kDoorHiddenVert:
                case shFeature::kDoorHiddenHoriz:
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
                case shFeature::kRadTrap:
                    I->p ("You see a radiation trap."); break;
                }
            } else {
                if (Level->isFloor (x, y)) {
                    if (!seen) I->p ("You see the floor.");
                } else {
                    I->p ("You see a wall.");
                }
            }
            I->pause ();
        }
        I->p ("Done.");
        goto getcmd;
    }

    case shInterface::kPay:
    {
        payShopkeeper ();
        goto getcmd;
    }

    case shInterface::kSearch:
    {
        int x, y, sk;
        char buf[50];

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
                            f->getDescription (buf, 40);
                            I->p ("You find a %s trap!", buf);
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
        editSkills (0);
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
            char buf[80];
            obj->your (buf, 80);
            doff (obj);
            I->p ("You take off %s.", buf);
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
            char buf[50];
            obj->your (buf, 50);
            I->p ("You uninstall %s.", buf);        
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

    if (0 == elapsed) {
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
    char buf[80];
    resetBlind ();
    Level->setLit (killer->mX, killer->mY);
    killer->an (buf, 80);
    return die (how, buf);
}


int
shHero::die (shCauseOfDeath how, char *killer)
{
    int won = 0;
    int died = 1;
    char message[200];
    switch (how) {
    case kSlain:
    case kKilled:
    case kMisc:
    case kSuicide:
        I->pause ();
        I->p ("You die."); break;
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
        I->smallPause ();
    }
    if (I->yn ("Do you want your possessions identified?")) {
        int i;

        for (i = 0; i < mInventory->count (); i++) {
            mInventory->get (i) -> identify ();
        }
        listInventory ();
    }

    tallyScore ();

    deathMessage (message, 200, how, killer);
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
shHero::init (char *name, shProfession *profession)
{
    int x = 1;
    int y = 1;
    
    mIlk = findAMonsterIlk ("earthling");
    strncpy (mName, name, 14);
    mCLevel = 1;
    mBAB = 0;
    mInateResistances[kMagnetic] = 100;

    profession->mInitFunction (this);

    mAP = -1000;
    mHP = mMaxHP;

    computeIntrinsics ();
    computeAC ();

    mScore = 0;

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




