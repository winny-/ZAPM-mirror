#include "Global.h"
#include "Util.h"
#include "Creature.h"
#include "Object.h"
#include "Attack.h"
#include "Hero.h"


void
shAbilities::setByIndex (int idx, int val)
{
    switch (idx) {
    case kStr: mStr = val; break;
    case kCon: mCon = val; break;
    case kAgi: mAgi = val; break;
    case kDex: mDex = val; break;
    case kInt: mInt = val; break;
    case kWis: mWis = val; break;
    case kCha: mCha = val; break;
    default: abort ();
    }
}

void
shAbilities::changeByIndex (int idx, int delta)
{
    setByIndex (idx, getByIndex (idx) + delta);
}


//constructor:
shCreature::shCreature ()
    : mTimeOuts (), mSkills ()
{
    int i;

    mX = -10;
    mY = -10;
    mType = kHumanoid;
    mProfession = NULL;
    mName[0] = 0;
    mAP = -1000;
    mCLevel = 0;
    mAC = 0;
    mConcealment = 0;
    mBAB = 0;
    mChaDrain = 0;
    mLastRegen = MAXTIME;
    mLastLevel = NULL;
    mReflexSaveBonus = 0;
    mInateIntrinsics = 0;
    mLevel = NULL;
    for (i = 0; i < kMaxEnergyType; i++) {
        mInateResistances[i] = 0;
        mResistances[i] = 0;
    }
    for (i = 0; i < kMaxMutantPower; i++) {
        mMutantPowers[i] = 0;
    }
    for (i = 0; i < shImplantIlk::kMaxSite; i++) {
        mImplants[i] = NULL;
    }
    for (i = 0; i < TRACKLEN; i++) {
        mTrack[i].mX = -1;
    }
    mConditions = 0;
    mRad = 0;

    mInventory = new shVector <shObject *> (8);
    mWeight = 0;
    mPsiModifier = 0;

    mSpeed = 100;
    mSpeedBoost = 0;
    mBodyArmor = NULL;
    mJumpsuit = NULL;
    mHelmet = NULL;
    mBoots = NULL;
    mBelt = NULL;
    mGoggles = NULL;
    mWeapon = NULL;
    mState = kWaiting;
    mHidden = 0;
    mMimic = kNothing;
    mSpotAttempted = 0;
    mTrapped = 0;
    mLastMoveTime = 0;
}


shCreature::~shCreature ()
{
    delete mInventory;
};


int
shCreature::isA (char *ilk)
{
    return (0 == strcmp (ilk, mIlk->mName));
}


char *
shCreature::herself ()
{
    return (char *) (hasMind () ? "himself" : "itself");
}


shCreature::TimeOut *
shCreature::getTimeOut (shTimeOutType key)
{
    int i;

    for (i = 0; i < mTimeOuts.count (); i++) {
        if (key == mTimeOuts.get (i) -> mKey) {
            return mTimeOuts.get (i);
        }
    }
    return NULL;
}


shTime
shCreature::setTimeOut (shTimeOutType key, shTime howlong, int additive)
{
    TimeOut *t = getTimeOut (key);

    if (NULL == t) {
        mTimeOuts.add (new TimeOut (key, howlong + Clock));
        return howlong + Clock;
    } else {
        if (additive) {
            t->mWhen += howlong;
        } else if (0 == howlong) {
            t->mWhen = 0;
        } else {
            t->mWhen = maxi (howlong + Clock, t->mWhen);
        }
        return t->mWhen;
    }
}


/* returns non-zero iff the creature dies */
int 
shCreature::checkTimeOuts ()
{
    TimeOut *t;
    int i;

    for (i = mTimeOuts.count () - 1; i >= 0; --i) {
        t = mTimeOuts.get (i);
        if (t->mWhen <= Clock) {
            if (XRAYVISION == t->mKey) {
                mInateIntrinsics &= ~kXRayVision;
                computeIntrinsics ();
                if (isHero () && 
                    hasPerilSensing () && 
                    mGoggles->isToggled ()) 
                {
                    I->p ("You can no longer see through "
                          "your darkened goggles.");
                }
                interrupt ();
            } else if (TELEPATHY == t->mKey) {
                mInateIntrinsics &= ~kTelepathy;
                computeIntrinsics ();
                interrupt ();
            } else if (STUNNED == t->mKey) {
                mConditions &= ~kStunned;
                if (isHero ()) {
                    I->drawSideWin ();
                    I->p ("You feel steadier.");
                }
                interrupt ();
            } else if (CONFUSED == t->mKey) {
                mConditions &= ~kConfused;
                if (isHero ()) {
                    I->drawSideWin ();
                    I->p ("You feel less confused.");
                }
                interrupt ();
            } else if (HOSED == t->mKey) {
                mConditions &= ~kHosed;
                computeIntrinsics ();
                if (isHero ()) {
                    I->drawSideWin ();
                    I->p ("You are no longer hosed.");
                }
                interrupt ();
            } else if (PARALYZED == t->mKey) {
                mConditions &= ~kParalyzed;
                if (isHero ()) {
                    I->drawSideWin ();
                    I->p ("You can move again.");
                }
                interrupt ();
            } else if (FRIGHTENED == t->mKey) {
                mConditions &= ~kFrightened;
                if (isHero ()) {
                    I->drawSideWin ();
                    I->p ("You regain your courage.");
                }
                interrupt ();
            } else if (BLINDED == t->mKey) {
                mInateIntrinsics &= ~kBlind;
                computeIntrinsics ();
                if (isHero () && !isBlind ()) {
                    I->p ("You can see again.");
                }
                interrupt ();
            } else if (VIOLATED == t->mKey) {
                mConditions &= ~kViolated;
                if (isHero ()) {
                    I->p ("You don't feel so sore anymore.");
                }
                computeIntrinsics ();
                interrupt ();
            } else if (SICKENED == t->mKey) {
                mConditions &= ~kSickened;
                if (isHero ()) {
                    I->p ("You feel less sick.");
                }
                interrupt ();
            } else if (SPEEDY == t->mKey) {
                mConditions &= ~kSpeedy;
                if (isHero ()) {
                    I->p ("You slow down.");
                }
                computeIntrinsics ();
                interrupt ();
            } else if (ASLEEP == t->mKey) {
                if (isAsleep ()) {
                    mConditions &= ~kAsleep;
                    if (isHero ()) {
                        I->drawSideWin ();
                        I->p ("You wake up.");
                    }
                }
                interrupt ();
            } else if (TRAPPED == t->mKey) {
                if (isTrapped ()) {
                    char buf[60];
                    the (buf, 60);
                    shFeature *f = mLevel->getFeature (mX, mY);
                    if (f && shFeature::kAcidPit == f->mType) {
                        if (isHero ()) {
                            I->p ("You are being dissolved in acid!");
                        }
                        if (sufferDamage (&AcidPitTrapDamage)) {
                            if (!isHero () && Hero.canSee (this)) {
                                I->p ("%s is %s!", buf, deathVerb ());
                            }
                            die (kMisc, "Dissolved in acid");
                            return -1;
                        }
                    }
                    t->mWhen += 1000; 
                    ++i;
                    goto dontremove;
                }
            }
            mTimeOuts.remove (t);
            delete t;
        dontremove: ;
        }
    }
    return 0;
}


void
shCreature::makeAsleep (int howlong)
{
    mConditions |= kAsleep;
    computeIntrinsics ();
    setTimeOut (ASLEEP, howlong, 0);
}


void
shCreature::wakeUp ()
{
    mConditions &= ~kAsleep;
    computeIntrinsics ();
    if (isHero () && mHP > 0) {
        I->drawSideWin ();
        I->p ("You wake up.");
    }
    clearTimeOut (ASLEEP);
}


void
shCreature::makeBlinded (int howlong)
{
    mInateIntrinsics |= kBlind;
    computeIntrinsics ();
    setTimeOut (BLINDED, howlong);
}


void
shCreature::makeConfused (int howlong)
{
    mConditions |= kConfused;
    checkConcentration ();
    setTimeOut (CONFUSED, howlong);
}


void
shCreature::resetConfused ()
{
    mConditions &= ~kConfused;
    clearTimeOut (CONFUSED);
}


void
shCreature::makeFrightened (int howlong)
{
    mConditions |= kFrightened;
    computeIntrinsics ();
    setTimeOut (FRIGHTENED, howlong, 0);
}


void
shCreature::resetFrightened ()
{
    mConditions &= ~kFrightened;
    clearTimeOut (FRIGHTENED);
}


void
shCreature::makeHosed (int howlong)
{
    if (isHosed ()) {
        howlong /= 2;
    }
    if (isHero ()) {
        I->p ("Your connection is hosed!");
    }
    mConditions |= kHosed;
    setTimeOut (HOSED, howlong);
}


void
shCreature::resetHosed ()
{
    mConditions &= ~kHosed;
    clearTimeOut (HOSED);
}


void
shCreature::makeParalyzed (int howlong)
{
    mConditions |= kParalyzed;
    setTimeOut (PARALYZED, howlong, isHero () ? 0 : 1);
}


void
shCreature::makeSickened (int howlong)
{
    if (isSickened ()) {
        I->p ("You feel worse.");
        howlong /= 2;
    } else {
        I->p ("You feel sick.");
    }
    mConditions |= kSickened;
    setTimeOut (SICKENED, howlong);
}


void
shCreature::resetSickened ()
{
    mConditions &= ~kSickened;
    clearTimeOut (SICKENED);
}


void
shCreature::makeSpeedy (int howlong)
{
    mConditions |= kSpeedy;
    setTimeOut (SPEEDY, howlong);
}


void
shCreature::makeStunned (int howlong)
{
    mConditions |= kStunned;

    checkConcentration ();

    if (mWeapon && RNG (2)) {
        char buf[80];
        shObject *obj = mWeapon;
        unwield (obj);
        removeObjectFromInventory (obj);
        if (isHero ()) {
            I->drawSideWin ();
            obj->your (buf, 80);
            I->p ("You drop %s.", buf);
        } else if (Hero.canSee (this)) {
            char buf2[80];
            the (buf, 80);
            obj->her (buf2, 80, this);
            I->p ("%s drops %s.", buf, buf2);
            if (obj->isUnpaid () && !Level->isInShop (mX, mY)) {
                obj->resetUnpaid ();
            }
        }
        Level->putObject (obj, mX, mY);
    }
    setTimeOut (STUNNED, howlong);
}


void
shCreature::resetStunned ()
{
    mConditions &= ~kStunned;
    clearTimeOut (STUNNED);
}


void
shCreature::makeViolated (int howlong)
{
    if (isHero ()) {
        I->p ("You feel violated!");
    }
    mConditions |= kViolated;
    computeIntrinsics ();
    setTimeOut (VIOLATED, howlong, 0);
}


void
shCreature::resetViolated ()
{
    mConditions &= ~kViolated;
    clearTimeOut (VIOLATED);
}


void
shCreature::sterilize ()
{
    mIntrinsics &= ~kMultiplier;
}


void
shObject::applyConferredResistances (shCreature *target)
{
    if (!isInUse ()) return;

    if (isA (kArmor)) {
        shArmorIlk *ilk = (shArmorIlk *) mIlk;
        int i;
        for (i = 0 ; i < kMaxEnergyType; i++) {
            target->mResistances[i] += ilk->mResistances[i];
        }
        if (isA ("chameleon suit")) {
            target->mConcealment += 20;
        }
        target->mPsiModifier += getPsiModifier ();
    } else if (isA (kImplant)) {
        target->mPsiModifier += getPsiModifier ();
        if (isA ("cerebral coprocessor")) {
            target->mExtraAbil.mInt += mEnhancement;
        } else if (isA ("reflex coordinator")) {
            target->mExtraAbil.mAgi += mEnhancement;
        } else if (isA ("adrenaline generator")) {
            target->mExtraAbil.mStr += mEnhancement;
        } else if (isA ("poison resistor")) {
            target->mResistances[kPoisonous] += 10;
        }
    }

}


void
shCreature::levelUp ()
{
    int x = ABILITY_MODIFIER (getCon ()) + RNG (1, mProfession->mHitDieSize);

    mCLevel++;

    mBAB = mProfession->mBAB * mCLevel / 4;
    mReflexSaveBonus = mProfession->mReflexSaveBonus * mCLevel / 4;
    mWillSaveBonus = mProfession->mWillSaveBonus * mCLevel / 4;
    if (x < 1) x = 1;
    x++; /* fudge things slightly in favor of the hero... */
    mMaxHP += x;
    mHP += x;

    if (isHero ()) {
        I->p ("You've obtained level %d!", mCLevel);
        I->pause ();
        I->doMorePrompt ();
        I->drawSideWin ();
        if (0 == mCLevel % 4) {
            Hero.gainAbility ();
        }
        Hero.editSkills (Hero.mProfession->mNumPracticedSkills);
        Hero.computeIntrinsics ();
    }
}


//RETURNS: 1 if attack is deadly (caller responsible for calling die ()); o/w 0
int
shCreature::sufferAbilityDamage (shAbilityIndex idx, int amount, 
                                 int ispermanent /* = 0 */)
{
    int oldscore = mAbil.getByIndex (idx);
    int oldmodifier = ABILITY_MODIFIER (oldscore);
    mAbil.setByIndex (idx, mAbil.getByIndex (idx) - amount);
    if (ispermanent) {
        int newamount = mMaxAbil.getByIndex (idx) - amount;
        if (newamount < 0) {
            newamount = 0;
        }
        mMaxAbil.setByIndex (idx, newamount);
    }

    interrupt ();

    if (kCon == idx) {
        /* constitution damage results in lost HP */
        int hploss = mCLevel * 
            (oldmodifier - ABILITY_MODIFIER (mAbil.getByIndex (idx)));

        mMaxHP -= hploss;
        if (mMaxHP < mCLevel) { 
            hploss -= (mCLevel - mMaxHP);
            mMaxHP = mCLevel;
        }
        mHP -= hploss;
        if (mHP <= 0) {
            mHP = 0;
            if (&Hero == this) {
                I->drawSideWin ();
                I->drawLog ();
            }
            return 1;
        }
    }

    computeIntrinsics ();

    if (mAbil.getByIndex (idx) <= 0) {
        mAbil.setByIndex (idx, 0);
        if (&Hero == this) {
            I->drawSideWin ();
            I->drawLog ();
        }
        return 1;
    }
    return 0;
}


/* if the attack is reflected, prints a message and returns 1,
   o/w returns 0 */
int
shCreature::reflectAttack (shAttack *attack)
{
    if ((kLaser == attack->mDamage[0].mEnergy && hasReflection ()) 
        || (kForce == attack->mDamage[0].mEnergy && 
            mWeapon && mWeapon->isA ("light saber")))
    {
        char who[80];
        char what[80];

        if (isHero ()) {
            strcpy (who, "You");
        } else {
            the (who, 80);
        }
        if (mWeapon && mWeapon->isA ("light saber")) {
            /* this could possibly involve a skill check? 
               Note you can parry even when blind! */
            if (isHero ()) {
                mWeapon->your (what, 80);
                I->p ("You parry the blast with %s.", what);
            } else {
                mWeapon->her (what, 80, this);
                I->p ("%s parries the blast with %s.", who, what);
            }
        } else if (Hero.isBlind ()) { /* silently reflect */
            return 1;
        } else if (mHelmet && mHelmet->isA ("brain shield")) {
            if (isHero ()) {
                I->p ("The blast is deflected by your shiny hat.");
            } else {
                I->p ("The blast is deflected by %s's shiny hat.", who);
            }
        } else if (mBodyArmor && mBodyArmor->isA ("suit of reflec armor")) {
            if (isHero ()) {
                I->p ("The blast is deflected by your shiny armor.");
            } else {
                I->p ("The blast is deflected by %s's shiny armor.", who);
            }
        } else {
            if (isHero ()) {
                I->p ("The blast is deflected by your shiny skin.");
            } else {
                I->p ("The blast is deflected by %s's shiny skin.", who);
            }
        }
        return 1;
    } else {
        return 0;
    }
}


/* works out damage done by an attack that has hit.  applies resistances, 
   special defences, special damage (e.g. stunning), and subtracts HP.
   returns: 1 if attack kills us; o/w 0
*/
int
shCreature::sufferDamage (shAttack *attack,
                          shCreature *attacker, /* = NULL */
                          int bonus,            /* = 0 */
                          int multiplier,       /* = 1 */
                          int divider)          /* = 1 */ 
{
    int i;
    int damage;
    int totaldamage = 0;
    int shieldworked = 0;
    char buf[64];
    int flatfootedAC = getAC (1);
    int oldhp = mHP;

    interrupt ();

    if (isHero ()) {
        strcpy (buf, "you");
    } else {
        the (buf, 64);
    }

    if (attacker && attacker->isHero () && mHidden) {
        mHidden = 0;
    }

    /* iterate through two possible types of damage */

    for (i = 0; i < 2; i++) { 
        shEnergyType energy = (shEnergyType) attack->mDamage[i].mEnergy;
        int resist = getResistance (energy);
        int doresistmsg = 1;

        /* compute base damage */

        damage = NDX (multiplier * attack->mDamage[i].mNumDice,
                      attack->mDamage[i].mDieSides);
        damage += bonus;
        damage /= divider;
        if (damage < 1) damage = 1;
        bonus = 0;
        multiplier = 1;

        /* possibly absorb some of the damage with a shield*/

        if (hasShield () && 
            (attack->isMissileAttack () || attack->isAimedAttack ()) &&
            shAttack::kHealingRay != attack->mType &&
            shAttack::kRestorationRay != attack->mType)
        {
            int absorbed = loseEnergy (damage);
            if (absorbed) {
                shieldworked = 1;
            }
            damage -= absorbed;
            if (0 == damage) {
                continue;
            }
        }

        /* possibly damage some equipment */

        if (kNoEnergy == energy) {
            break;
        } else if (shAttack::kOther == attack->mEffect) {
            /* special attack methods won't affect your equipment */
        } else if (kCorrosive == energy) {
            if (shAttack::kSlime == attack->mType) {
                if (Hero.canSee (this)) {
                    I->p ("%s %s slimed!", buf, isHero () ? "get" : "gets");
                }
            }
            damageEquipment (attack, energy);
        } else if (kElectrical == energy) {
            damageEquipment (attack, energy);
        } else if (kMagnetic == energy) {
            damageEquipment (attack, energy);
        } else if (kBurning == energy) {
            damageEquipment (attack, energy);
        } else if (kBugging == energy) {
            damageEquipment (attack, energy);
        }

        /* reduce damage according to resistances */

        damage = damage - resist;
        if (damage < 0) damage = 0;

        /* apply energy-specific special effects */
        
        if (kPoisonous == energy) {
            if (Hero.canSee (this) && 
                !(shAttack::kPoisonRay == attack->mType ||
                  shAttack::kNoAttack == attack->mType))
            {
                I->p ("The %s was poisonous!", attack->noun ());
            }
            if (0 == damage) {
                if (Hero.canSee (this)) {
                    I->p ("But the poison doesn't affect %s.", buf);
                }
                doresistmsg = 0;
            } else if (resist) {
                if (Hero.canSee (this)) {
                    I->p ("%s %s some of the poison.", 
                          buf, isHero () ? "resist" : "resists");
                }
                doresistmsg = 0;
            }
            if (sufferAbilityDamage (kStr, damage, 0)) { 
                /* dead, dead, dead! */
                return 1;
            }
            if (isHero ()) { 
                continue;
            }
            /* for balance, monsters take HP damage in addition to str drain */
            damage *= 2;
        } else if (kBlinding == energy) {
            if (!isBlind ()) {
                if (isHero ()) {
                    I->p ("You are blinded by the beam.");
                }
                makeBlinded (1000 * damage);
            }
            continue;
        } else if (kHosing == energy) {
            makeHosed (1000 * NDX (4, 10));
        } else if (kSickening == energy) {
            makeSickened (1000 * NDX (4, 10));
        } else if (kStunning == energy) {
            makeStunned (1000 * damage);
            continue;
        } else if (kConfusing == energy) {
            makeConfused (1000 * damage);
            continue;
        } else if (kViolating == energy) {
            makeViolated (1000 * damage);
            continue;
        } else if (kParalyzing == energy) {
            if (isHero ()) {
                makeParalyzed (1000 * damage);
            } else {
                makeParalyzed (10000 * damage);
            }
            continue;
        } else if (kDisintegrating == energy) {
            if (isHero ()) {
                shObject *obj;
                if (NULL != (obj = mBodyArmor) ||
                    NULL != (obj = mJumpsuit))
                {
                    obj->your (buf, 80);
                    I->p ("%s is annihilated!", buf);
                    removeObjectFromInventory (obj);
                    delete obj;
                    continue;
                } 
                return 1;
            } else if (damage) {
                return 1;
            } else if (Hero.canSee (this)) {
                I->p ("%s resists!", buf);
                doresistmsg = 0;
            }
        } else if (kMagnetic == energy) {
            if (0 == damage && !isHero () && Hero.canSee (this)) {
                I->p ("%s is unaffected.", buf);
                doresistmsg = 0;
            }
        } else if (kRadiological == energy) {
            damage *= RNG (1, 10);
            if (isHero ()) {
                mRad += damage;
            } else if (0 == damage) {
                if (Hero.canSee (this)) {
                    I->p ("%s is unaffected.", buf);
                    doresistmsg = 0;
                }
            } else { /* treat like poison */
                if (sufferAbilityDamage (kCon, RNG (6), 0)) { 
                    /* dead, dead, dead! */
                    return 1;
                } else if (Hero.canSee (this)) {
                    I->p ("%s seems weakened.", buf);
                }
                sterilize ();
            }
            continue;
        } else if (kSuing == energy) {
            if (isHero ()) {
                int amount = loseMoney (damage);
                if (amount) {
                    attacker->gainMoney (damage);
                } else {
                    /* I->p ("But you refuse to pay!"); */
                }
            }
            continue;
        } else if (kSeizing == energy) {
            shObjectVector v;
            if (isHero ()) {
                selectObjectsByFunction (&v, mInventory, &shObject::isCracked);
                if (v.count ()) {
                    int i = selectObjects (&v, mInventory, Computer);
                    I->p ("The lawyer seizes your pirated software%s",
                          i > 1 ? " and your computers!" : 
                          1 == i ? " and your computer!" : "!");

                    for (i = 0; i < v.count (); i++) {
                        shObject *obj = v.get (i);
                        removeObjectFromInventory (obj);
                        attacker -> addObjectToInventory (obj);
                    }
                }
            }
            continue;
        } else if (kCreditDraining == energy) {
            int amount = loseMoney (damage);
            if (amount) {
                I->p ("Your wallet feels lighter.");
                attacker->gainMoney (amount);
                attacker->mMaxHP += amount / 10;
                attacker->mHP += amount / 5;
                if (attacker->mHP > attacker->mMaxHP) {
                    attacker->mHP = attacker->mMaxHP;
                }
/*   BUG: there is a remote possibility that the attacker could die as a 
           result of this transport; if this happens we surely crash :-( */
                attacker->transport (-1, -1, 100);
                attacker->mHidden = 0;
            }
            continue;
        } else if (kHealing == energy) {
            if (isHero () && mHP < mMaxHP) {
                I->p ("Your wounds are rapidly healing!");
            }
            mHP += NDX (4, 8);
            if (mHP > mMaxHP) {
                mHP = mMaxHP;
            }
            continue;
        } else if (kRestoring == energy) {
            restoration ();
            continue;
        }

        if (resist && doresistmsg && 
            !isHero () && Hero.canSee (this)) 
        {
            if (0 == damage) {
                I->p ("%s is unaffected.", buf);
            } else {
                I->p ("%s resists.", buf);
            }
        }

        totaldamage += damage;
    }

    /* print a mesage about your shield */
    
    if (shieldworked) {
        int fizzle =  (0 == countEnergy ());
        if (isHero ()) {
            I->p ("Your force shield absorbs %s damage%s", 
                  totaldamage ? "some of the" : "the",
                  fizzle ? " and fizzles out." : ".");
        } else if (Hero.canSee (this)) {
            I->p ("%s's force shield absorbs %s damage%s", buf,
                  totaldamage ? "some of the" : "the",
                  fizzle ? " and fizzles out." : ".");
        }
    }

    /* wearing lots of armor results in damage reduction */

    if (flatfootedAC > 20 && totaldamage) {  
        totaldamage -= (flatfootedAC - 19) / 2;
        if (totaldamage < 1) totaldamage = 1;
    }

    /* TODO: system shock check for massive damage */

    /* reduce hit points */

    mHP -= totaldamage;
    if (mHP < 0) {
        mHP = 0;
    }
    I->diag ("dealt %d damage to %p, now has %d / %d HP", 
             totaldamage, this, mHP, mMaxHP);

    if (isHero ()) {
        if (isAsleep () && sportingD20 () > 16) {
            wakeUp ();
        }
        if (hasHealthMonitoring () &&
            mHP < hpWarningThreshold () &&
            mHP > 0 && 
            oldhp >= hpWarningThreshold ())
        {
            I->p ("You are about to die!");
        }
        I->drawSideWin ();
        I->drawLog ();
    } else {
        if (isAsleep () && RNG (1, 20) > 16) {
            wakeUp ();
        }
    }

    /* die if HP <= 0 */

    if (mHP <= 0) {
        if (isA ("borg") && attacker && attacker->isHero ()) {
            /* the borg adapt to defend against the attacks you use 
               against them!  I am so evil!! */

            ++mIlk->mNaturalArmorBonus;
#if 0
            for (i = 0; i < 2; i++) {
                shEnergyType energy = (shEnergyType) attack->mDamage[i].mEnergy;
                if (kNoEnergy = energy) {
                    break;
                }
                if (getResistance (energy) < 10) {
                    mIlk->mInateResistances[energy] += 10;
                    break;
                }
            }
#endif
        }
        return 1;
    }
    return 0;
}


char *
shCreature::deathVerb (int presenttense /* = 0*/)
{
    if (presenttense) {
        if (isAlive ()) return "kill"; 
        if (isRobot ()) return "disable";
        if (isProgram ()) return "derez";
        return "destroy";
    } else {
        if (isAlive ()) return "killed"; 
        if (isRobot ()) return "disabled";
        if (isProgram ()) return "derezzed";
        return "destroyed";
    }
}


int
shCreature::die (shCauseOfDeath how, char *killer)
{
    int i;
    char buf[64];

    the (buf, 64);
    I->debug ("%s (%p) is dead", buf, this);
    
    if (isPet ()) {
        Hero.mPets.remove (this);
        if (!Hero.canSee (this)) {
            I->p ("You have a sad feeling for a moment, and then it passes.");
        }
        if (RNG (3)) {
            Level->putObject (createObject ("restraining bolt", 1), mX, mY);
        }
    }

    if (hasAcidBlood () && kSlain == how) {
        int x, y;
        shCreature *c;
        char buf[60];

        for (x = mX - 1; x <= mX + 1; x++) {
            for (y = mY - 1; y <= mY + 1; y++) {
                c = mLevel->getCreature (x, y);
                if (!c || c == this) continue;
                if (c->isHero ()) {
                    I->p ("You are splashed by acid!");
                } else if (Hero.canSee (this)) {
                    c->the (buf, 50);
                    I->p ("%s is splashed by acid!", buf);
                }
                if (c->sufferDamage (&AcidBloodAttack)) {
                    c->die (kSlain, "an acid splash");
                    if (!c->isHero ()) {
                        I->p ("%s is %s!", buf, c->deathVerb ());
                    }
                }
            }
        }
    }

    for (i = 0; i < mInventory->count (); i++) {
        shObject *obj = mInventory->get (i);
        obj->resetWorn ();
        obj->resetWielded ();
        if (obj->isUnpaid () && !Level->isInShop (mX, mY)) {
            obj->resetUnpaid ();
        }
        if (kAnnihilated == how && !obj->isIndestructible ()) {
            delete obj;
        } else {
            Level->putObject (obj, mX, mY);
        }
    }
    if (kAnnihilated == how) {
        /* no corpse */
    } else if (isA ("killer rabbit")) {
        Level->putObject (createObject ("rabbit's foot", 1), mX, mY);
    } else if (isRobot ()) {
        shObject *wreck = createCorpse (this);
        Level->putObject (wreck, mX, mY);
    }
    I->dirty ();

    mState = kDead;
    if (mLevel) {
        mLevel->removeCreature (this);
    }

    return 1;
}


/* check to see if the creature is getting irradiated
   returns: number of rads taken
   modifies: nothing
*/
int
shCreature::checkRadiation ()
{
    int i;
    int res = 0;
    shObjectVector *v = Level->getObjects (mX, mY);
    for (i = 0; i < mInventory->count (); i++) {
        shObject *obj = mInventory->get (i);
        if (obj->isRadioactive ()) {
            res += 2;
        }
    }
    if (v) {
        for (i = 0; i < v->count (); i++) {
            shObject *obj = v->get (i);
            if (obj->isRadioactive ()) {
                res += 1;
            }
        }
    }

    if (Level->isRadioactive (mX, mY)) {
        res += 2;
    }

    return res;
}


int
shCreature::owns (shObject *obj)
{
    return mInventory->find (obj) >= 0;
}


int
shCreature::addObjectToInventory (shObject *obj, int quiet)
{
    mInventory->add (obj);

    obj->mLocation = shObject::kInventory;
    obj->mOwner = this;
    mWeight += obj->getMass ();
    computeIntrinsics ();
    return 1;
}


void 
shCreature::removeObjectFromInventory (shObject *obj)
{
    mInventory->remove (obj);
    mWeight -= obj->getMass ();
    if (obj->isWorn ()) { 
        doff (obj); 
    }
    obj->resetWorn ();
    obj->resetWielded ();
    obj->resetActive ();
    if (mWeapon == obj) { mWeapon = NULL; }
    computeIntrinsics ();
    computeAC ();
}


shObject *
shCreature::removeOneObjectFromInventory (shObject *obj)
{
    if (1 == obj->mCount) {
        removeObjectFromInventory (obj);
        return obj;
    } else {
        mWeight -= obj->mIlk->mWeight;
        return obj->split (1);
    }
}


void
shCreature::useUpOneObjectFromInventory (shObject *obj)
{
    if (isHero () && obj->isUnpaid ()) {
        Hero.usedUpItem (obj, 1, "use");
    }
    if (1 == obj->mCount) {
        removeObjectFromInventory (obj);
        delete obj;
    } else {
        --obj->mCount;
        mWeight -= obj->mIlk->mWeight;
    }
}


//RETURNS: number of objects used
int
shCreature::useUpSomeObjectsFromInventory (shObject *obj, int cnt)
{    
    if (obj->mCount > cnt) {
        obj->mCount -= cnt;
        mWeight -= cnt * obj->mIlk->mWeight;
        if (isHero () && obj->isUnpaid ()) {
            Hero.usedUpItem (obj, cnt, "use");
        }
        return cnt;
    } else {
        cnt = obj->mCount;
        if (isHero () && obj->isUnpaid ()) {
            Hero.usedUpItem (obj, cnt, "use");
        }
        removeObjectFromInventory (obj);
        return cnt;
    }
}


shObject *
shCreature::removeSomeObjectsFromInventory (shObject *obj, int cnt)
{    
    if (obj->mCount > cnt) {
        obj = obj->split (cnt);
        mWeight -= cnt * obj->mIlk->mWeight;
        return obj;
    } else {
        removeObjectFromInventory (obj);
        return obj;
    }
}


int
shCreature::countEnergy (int *tankamount)
{
    shObjectVector v;
    int res = 0;
    int i;

    selectObjects (&v, mInventory, EnergyCell);
    for (i = 0; i < v.count (); i++) {
        res += v.get (i) -> mCount;
    }
    v.reset ();

    if (tankamount) *tankamount = 0;
    selectObjects (&v, mInventory, PowerPlant);
    for (i = 0; i < v.count (); i++) {
        res += v.get (i) -> mCharges;
        if (tankamount) *tankamount += 100;
    }


    return res;

}

/* expends some energy from the following sources, in order of preference:
   1. power plants
   2. energy cells
   3. unpaid power plants
   4. unpaid cells
   returns: # of energy units spent 
*/

int
shCreature::loseEnergy (int cnt)
{
    shObjectVector v;
    int res = 0;
    int i;
    int pass;

    for (pass = 0; pass < 2; pass++) {
        selectObjects (&v, mInventory, PowerPlant);
        for (i = 0; i < v.count (); i++) {
            shObject *obj = v.get (i);
            int unpaid = isHero () && obj->isUnpaid ();
            if (unpaid) {
                if (0 == pass) {
                    continue;
                } else if (obj->mCharges) {
                    /* easy way to deal with this is to make 'em pay for the 
                       whole damn tank */
                    Hero.usedUpItem (obj, 1, "use");
                }
            }
            
            if (obj->mCharges > cnt) {
                obj->mCharges -= cnt;
                res += cnt;
                return res;
            } else {
                res += obj->mCharges;
                obj->mCharges = 0;
            }
            cnt -= res;
        }
        v.reset ();

        selectObjects (&v, mInventory, EnergyCell);
        for (i = 0; i < v.count (); i++) {
            shObject *obj = v.get (i);
            int unpaid = isHero () && obj->isUnpaid ();
            if (0 == pass && unpaid) {
                continue;
            }
            int tmp = useUpSomeObjectsFromInventory (obj, cnt);
            cnt -= tmp;
            res += tmp;
            if (0 == cnt) {
                return res;
            }
        }
        v.reset ();
    }
    return res;
}


void
shCreature::gainEnergy (int amount)
{
    shObjectVector v;
    int i;

    selectObjects (&v, mInventory, PowerPlant);
    for (i = 0; i < v.count (); i++) {
        shObject *obj = v.get (i);
        
        if (obj->mCharges < 100) {
            obj->mCharges += amount;
            if (obj->mCharges > 100) {
                amount -= (obj->mCharges - 100);
                obj->mCharges = 100;
            } else {
                return;
            }
        }
    }    
}


int
shCreature::countMoney ()
{
    shObjectVector v;
    int res = 0;
    int i;

    selectObjects (&v, mInventory, kMoney);
    for (i = 0; i < v.count (); i++) {
        res += v.get (i) -> mCount;
    }
    return res;
}


int
shCreature::loseMoney (int amount)
{
    shObjectVector v;
    int res = 0;
    int i;

    selectObjects (&v, mInventory, kMoney);
    for (i = 0; i < v.count (); i++) {
        shObject *obj = v.get (i);
        int tmp = useUpSomeObjectsFromInventory (obj, amount);
        amount -= tmp;
        res += tmp;
    }
    return res;
}


int
shCreature::gainMoney (int amount)
{
    shObject *obj = createMoney (amount);
    addObjectToInventory (obj);
    return amount;
}



int
shCreature::wield (shObject *obj, int quiet /* = 0 */ )
{
    if (mWeapon && 0 == unwield (mWeapon)) {
        return 0;
    }
    mWeapon = obj;
    if (0 == quiet && Hero.canSee (this) && mWeapon) {
        char buf[40], buf2[40];
        the (buf, 40);
        mWeapon->setAppearanceKnown ();
        mWeapon->an (buf2, 40);
        I->p ("%s wields %s!", buf, buf2);
    }
    computeIntrinsics ();    
    return 1;
}


int
shCreature::unwield (shObject *obj, int quiet /* = 0 */ )
{
    assert (mWeapon == obj);
    mWeapon = NULL;
    computeIntrinsics ();
    return 1;
}


int
shCreature::don (shObject *obj, int quiet /* = 0 */ )
{
    int before = mIntrinsics;

    if (obj->isA (shBodyArmor) && NULL == mBodyArmor) {
        mBodyArmor = obj;
    } else if (obj->isA (shJumpsuit) && NULL == mJumpsuit) {
        mJumpsuit = obj;
    } else if (obj->isA (shHelmet) && NULL == mHelmet) {
        mHelmet = obj;
    } else if (obj->isA (shGoggles) && NULL == mGoggles) {
        mGoggles = obj;
    } else if (obj->isA (shBelt) && NULL == mBelt) {
        mBelt = obj;
    } else if (obj->isA (kImplant)) {
        shImplantIlk::Site site = ((shImplantIlk *) (obj->mIlk)) -> mSite;
        if (shImplantIlk::kAnyBrain == site) {
            int i;
            for (i = 0; i < shImplantIlk::kLeftEar; i++) {
                if (!mImplants[i]) {
                    site = (shImplantIlk::Site) i;
                    goto foundsite;
                }
            }
            if (isHero ()) {
                I->p ("There's no more room in your brain for this implant!");
            }
            return 0;
        } else if (shImplantIlk::kAnyEar == site) {
            site = shImplantIlk::kLeftEar;
        }

    foundsite:
        if (mImplants[site]) {
            if (isHero ()) {
                I->p ("You already have an implant installed in your %s.",
                      describeImplantSite (site));
            }
            return 0;
        }
        mImplants[site] = obj;
        if (isHero ()) {
            char buf[50];
            obj->the (buf, 50);
            I->p ("You install %s in your %s.",
                  buf, describeImplantSite (site));
            obj->mImplantSite = site;
        }
    } else {
        return 0;
    }
    obj->setWorn ();
    computeIntrinsics ();
    computeAC ();
    if (isHero ()) {
        int wasunknown = !obj->isIlkKnown ();
        //int knowledge = obj->getFlag (shObject::kKnownExact);
        
        if (obj->isA (kArmor) && obj->isEnhanceable ()) {
            obj->setEnhancementKnown ();
        }
        if (obj->isA ("cortex crossover")) {
            I->p ("You flip out!");
            obj->setIlkKnown ();
        } else if (obj->isA ("tissue regenerator")) {
            mLastRegen = Clock;
        } else if (obj->isA ("chameleon suit")) {
            I->p ("You blend in with your surroundings.");
            obj->setIlkKnown ();
        } else if (obj->isA ("shield belt") && !(before & kShielded)) {
            I->p ("You are surrounded by a force field.");
            obj->setIlkKnown ();
        } else if (obj->isA ("jetpack") && !(before & kFlying)) {
            I->p ("You zoom into the air!");
            obj->setIlkKnown ();
            mTrapped = 0;
        } else if (obj->isA ("pair of x-ray goggles") &&
                   !(before & kXRayVision)) 
        {
            I->p ("You can see through things!");
            obj->setIlkKnown ();
        } else if (obj->isA ("cerebral coprocessor") ||
                   obj->isA ("reflex coordinator") ||
                   obj->isA ("adrenaline generator")) 
        {
            if (obj->mEnhancement) {        
                wasunknown = !obj->isEnhancementKnown ();
                obj->setIlkKnown ();
                obj->setEnhancementKnown ();
            }
        }
        /* 
        if ((wasunknown && obj->isIlkKnown ()) || 
            (knowledge != obj->getFlag (shObject::kKnownExact)))
        */
        {
            char buf[80];
            obj->resetWorn ();
            obj->inv (buf, 80);
            obj->setWorn ();
            I->p ("%c - %s", obj->mLetter, buf);
        }
    }
    return 1;
}


int
shCreature::doff (shObject *obj, int quiet /* = 0 */ )
{
    if (obj == mBodyArmor) {
        mBodyArmor = NULL;
    } else if (obj == mJumpsuit) {
        mJumpsuit = NULL;
    } else if (obj == mHelmet) {
        mHelmet = NULL;
    } else if (obj == mGoggles) {
        mGoggles = NULL;
    } else if (obj == mBelt) {
        mBelt = NULL;
    } else {
        int i;
        for (i = 0; i < shImplantIlk::kMaxSite; i++) {
            if (obj == mImplants[i]) {
                mImplants[i] = NULL;
                goto doffed;
            }
        }
        return 0;
    }
doffed:
    obj->resetWorn ();
    computeIntrinsics ();
    computeAC ();
    if (isHero ()) {
        if (obj->isA ("cortex crossover")) {
            I->p ("You flip out!");
        } else if (obj->isA ("tissue regenerator")) {
            mLastRegen = MAXTIME;
        } else if (obj->isA ("chameleon suit")) {
            I->p ("You no longer blend in with your surroundings.");
        } else if (obj->isA ("jetpack")) {
            if (!isFlying ()) {
                obj->setIlkKnown ();
                I->p ("You float down.");
            }
        }
    }
    return 1;
}


void
shCreature::damageEquipment (shAttack *atk, shEnergyType energy)
{
    shObjectVector v;
    shObject *obj = NULL;

    if (kCorrosive == energy) {
        if (mWeapon) v.add (mWeapon);
        if (mBodyArmor) {
            v.add (mBodyArmor);
        } else if (mJumpsuit) {
            v.add (mJumpsuit);
        }
        if (mBoots) v.add (mBoots);
        if (mBelt) v.add (mBelt);
        if (mHelmet) v.add (mHelmet);
        if (mGoggles) v.add (mGoggles);
    } else if (kMagnetic == energy ||
               (kElectrical == energy && !mHelmet)) 
    {
        int i;
        for (i = 0; i < shImplantIlk::kMaxSite; i++) {
            if (mImplants[i]) {
                v.add (mImplants[i]);
            }
        }
        if (kElectrical == energy && RNG (2) && loseEnergy (NDX (2, 10))) {
            if (isHero ()) {
                I->p ("Some of your energy cells have shorted out!");
            }
        }
    } else if (kBurning == energy) {
        if (mBodyArmor) {
            v.add (mBodyArmor);
        } else if (mJumpsuit) {
            v.add (mJumpsuit);
        }
        if (mBoots) v.add (mBoots);
        selectObjects (&v, mInventory, kFloppyDisk);
    } else if (kBugging == energy) {
        selectObjectsByFunction (&v, mInventory, &shObject::isBuggy, 1);
    }

    if (0 != v.count ()) {
        obj = v.get (RNG (v.count ()));
        if (obj->sufferDamage (atk, mX, mY, 1, 1)) {
            if (isHero()) {
                Hero.usedUpItem (obj, 1, "destroy");
            }
            delete removeOneObjectFromInventory (obj);
        }
    }
    computeIntrinsics ();
    computeAC ();
}


int 
shCreature::canSmell (shCreature *c)
{
    if (!hasScent ()) return 0;
    if (distance (this, c) <= 25) return 1;
    return 0;
}


//RETURNS: 1 if successful, 0 o/w
int 
shCreature::openDoor (int x, int y)
{
    shFeature *f = mLevel->getFeature (x,y);
    if (shFeature::kDoorClosed != f->mType ||
        !isAdjacent (x, y)) 
    {
        return 0;
    }
    if (0 == numHands ()) {
        return 0;
    }
    if (shFeature::kLocked & f->mDoor) {
        if (&Hero == this) {
            shObjectIlk *keyneeded = f->keyNeededForDoor ();
            I->p ("The door is locked.  A %s is required.", 
                  isBlind () ? "keycard of some kind" : keyneeded->mName);
        }
        return 0;
    }
    f->mType = shFeature::kDoorOpen;
    return 1;
}


//RETURNS: 1 if successful, 0 o/w
int 
shCreature::closeDoor (int x, int y)
{
    shFeature *f = mLevel->getFeature (x,y);
    if (shFeature::kDoorOpen != f->mType ||
        !isAdjacent (x, y) || 
        0 != mLevel->countObjects (x, y)) 
    {
        return 0;
    }
    if (0 == numHands ()) {
        return 0;
    }
    f->mType = shFeature::kDoorClosed;
    return 1;
}


/* returns true if the creature's movement's would trigger a motion detector */
int
shCreature::isMoving ()
{ 
    if (mConcealment > 15) {
        return 0;
    }
    if (mLastMoveTime + 2500 > Clock) {
        return 1;
    }
    switch (mType) {
    case kBot:
    case kConstruct:
    case kDroid:
    case kEgg:
    case kAlien:
    case kProgram:
    case kOoze:
        /* these creatures have the ability to remain perfectly still */
        return 0;
    case kCyborg:
    case kAbberation:
    case kAnimal:
    case kBeast:
    case kHumanoid:
    case kMutant:
    case kInsect:
    case kOutsider:
    case kVermin:
        /* the creatures shift around enough when lurking that they are
           still detectable */
        return 1;
    }
    return 0;
}


static
int
permuteScore (int base, int score)
{
    score += (base - 10);
    return score < 3 ? 3 : score;
}


void
shCreature::rollAbilityScores (int strbas, int conbas, int agibas, int dexbas, 
                               int intbas, int wisbas, int chabas)
{
    if (isHero ()) {
        int totalpoints = strbas + conbas + agibas + dexbas +
                          intbas + wisbas + chabas;

        mAbil.mStr = strbas; 
        mAbil.mCon = conbas;
        mAbil.mAgi = agibas;
        mAbil.mDex = dexbas;
        mAbil.mInt = intbas;
        mAbil.mWis = wisbas;
        mAbil.mCha = chabas;
        
        while (totalpoints < 83) {
            int idx = RNG (1, 7);
            int score = mAbil.getByIndex (idx);

            if (RNG (3, 18) <= score
                && score < 17) 
            { 
                /* prefer to increase already high scores */
                int boost = RNG (1, 2);
                mAbil.setByIndex (idx, score + boost);
                totalpoints += boost;
            }
        }
    } else if (mCLevel < 4 && RNG (6)) {
        /* lower level monsters have less variety */
        mAbil.mStr = permuteScore (strbas, 5 + NDX (2, 4));
        mAbil.mCon = permuteScore (conbas, 5 + NDX (2, 4));
        mAbil.mAgi = permuteScore (agibas, 5 + NDX (2, 4));
        mAbil.mDex = permuteScore (dexbas, 5 + NDX (2, 4));
        mAbil.mInt = permuteScore (intbas, 5 + NDX (2, 4));
        mAbil.mWis = permuteScore (wisbas, 5 + NDX (2, 4));
        mAbil.mCha = permuteScore (chabas, 5 + NDX (2, 4));
    } else {
        mAbil.mStr = permuteScore (strbas, NDX (3, 6));
        mAbil.mCon = permuteScore (conbas, NDX (3, 6));
        mAbil.mAgi = permuteScore (agibas, NDX (3, 6));
        mAbil.mDex = permuteScore (dexbas, NDX (3, 6));
        mAbil.mInt = permuteScore (intbas, NDX (3, 6));
        mAbil.mWis = permuteScore (wisbas, NDX (3, 6));
        mAbil.mCha = permuteScore (chabas, NDX (3, 6));
    }

    mMaxAbil.mStr = mAbil.mStr;
    mMaxAbil.mCon = mAbil.mCon;
    mMaxAbil.mAgi = mAbil.mAgi;
    mMaxAbil.mDex = mAbil.mDex;
    mMaxAbil.mInt = mAbil.mInt;
    mMaxAbil.mWis = mAbil.mWis;
    mMaxAbil.mCha = mAbil.mCha;
    mExtraAbil.mStr = 0;
    mExtraAbil.mCon = 0;
    mExtraAbil.mAgi = 0;
    mExtraAbil.mDex = 0;
    mExtraAbil.mInt = 0;
    mExtraAbil.mWis = 0;
    mExtraAbil.mCha = 0;

}


static const int abilworth[25] =
{
   -10, -10, -10, -10,  -6, 
    -3,  -1,   1,   3,   5,
     6,   7,   8,   9,  10,
    11,  13,  15,  17,  20,
    23,  26,  29,  32,  35 
};

/* because of the effect on carrying capacity, a low strength really
   hurts a starting character
*/
static const int strworth[25] =
{
    -40, -40, -40, -40, -30,
    -20, -10,  -5,   0,   3,
      6,   8,  10,  12,  13, 
     14,  16,  18,  20,  22, 
     24,  26,  29,  32,  35
};

/* agility is also more important to a character because of the effect
   on armor class
*/
static const int agiworth[25] =
{
    -40, -40, -20, -15, -11,
     -8,  -5,  -2,   1,   4,
      6,   8,  10,  12,  14, 
     16,  18,  20,  23,  26, 
     29,  32,  35,  38,  41    
};


/* returns: */
int
shCreature::statsViability ()
{
    int score = 0;
    int idx, x;

    for (idx = 1; idx <= 7; idx++) {
        x = mMaxAbil.getByIndex (idx);
        if (kStr == idx) {
            score += strworth [x];
        } else if (kAgi == idx) {
            score += agiworth [x];
        } else {
            score += abilworth [x];
        }
    }
    return score;
}

void
shCreature::rollHitPoints (int hitdice, int diesize)
{
    int i;
    mMaxHP = 0;
    for (i = 0; i < hitdice; i++) {
        int roll = (0 == i && isHero ()) ? diesize : RNG (1, diesize)
            + ABILITY_MODIFIER (getCon ());;
        mMaxHP += roll > 1 ? roll : 1;
    }
    mHP = mMaxHP;
}


shThingSize
shCreature::getSize ()
{
    return mIlk->mSize;
}


void
shCreature::computeAC ()
{
    mAC = 10 + mNaturalArmorBonus;
    mAC += ABILITY_MODIFIER (getAgi ());

    switch (getSize ()) {
    case kFine: mAC += 8; break;
    case kDiminutive: mAC += 4; break;
    case kTiny: mAC += 2; break;
    case kSmall: mAC += 1; break;
    case kMedium: break;
    case kLarge: mAC -= 1; break;
    case kHuge: mAC -= 2; break;
    case kGigantic: mAC -= 4; break;
    case kColossal: mAC -= 8; break;
    default:
        I->p ("computeAC: unknown size!");
        abort ();
    }
        
    if (mBodyArmor) {
        mAC += mBodyArmor->getArmorBonus ();
    }
    if (mJumpsuit) {
        mAC += mJumpsuit->getArmorBonus ();
    }
    if (mHelmet) {
        mAC += mHelmet->getArmorBonus ();
    }
    if (mBelt) {
        mAC += mBelt->getArmorBonus ();
    }
    if (mBoots) {
        mAC += mBoots->getArmorBonus ();
    }   
}


/*
  According to this article I just read, US Soldiers carry 54
  kilograms of gear, and this load encumbers them.
*/

static int CarryingCapacityByStrength[30] = {
0,      4535,   5210,   5985,   6875,   7897,   9071,   10420,  11970,  13750,
15795,  18143,  20841,  23940,  27500,  31590,  36287,  41683,  47881,  55001,
63180,  72574,  83366,  95762,  110002, 126360, 145149, 166733, 191525, 220005
};  

void
shCreature::computeIntrinsics ()
{
    shObject *obj;
    int i;
    static int oldburden = 0; 
    int newburden;
    int weight;
    int intrinsics;
    
    mIntrinsics = mInateIntrinsics;
    memcpy (&mResistances, &mInateResistances, sizeof (mResistances));
    mConcealment = 0;
    mPsiModifier = 0;
    mSpeed = mIlk->mSpeed + mSpeedBoost;
    for (i = 1; i <= 7; i++) {
        mMaxAbil.changeByIndex (i, - mExtraAbil.getByIndex (i));
        mAbil.changeByIndex (i, - mExtraAbil.getByIndex (i));
        mExtraAbil.setByIndex (i, 0);
    }
    for (i = 0; i < mInventory->count (); i++) {
        obj = mInventory->get (i);
        intrinsics = obj->getConferredIntrinsics ();
        if (kCrazyIvan == intrinsics) {
            mIntrinsics ^= intrinsics;
        } else {
            mIntrinsics |= intrinsics;
        }
        obj->applyConferredResistances (this);
    }
    for (i = 1; i <= 7; i++) {
        mMaxAbil.changeByIndex (i, mExtraAbil.getByIndex (i));
        mAbil.changeByIndex (i, mExtraAbil.getByIndex (i));
    }
    if (mGoggles && 
        mGoggles->isA ("pair of peril-sensitive sunglasses") && 
        mGoggles->isToggled () &&
        !hasXRayVision ())
    {
        setBlind ();
    }      

    if (isHero ()) {
        I->crazyIvan (mIntrinsics & kCrazyIvan);
    }
    if (mAbil.mStr <= 0) {
        mCarryingCapacity = 0;
    } else if (mAbil.mStr < 30) {
        mCarryingCapacity = CarryingCapacityByStrength[mAbil.mStr];
    } else {
        mCarryingCapacity = 64 * CarryingCapacityByStrength[29];
    }
    
    if (mBodyArmor && mBodyArmor->isPoweredArmor ()) {
        mCarryingCapacity += mBodyArmor->getMass ();
    }

    weight = mWeight;

    if (isViolated ()) {
        mSpeed -= 50;
    }
    if (isSpeedy ()) {
        mSpeed += 100;
    }
    if (isHosed ()) {
        mSpeed -= 100;
    }
    mSpeed += 5 * ABILITY_BONUS (getAgi ());

    if (weight <= mCarryingCapacity) {
        newburden = 0;
    } else if (weight <= mCarryingCapacity * 2) {
        newburden = kBurdened;
        mSpeed -= 50;
    } else if (weight <= mCarryingCapacity * 3) {
        newburden = kStrained;
        mSpeed -= 100;
    } else if (weight <= mCarryingCapacity * 4) {
        newburden = kOvertaxed;
        mSpeed -= 300;
    } else {
        newburden = kOverloaded;
        mSpeed -= 700;
    }

    if (isHero () && oldburden != newburden) {
        if ((unsigned) oldburden > (unsigned) newburden) {
            if (0 == newburden) {
                I->p ("You movement is no longer encumbered.");
            } else {
                I->p ("Your movement is somewhat less encumbered now.");
            }
        } else if (kBurdened == newburden) {
            I->p ("You are encumbered by your load.");
        } else if (kStrained == newburden) {
            I->p ("You strain to manage your load.");
        } else if (kOvertaxed == newburden) {
            I->p ("You can barely move under this load.");
        } else {
            I->p ("You collapse under your load.");
        }
        I->drawSideWin ();
        oldburden = newburden;
    }
    mConditions &= ~kOverloaded;
    mConditions |= newburden;
}


/*
  RETURNS: a d20 roll, with small possibility of rolling higher than 20,
           to give a sporting chance of achieving a high result
*/

int
sportingD20 ()
{
    int result;

    result = RNG (1, 21);
    if (21 == result || Hero.isLucky ()) {
        do {
            result += RNG (6);
        } while (! RNG (3));
    }
    return result;
}


shSkill *
shCreature::getSkill (shSkillCode c, shMutantPower power)
{
    int i;
    shSkill *s;

    for (i = 0; i < mSkills.count (); i++) {
        s = mSkills.get (i);
        if (c == s->mCode) {
            switch (c) {
            case kMutantPower:
                if (power == s->mPower) {
                    return s;
                }
                break;
            default:
                return s;
            }
        }
    }
    return NULL;
}


int
shCreature::getSkillModifier (shSkillCode c, 
                              shMutantPower power)
{
    shSkill *skill;
    int ability;
    int result = 0;

    switch (c) {
    case kMutantPower:
        skill = getSkill (c, power);
        ability = kCha;
        break;
    default:
        skill = getSkill (c);
        ability = SKILL_KEY_ABILITY (c);
        break;
    }
    result += ABILITY_MODIFIER (mAbil.getByIndex (ability));
    if (skill) {
        result += skill->mRanks + skill->mBonus;
    }
    if (isSickened ()) {
        result -= 2;
    }
    return result;
}


int
shCreature::getWeaponSkillModifier (shObjectIlk *ilk)
{
    shSkillCode c;
    shSkill *skill = NULL;
    int ability;
    int result = 0;
    
    if (NULL == ilk) {
        /* barehanded */
        c = kUnarmedCombat;
        ability = SKILL_KEY_ABILITY (c);
        skill = getSkill (c);
    } else if (kWeapon == ilk->mType) {
        c = ((shWeaponIlk *) ilk) -> mSkill;
        ability = SKILL_KEY_ABILITY (c);
        skill = getSkill (c);
    } else {
        /* improvised melee weapon */
        ability = kStr;
        result -= 4;
    }

    result += mBAB;
    switch (getSize ()) {
    case kFine: result += 8; break;
    case kDiminutive: result += 4; break;
    case kTiny: result += 2; break;
    case kSmall: result += 1; break;
    case kMedium: break;
    case kLarge: result += 1; break;
    case kHuge: result += 2; break;
    case kGigantic: result += 4; break;
    case kColossal: result += 8; break;
    default:
        I->p ("getWeaponSkillModifier: unknown size!");
        abort ();
    }

    result += ABILITY_MODIFIER (mAbil.getByIndex (ability));
    if (NULL == skill) {
        /* inflict a slight penalty for lacking weapon skill,
           since we're not using SRD Feats */
        result -= 2; 
    } else {
        result += skill->mRanks + skill->mBonus;
    }
    if (isSickened ()) {
        result -= 2;
    }
    return result;
}


int
shCreature::gainRank (shSkillCode c, int howmany, shMutantPower power)
{
    shSkill * skill;

    skill = getSkill (c, power);
    if (NULL == skill) {
        skill = new shSkill (c, power);
        mSkills.add (skill);
    }
/*
    if (mCLevel < skill->mRanks) {
        return 0;
    }
*/
    skill->mRanks += howmany;
    return 1;
}


int
shCreature::exerciseSkill (shSkillCode c, int amt, shMutantPower power)
{
#if 1
    return 1;
#else
    shSkill * skill;

    skill = getSkill (c, power);
    
    if (NULL == skill) {
        skill = new shSkill (c, power);
        mSkills.add (skill);
    }
    skill->mExercise += amt;
    if (skill->mExercise >= 10 * (skill->mRanks + 1)) {
          skill->mExercise -= 10 * (skill->mRanks + 1);
          ++skill->mRanks;
          I->p ("You feel like you're getting the hang of this.");
    }
    return 1;
#endif
}




void
shCreature::addSkill (shSkillCode c, int access, shMutantPower power)
{
    shSkill *skill = new shSkill (c, power);
    skill->mAccess = access;
    mSkills.add (skill);
}
