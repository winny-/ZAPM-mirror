/*******************************

        Mutant Power Code

how to add a mutant power:
1. edit shMutantPower enum in Global.h
2. edit the little table below, and make sure the order is consistent with
   the enum!
3. write a usePower function, or startPower/stopPower functions
4. possibly write shCreature::Power function if you want monsters to use it too
5. add the appropriate skill to the Psion and maybe other character classes

********************************/

#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"
#include "Interface.h"

static int 
shortRange (shCreature *c)
{
    return 5 + c->mCLevel / 4;
}


int
shCreature::getPsionicDC (int powerlevel)
{
    return 10 + powerlevel + ABILITY_MODIFIER (getCha () + mChaDrain);
}


/* roll to save against the psionic attack
   returns: 0 if save failed, true if made
*/

int
shCreature::willSave (int DC)
{
    int modifier =  mWillSaveBonus + ABILITY_MODIFIER (getWis ());
    int result = RNG (1, 20) + modifier;

    if (Hero.isLucky ()) {
        if (isHero () || isPet ()) {
            result += RNG (1, 3);
        }
    }

    I->debug ("Rolled will save %d+%d=%d, dc %d",
              result - modifier, modifier, result, DC);

    return result >= DC;
}



int
shCreature::digestion (shObject *obj)
{
    if (isHero ()) {
        if (obj->isUnpaid())
            I->p ("You devour %s.", THE (obj));
        else 
            I->p ("You devour %s.", YOUR (obj));
        Hero.usedUpItem(obj, obj->mCount, "ate");
    } else {
        abort ();
    }
    removeObjectFromInventory (obj);
    if (obj->isA ("bizarro orgasmatron")) {
        I->p ("You feel eating that was a bad idea.");
    }
    delete obj;

    return FULLTURN;
}


int
shCreature::telepathy (int on)
{
    if (on) {
        mInateIntrinsics |= kTelepathy;
        //Hero.setTimeOut (TELEPATHY, 1000 * NDX (5, 10), 0);
    } else {
        mInateIntrinsics &= ~kTelepathy;
    }
    Hero.computeIntrinsics ();
    return FULLTURN;
}


int
shCreature::hypnosis (shDirection dir)
{
    int x = mX;
    int y = mY;
    shCreature *target;

    if (mLevel->moveForward (dir, &x, &y) &&
        (target = mLevel->getCreature (x, y)))
    {
        int saved = target->hasBrainShield () || 
                    target->willSave (getPsionicDC (1));

        if (target->isHero ()) {
            if (!Hero.canSee (this)) {
                /* you can't see the hypnotic gaze */
            } else if (saved) {
                I->p ("You blink.");
            } else {
                I->p ("You fall asleep!");
                target->makeAsleep (NDX (2, 10) * 1000);
            }
        } else {
            const char *t_buf;

            if (Hero.canSee (target)) {
                t_buf = THE (target);
            } else {
                t_buf = "it";
            }
            if (!target->canSee (this)) {
                I->p ("%s doesn't seem to notice your gaze.");
            } else if (!target->hasMind ()) {
                I->p ("%s is unaffected.", t_buf);
            } else if (saved) {
                I->p ("%s resists!", t_buf);
            } else {
                I->p ("%s falls asleep!", t_buf);
                target->makeAsleep (NDX (2, 10) * 1000);
            }
        }
    }
    return 500;
}


shAttack WebAttack = 
    shAttack (NULL, shAttack::kWeb, shAttack::kSingle, kAimed | kPsychic, 
              kWebbing, 0, 6, 
              FULLTURN, 1, 12);

int
shCreature::shootWeb (shDirection dir)
{
    int x, y, r;
    int maxrange = shortRange (this);
    int elapsed = 800;
    int attackmod;

    shSkill *skill = getSkill (kMutantPower, kShootWebs);


    WebAttack.mDamage[0].mNumDice = mCLevel / 2 + 1 
                                  + (skill ? 2 * skill->mRanks : 0);

    attackmod = mBAB + mToHitModifier 
              + getSkillModifier (kMutantPower, kShootWebs);

    if (kUp == dir) {
        return elapsed;
    } else if (kDown == dir) {
        resolveRangedAttack (&WebAttack, NULL, attackmod, this);
        return elapsed;
    }

    x = mX;    y = mY;

    while (maxrange--) {
        shFeature *f = Level->getFeature (x, y);

        if (!Level->moveForward (dir, &x, &y)) {
            Level->moveForward (uTurn (dir), &x, &y);
            return elapsed;
        }

        if (Level->isOccupied (x, y)) {
            shCreature *c = Level->getCreature (x, y);
            if (c->mZ < 0 && f) {
                /* sails overhead.  This is a kludge to avoid confusion over
                   the cause of mTrapped (pit or web?) */
                continue;
            }
            r = resolveRangedAttack (&WebAttack, NULL, attackmod, c);
            if (r >= 0) 
                return elapsed;
        }
        if (Level->isObstacle (x, y)) {
            return elapsed;
        }
    }

    return elapsed;
}


shAttack OpticBlastAttack = 
    shAttack (NULL, shAttack::kLaser, shAttack::kBeam, kAimed | kPsychic, 
              kLaser, 0, 6, 
              FULLTURN, 1, 12);

int
shCreature::opticBlast (shDirection dir)
{
    int x, y;
    //int maxrange = shortRange (this);
    int elapsed = 800;

    OpticBlastAttack.mDamage[0].mNumDice = mCLevel;

    if (kUp == dir) {
        /* TODO: maybe loosen debris from the ceiling? */
        return elapsed;
    } else if (kDown == dir) {
        return elapsed;
    }

    x = mX;    y = mY;

    int attackmod = getSkillModifier (kMutantPower, kOpticBlast)
                   + mBAB + mToHitModifier;

    int foo = Level->areaEffect (&OpticBlastAttack, NULL, mX, mY, 
                                 dir, this, attackmod);
    if (foo < 0)
        return foo;
    return elapsed;
}


int
shCreature::xRayVision (int on)
{
    assert (isHero ());
    if (on) {
        if (!hasXRayVision () && 
            hasPerilSensing () && 
            mGoggles->isToggled ())
        {
            I->p ("You can see through your darkened goggles now.");
        }
        mInateIntrinsics |= kXRayVision;
        //Hero.setTimeOut (XRAYVISION, 1000 * NDX (10, 10), 0);
    } else {
        mInateIntrinsics &= ~kXRayVision;
        if (isHero () && 
            hasPerilSensing () && 
            mGoggles->isToggled ()) 
        {
            I->p ("You can no longer see through your darkened goggles.");
        }
    }
    Hero.computeIntrinsics ();
    return 500;
}

shAttack MentalBlastAttack = 
    shAttack (NULL, shAttack::kMentalBlast, shAttack::kSingle, 0, 
              kPsychic, 0, 6, 
              FULLTURN, 1, 999999, kConfusing, 1);

shAttack SavedMentalBlastAttack = 
    shAttack (NULL, shAttack::kMentalBlast, shAttack::kSingle, 0,
              kPsychic, 0, 3, 
              FULLTURN, 1, 999999);

int
shCreature::mentalBlast (int x, int y)
{
    shCreature *c = mLevel->getCreature (x, y);
    int elapsed = 1500;
    const char *t_buf;
    int shielded;
    shAttack *atk;
    
    if (!c) {
        return elapsed;
    }
    shielded = c->hasBrainShield ();

    t_buf = THE (c);

    if (c->isHero ()) {
        if (isHero ()) {
            I->p ("You blast yourself with psionic energy!");
            shielded = 0;
        } else {
            if (shielded) {
                I->p ("Your scalp tingles.");
                return elapsed;
            }
            I->p ("You are blasted by a wave of psionic energy!");
            if (!Hero.canSee (this)) {
                Level->drawSqCreature (mX, mY);
                I->pauseXY (mX, mY);
            }
        }
    } else if (isHero ()) {
        if (c->hasMind ()) {
            if (!canSee (c)) {      
                Level->setVisible (x, y, 1);
                Level->drawSqCreature (x, y);
                t_buf = THE (c);
                Level->setVisible (x, y, 0);
                I->p ("You blast %s with a wave psionic energy.", t_buf);
                I->pauseXY (x, y);
            } else {
                I->p ("You blast %s with a wave psionic energy.", THE (c));
            }
        } else if (!canSee (x, y)) {
            return elapsed;
        }
    }
    if (!c->hasMind () || c->hasBrainShield ()) {
        if (Hero.canSee (x, y)) {
            I->p ("%s is not affected.", t_buf);
        }
        return elapsed;
    }

    if (c->willSave (getPsionicDC (3))) {
        atk = &SavedMentalBlastAttack;
    } else { /* not confused if save made */
        atk = &MentalBlastAttack;
    }
    atk->mDamage[0].mNumDice = (mCLevel + 1) / 2;

    if (c->sufferDamage (atk, this, 1, 1)) {
        if (isHero () && !c->isHero ()) {
            Hero.earnXP (c->mCLevel);
        }
        if (c->isHero ()) { 
        } else if (isHero () || Hero.canSee (c)) {
            I->p ("%s is killed!", t_buf);
        }
        c->die (kSlain, "a mental blast");
    }
    return elapsed;
}


int
shCreature::regeneration ()
{
    mHP += NDX (mCLevel, 4);
    if (mHP > mMaxHP) {
        mHP = mMaxHP; 
    }
    return FULLTURN;
}


/* returns: */
int
shCreature::restoration ()
{
    int i;
    int permute[7] = {1, 2, 3, 4, 5, 6, 7};
    int helped = 0;

    shuffle (permute, 7, sizeof (int));
    for (i = 0; i < 7; i++) {
        int a = mAbil.getByIndex (permute[i]);
        int m = mMaxAbil.getByIndex (permute[i]);
        int b = RNG (2, 4);

        if (isHero () && kCha == permute[i]) { 
           /* don't restore charisma drain */
            a += Hero.mChaDrain;
        }
        if (a < m) {
            if (helped) {
                computeIntrinsics ();
                if (isHero ()) {
                    I->p ("You feel restored.");
                }
                return 1;
            }
            if (a + b > m) {
                b = m - a;
                helped = 1;
            }
            if (kCon == permute[i]) {
                int hpgain = mCLevel * 
                    (ABILITY_MODIFIER (a + b) - ABILITY_MODIFIER (a));
                mMaxHP += hpgain;
                mHP += hpgain;
            }
            mAbil.setByIndex (permute[i], a + b);
            if (a + b == m) {
                helped = 1;
                break;
            } else {
                computeIntrinsics ();
                if (isHero ()) {
                    I->p ("You feel restored.");
                }
                return 1;
            }
            break; /* why is this break here?  I think we should keep looping */
        }
    }
    if (helped) {
        computeIntrinsics ();
        if (isHero ()) {
            I->p ("You feel fully restored.");
        }
        return 1;
    } else {
        if (isHero ()) {
            I->p ("You feel great!");
        } 
        return 0;
    }
}


/* is this balanced? trade -4 cha for +4 str - is awesome for non-psions! */
int
shCreature::adrenalineControl (int on)
{
    mAbil.mStr += on ? 4 : -4;
    mMaxAbil.mStr += on ? 4 : -4;
    computeIntrinsics ();
    return FULLTURN;
}


int
shCreature::haste (int on)
{
    mSpeedBoost += on ? 25 : -25;
    computeIntrinsics ();
    return FULLTURN;
}

static int
useDigestion (shHero *h)
{
    if (Hero.getStoryFlag ("superglued tongue")) {
        I->p ("You can't eat anything with this stupid canister "
              "glued to your mouth!");
        return 0;
    }

    shObject *obj = 
        h->quickPickItem (h->mInventory, "eat", shMenu::kAnythingAllowed | 
                          shMenu::kCategorizeObjects); 
    if (obj) {
        /* objects on your head can't be eaten conveniently, but otherwise
           we'll assume that digestion comes along with super flexibility 
           required to eat belts, boots, armor, etc.
         */
        if (obj->isA (kImplant) && obj->isWorn ()) {
            I->p ("You'll have to uninstall that first.");
        } else if (h->mHelmet == obj || h->mGoggles == obj) {
            I->p ("You'll have to take it off first.");
        } else {
            return h->digestion (obj);
        }
    } else {
        I->nevermind ();
    }
    return 0;
}


static int
useAdrenalineControl (shHero *h)
{
    return h->adrenalineControl (1);
}

static int
stopAdrenalineControl (shHero *h)
{
    return h->adrenalineControl (0);
}


static int
useHaste (shHero *h)
{
    return h->haste (1);
}

static int
stopHaste (shHero *h)
{
    return h->haste (0);
}


static int
useTelepathy (shHero *h)
{
    return h->telepathy (1);
}

static int
stopTelepathy (shHero *h)
{
    return h->telepathy (0);
}


shAttack BurningGogglesAttack = 
    shAttack (NULL, shAttack::kBlast, shAttack::kSingle, 
              0, kBurning, 2, 6);

static int
useOpticBlast (shHero *h)
{
    if ((h->hasPerilSensing () && h->mGoggles->isToggled ()) ||
        (h->mGoggles && h->mGoggles->isA ("blindfold")))
    {
        I->p ("You incinerate your goggles!");
        if (h->sufferDamage (&BurningGogglesAttack)) {
            h->die (kMisc, "Burned his face off");
        }
        h->removeObjectFromInventory (h->mGoggles);
        return FULLTURN;
    }
    if (h->isBlind ()) {
        I->p ("But you are blind!");
        return 0;
    } else {
        shDirection dir = I->getDirection ();
        if (kNoDirection == dir) {
            return 0;
        } else {
            return h->opticBlast (dir);
        }
    }
}

static int
useRegeneration (shHero *h)
{
    return h->regeneration ();
}


static int
useRestoration (shHero *h)
{
    return h->restoration ();
}


static int
useHypnosis (shHero *h)
{
    shDirection dir = I->getDirection ();
    if (kNoDirection == dir) {
        return 0;
    } else {
        return h->hypnosis (dir);
    }
}
 

static int
useXRayVision (shHero *h)
{
    return h->xRayVision (1);
}

static int
stopXRayVision (shHero *h)
{
    return h->xRayVision (0);
}


static int
useTeleport (shHero *h)
{
    int x, y;
    Level->findSquare (&x, &y);
    h->transport (x, y, 100);
    return 500;
}


static int
useIllumination (shHero *h)
{
    int x, y;
    int radius = 4; /* vary according to level? */

    for (x = h->mX - radius; x <= h->mX + radius; x++) {
        for (y = h->mY - radius; y <= h->mY + radius; y++) {
            if (Level->isInBounds (x, y) && Level->isInLOS (x, y)
                && distance (x, y, h->mX, h->mY) < radius * 5) 
            {
                Level->setLit (x, y, 1, 1, 1, 1);
            }
        }
    }
    return 500;
}


static int
useWeb (shHero *h)
{
    shDirection dir = I->getDirection ();
    if (kNoDirection == dir) {
        return 0;
    } else {
        return h->shootWeb (dir);
    }
}


static int
useMentalBlast (shHero *h)
{
    int x = -1, y = -1;
    if (0 == I->getSquare ("What do you want to blast?  (select a location)",
                           &x, &y, shortRange (h)))
    {
        return 0;
    } else {
        return h->mentalBlast (x, y);
    }
}


struct MutantPower { 
    int mLevel;
    const char *mName; 
    int (*mFunc) (shHero *);
    int (*mOffFunc) (shHero *);

    int isPersistant () { return NULL != mOffFunc ? 1 : 0; }
};

/* make sure these agree with the enum def in Global.h: */
MutantPower MutantPowers[kMaxMutantPower] = 
{ { 0, "empty", NULL, NULL },
  { 1, "Illumination", useIllumination, NULL },
  { 1, "Digestion", useDigestion, NULL },
  { 1, "Hypnosis", useHypnosis, NULL },
  { 1, "Regeneration", useRegeneration, NULL },
  { 2, "Optic Blast", useOpticBlast, NULL },
  { 2, "Haste", useHaste, stopHaste },
  { 2, "Telepathy", useTelepathy, stopTelepathy },
  { 2, "Web", useWeb, NULL },
  { 3, "Mental Blast", useMentalBlast, NULL},
  { 3, "Pyrokinesis", NULL, NULL },
  { 3, "Restoration", useRestoration, NULL },
  { 4, "Adrenaline Control", useAdrenalineControl, stopAdrenalineControl },
  { 4, "X-Ray Vision", useXRayVision, stopXRayVision },
  { 4, "Telekinesis", NULL, NULL },
  { 4, "Invisibility", NULL, NULL },
  { 4, "Charm", NULL, NULL },
  { 5, "Teleport", useTeleport, NULL },
};


const char *
getMutantPowerName (shMutantPower id)
{
    return MutantPowers[id].mName;
}


/* returns: elapsed time */
int
shHero::useMutantPower ()
{
    shMenu menu ("Use which mutant power?", 0);
    int i;
    int n = 0;
    char buf[50];

    menu.addHeader ("       Power              Level Chance");
    for (i = kNoMutantPower; i < kMaxMutantPower; i++) {
        if (mMutantPowers[i]) {
            int l;
            int chance = 
                (getSkillModifier (kMutantPower, (shMutantPower) i) + mCLevel
                 + mPsiModifier + 15 - 2 * MutantPowers[i].mLevel) * 5;
            if (MutantPowers[i].mLevel * 2 > mCLevel + 1) chance -= 20;
            if (Psion == mProfession) chance += mCLevel * 5;
            if (chance > 100) chance = 100;
            if (chance < 0) chance = 0;

            l = snprintf (buf, 50, "%-19s  %d    ",
                          MutantPowers[i].mName, MutantPowers[i].mLevel);
            if (mMutantPowers[i] > 1) {
                /* persistant power that's already toggled */
                snprintf (&buf[l], 10, "(on)");
            } else {
                snprintf (&buf[l], 10, "%3d%%", chance);
            }
            menu.addItem ('a' + n, buf, (void *) i, 1);
            ++n;
        }
    }
    if (0 == n) {
        I->p ("You don't have any mutant powers.");
        return 0;
    }
    i = 0;
    menu.getResult ((const void **) &i, NULL);
    if (i) {
        int score;

        if (mMutantPowers[i] > 1) { /* deactivating a power, always succeed */
            mChaDrain += MutantPowers[i].mLevel;
            mMutantPowers[i] = 1;
            return MutantPowers[i].mOffFunc (this);
        }

        if (mAbil.mCha <= MutantPowers[i].mLevel) {
            I->p ("You are too weak to use that power.");
            return 200;
        }

        score = getSkillModifier (kMutantPower, (shMutantPower) i) 
            + mCLevel + mPsiModifier + RNG (1, 20);
        if (Psion == mProfession) score += mCLevel;
        if (MutantPowers[i].mLevel * 2 <= mCLevel + 1) score += 4;
        I->debug ("mutant power attempt: rolled %d + %d = %d.",
                  score - getSkillModifier (kMutantPower, (shMutantPower) i) - 
                  mCLevel,
                  getSkillModifier (kMutantPower, (shMutantPower) i) + mCLevel, 
                  score);

        mAbil.mCha -= MutantPowers[i].mLevel;

        if (score >= 10 + 2 * MutantPowers[i].mLevel) {
            exerciseSkill (kMutantPower, 1, (shMutantPower) i);

            if (MutantPowers[i].isPersistant ()) {    
                mMutantPowers[i] = 2;
            } else {
                mChaDrain += MutantPowers[i].mLevel;
            }
            return MutantPowers[i].mFunc (this);
        } else {
            mChaDrain += MutantPowers[i].mLevel;
            I->p ("You fail to use your power successfully.");
            return HALFTURN;
        }
    }
    return 0;
}

/* certain distractions (such as getting stunned) may cause your concentration
 */

void
shCreature::checkConcentration ()
{
    int i;
    int failed = 0;
    int sk = Hero.getSkillModifier (kConcentration);

    if (!isHero ()) {
        return;
    }

    sk += RNG (1, 20);
    
    for (i = kNoMutantPower; i < kMaxMutantPower; i++) {
        if (mMutantPowers[i] > 1 && sk < 15) {
            if (!failed && isHero ()) {
                I->p ("You lose your concentration.");
            }
            failed = 1;
            mChaDrain += MutantPowers[i].mLevel;
            mMutantPowers[i] = 1;
            MutantPowers[i].mOffFunc ( (shHero *) this);
        }
    }
}



shMutantPower
shHero::getMutantPower (shMutantPower power, int silent)
{
    if (kNoMutantPower == power) {
        int attempts = 10;
        while (attempts--) {
            power = (shMutantPower) RNG (kNoMutantPower + 1, 
                                         kMaxMutantPower - 1);
            if (!mMutantPowers[power] && MutantPowers[power].mFunc) {
                mMutantPowers[power] = 1;
                if (!silent) {
                    I->p ("Your brain is warping!");
                    I->p ("You gain the \"%s\" mutant power!", 
                          MutantPowers[power].mName);
                }
                return power;
            }
        }
    } else {
        mMutantPowers[power] = 1;
        if (!silent) {
            I->p ("Your brain is warping!");
            I->p ("You gain the \"%s\" mutant power!", 
                  MutantPowers[power].mName);
        }
        return power;
    }
    return kNoMutantPower;
}



int
shMonster::getMutantLevel ()
{
    return mIlk->mMutantLevel;
}

/* returns elapsed time, 0 if no power used, -1 if creature dies */

int
shMonster::useMutantPower ()
{
    shMutantPower power;
    const char *who;
    int attempts;
    int x0, y0, x, y, res;

    if (mSpellTimer > Clock) {
        return 0;
    }

    who = the ();
    
    for (attempts = 20; attempts; --attempts) {
        power = (shMutantPower) RNG (kMaxMutantPower);
        if (!mIlk->mMutantPowers[power]) continue;

        switch (power) {
        case kHypnosis:
            if (!areAdjacent (this, &Hero) || Hero.isAsleep () || 
                Hero.isParalyzed () || Hero.isBlind ()) 
            {
                continue;
            }
            if (Hero.canSee (this)) {
                I->p ("%s gazes into your eyes.", who);
            }
            mSpellTimer = Clock + 10000;
            return hypnosis (vectorDirection (mX, mY, Hero.mX, Hero.mY));
        case kOpticBlast:
            if (!canSee (&Hero))
                continue;
            {
                shDirection dir = linedUpDirection (this, &Hero);
                if (kNoDirection == dir)
                    continue;
                if (Hero.canSee (this)) {
                    I->p ("%s shoots a laser beam out of %s!", 
                          who, her ("eyes"));
                } else {
                    I->p ("Someone shoots a laser beam out of %s!", 
                          her ("eyes")); 
                    Level->feelSq (mX, mY);
                }
                return opticBlast (dir);
            } 
        case kMentalBlast:
            if (!canSee (&Hero) && (!hasTelepathy () ||
                                    distance (&Hero, mX, mY) >= 40))
            {
                continue;
            }
            if (Hero.canSee (this) && numHands ()) {
                I->p ("%s concentrates.", who);
            }
            mSpellTimer = Clock + 10000;
            return mentalBlast (Hero.mX, Hero.mY);
        case kRegeneration:
            if (mHP >= mMaxHP) 
                continue;
            if (Hero.canSee (this)) {
                if (numHands ())
                    I->p ("%s concentrates.", who);
                I->p ("%s looks better.", who);
            }
            mSpellTimer = Clock + 1000;
            return regeneration ();
        case kTeleport:
            if ((mHP < mMaxHP / 4 || isFleeing ()) && canSee (&Hero)) {
                /* escape to safety */
                x = -1, y = -1;
                /* make double teleport less likely */
                mSpellTimer = Clock + RNG (10000); 
            } else if (hasTelepathy () && mHP == mMaxHP && !RNG (10)) {
                /* teleport in to make trouble */
                x = Hero.mX;
                y = Hero.mY;
                if (mLevel->findNearbyUnoccupiedSquare (&x, &y)) {
                    return 0;
                }
                mSpellTimer = Clock + 1000;
            } else {
                continue;
            }
            res = transport (x, y, 100);
            return (-1 == res) ? -1 : FULLTURN;
        case kTerrify:
            if (Hero.isFrightened () || !canSee (&Hero) || 
                distance (&Hero, mX, mY) >= 40)
            {
                continue;
            }
            if (Hero.canSee (this) && numHands ()) {
                I->p ("%s concentrates.", who);
            }
            if (!Hero.hasBrainShield () && 
                !Hero.isConfused () && !Hero.isStunned () &&
                !willSave (getPsionicDC (3))) 
            {
                I->p ("You suddenly feel very afraid!");
                Hero.makeFrightened (1000 * NDX (2, 10));
            } else {
                I->p ("You tremble for a moment.");
            }
            mSpellTimer = Clock + 10000;
            return FULLTURN;
        case kDarkness:
            if (RNG (4)) /* a rarely used power */
                continue;
            if (!canSee (&Hero)) {
                continue;
            }
            if (mLevel->isLit (mX, mY, Hero.mX, Hero.mY)) {
                x0 = mX;
                y0 = mY;
                if (Hero.canSee (this))
                    I->p ("%s is surrounded by darkness!", who);
            } else if (mLevel->isLit (Hero.mX, Hero.mY, Hero.mX, Hero.mY)) {
                x0 = Hero.mX;
                y0 = Hero.mY;
                I->p ("You are surrounded by darkness!");
            } else {
                continue;
            }
            for (y = y0 - 5; y <= y0 + 5; y++) {
                for (x = x0 - 5; x <= x0 + 5; x++) {
                    if (distance (x, y, x0, y0) <= 25 && 
                        mLevel->existsLOS (x, y, x0, y0))
                    {
                        mLevel->setLit (x, y, -1, -1, -1, -1);
                    }
                }
            }
            if (Hero.canSee (this)) {
                I->drawScreen ();

            }
            mSpellTimer = Clock + 2000;
            return FULLTURN;
        case kCeaseAndDesist:
            if (!areAdjacent (this, &Hero) || Hero.isParalyzed () || Hero.isAsleep ()) 
                continue;
            I->p ("%s reads a cease and desist letter to you!", who);
            if (Hero.hasBrainShield () || willSave (getPsionicDC (4))) {
                I->p("But you ignore it.");
            }  else {
                Hero.makeParalyzed (1000 * NDX (2, 6));
            }
            mSpellTimer = Clock + 2000;
            return FULLTURN;
        case kSeizeEvidence:
            if (!areAdjacent (this, &Hero) || isFleeing ())
                continue;
            if ((Hero.isParalyzed () || Hero.isAsleep ()) && RNG (4)) 
                /* hard for lawyer to seize your stuff unless you're helpless */
                continue;

            {
                I->p ("%s rifles through your pack!", who);
            
                shObjectVector v;
                selectObjectsByFunction (&v, Hero.mInventory, &shObject::isCracked);
                if (v.count ()) {
                    int i = selectObjects (&v, Hero.mInventory, Computer);
                    I->p ("%s seizes your pirated software%s",
                          who,
                          i > 1 ? " and your computers!" : 
                          1 == i ? " and your computer!" : "!");
                    for (i = 0; i < v.count (); i++) {
                        shObject *obj = v.get (i);
                        Hero.removeObjectFromInventory (obj);
                        addObjectToInventory (obj);
                    }
                    makeFleeing (RNG (50000));
                }
            }
            mSpellTimer = Clock + 1000;
            return LONGTURN;
        case kSueForDamages:
            if (!areAdjacent (this, &Hero) || isFleeing ())
                continue;
            
            I->p ("%s sues you for damages!", who);
            x = Hero.loseMoney (NDX (mCLevel, 100));
            if (x) {
                gainMoney (x);
            } else {
                I->p ("But you're broke!");
                makeFleeing (RNG (50000));
            }
            mSpellTimer = Clock + 1000;
            return FULLTURN;
        case kSummonWitness:
            if (!areAdjacent (this, &Hero) || isFleeing ())
                continue;
            {
                shMonsterIlk *ilk;
                shMonster *mon;

                do {
                    ilk = pickAMonsterIlk(RNG((Level->mDLevel+mCLevel+1)/2)); 
                } while (!ilk);
                mon = new shMonster (ilk);
                x = Hero.mX;
                y = Hero.mY;
                if (Level->findNearbyUnoccupiedSquare (&x, &y)) {
                    /* nowhere to put it */
                    delete mon;
                    res = transport (-1, -1, 100);
                    return (-1 == res) ? -1 : FULLTURN;
                }
                if (Level->putCreature (mon, x, y)) {
                    /* shouldn't happen */
                    return FULLTURN;
                }
                mon->mDisposition = kHostile;
                I->p ("%s summons a hostile witness!", who);
                mSpellTimer = Clock + 1000;
                return FULLTURN;
            }

        default:
            break;
        }
    }
    return 0;
}
