/***************************************************************
 This file is for all the ranged weapon attack resolution code

****************************************************************/

#include <unistd.h>

#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Interface.h"

static shSpecialEffect
beamSpecialEffect (shAttack *atk)
{
    switch (atk->mType) {
    case shAttack::kHeatRay:    
    case shAttack::kBlast:
    case shAttack::kBreatheFire:
        return kExplosionEffect;
    case shAttack::kFlash:
        return kRadiationEffect;
    case shAttack::kFreezeRay:
        return kColdEffect;
    case shAttack::kPoisonRay: 
        return kPoisonEffect;
    case shAttack::kBreatheTime:
    case shAttack::kBreatheTraffic:
    case shAttack::kBreatheBugs:
    case shAttack::kBreatheViruses:
    case shAttack::kDisintegrationRay: 
    case shAttack::kGammaRay:
    case shAttack::kGaussRay:
    case shAttack::kTransporterRay:
    case shAttack::kStasisRay:
    case shAttack::kHealingRay:
    case shAttack::kRestorationRay:
    default:
        return kInvisibleEffect;
    }
}


static const char *
beamHitsMesg (shAttack *atk)
{
    switch (atk->mType) {
    case shAttack::kBlast: return NULL; /*"The blast hits";*/
    case shAttack::kBreatheFire: return "The fireball hits";
    case shAttack::kBreatheBugs: return "The cloud of bugs envelopes";
    case shAttack::kBreatheViruses: return "The cloud of viruses envelopes";
    case shAttack::kBreatheTime: return "The time warp envelopes";
    case shAttack::kBreatheTraffic: return "The packet storm envelopes";
    case shAttack::kDisintegrationRay: 
        return "The disintegration ray hits";
    case shAttack::kFlash: return NULL;
    case shAttack::kFreezeRay: return "The freeze ray hits";
    case shAttack::kGammaRay: return NULL;
    case shAttack::kGaussRay: return NULL;
    case shAttack::kHeatRay: return "The heat ray hits";
    case shAttack::kLaser: return "The laser beam hits";
    case shAttack::kMentalBlast: return "The mental blast hits";
    case shAttack::kPoisonRay: return "The poison ray hits";
    case shAttack::kStasisRay: return "The stasis ray hits";
    case shAttack::kTransporterRay: return NULL;
    case shAttack::kHealingRay: return NULL;
    case shAttack::kRestorationRay: return NULL;
    }
    return "it hits you";
}


static const char *
monHitsYouMesg (shAttack *atk)
{
    switch (atk->mType) {
    case shAttack::kNoAttack: return " doesn't attack";
    case shAttack::kAttach: return " attaches an object to you";
    case shAttack::kAnalProbe: return " probes you";
    case shAttack::kBite: return " bites you";
    case shAttack::kBlast: return " blasts you";
    case shAttack::kBullet: return " shoots you";
    case shAttack::kBolt: return " blasts you";
    case shAttack::kCease: return " reads a cease and desist letter to you";
    case shAttack::kClaw: return " claws you";
    case shAttack::kClub: return " clubs you";
    case shAttack::kChoke: return " chokes you";
    case shAttack::kCook: return " cooks you";
    case shAttack::kCrush: return " crushes you";
    case shAttack::kDisintegrationRay: 
        return " zaps you with a disintegration ray";
    case shAttack::kExtractBrain: return " extracts your brain";
    case shAttack::kFaceHug: return " attacks your face";
    case shAttack::kFlash: return " blasts you with a bright light";
    case shAttack::kFreezeRay: return " zaps you with a freeze ray";
    case shAttack::kGammaRay: return " zaps you with a gamma ray";
    case shAttack::kGaussRay: return " zaps you with a gauss ray";
    case shAttack::kHeadButt: return " head butts you";
    case shAttack::kHealingRay: return " zaps you with a healing ray";
    case shAttack::kHeatRay: return " zaps you with a heat ray";
    case shAttack::kKick: return " kicks you";
    case shAttack::kLaser: return " zaps you with a laser beam";
    case shAttack::kMentalBlast: return " blasts your mind";
    case shAttack::kPoisonRay: return "zaps you with a poison ray";
    case shAttack::kPunch: return " punches you";
    case shAttack::kQuill: return " quills you";
    case shAttack::kRail: return " rails you";
    case shAttack::kRestorationRay: return " zaps you with a restoration ray";
    case shAttack::kSeize: return " rifles through your pack";
    case shAttack::kShot: return " shoots you";
    case shAttack::kSlash: return " slashes you";
    case shAttack::kSlime: return " slimes you";
    case shAttack::kSmash: return " smacks you";
    case shAttack::kStab: return " stabs you";
    case shAttack::kStasisRay: return " zaps you with a stasis ray";
    case shAttack::kSting: return " stings you";
    case shAttack::kSue: return " sues you";
    case shAttack::kSword: return " slices you";
    case shAttack::kTailSlap: return "'s tail whips you";
    case shAttack::kTouch: return " touches you";
    case shAttack::kTransporterRay: return " zaps you with a transporter ray";
    case shAttack::kZap: return " zaps you";
    }
    return "hits you";
}       


static const char *
monHitsMonMesg (shAttack *atk)
{
    switch (atk->mType) {
    case shAttack::kNoAttack: return " doesn't attack";
    case shAttack::kAnalProbe: return " probes";
    case shAttack::kAttach: return " attaches an object to";
    case shAttack::kBite: return " bites";
    case shAttack::kBlast: return " blasts";
    case shAttack::kBullet: return " shoots";
    case shAttack::kBolt: return " blasts";
    case shAttack::kCease: return " reads a cease and desist letter to";
    case shAttack::kClaw: return " claws";
    case shAttack::kClub: return " clubs";
    case shAttack::kChoke: return " chokes";
    case shAttack::kCook: return " cooks";
    case shAttack::kCrush: return " crushes";
    case shAttack::kDisintegrationRay: return " zaps a disintegration ray at";
    case shAttack::kExtractBrain: return " extracts the brain of";
    case shAttack::kFaceHug: return " attacks the face of";
    case shAttack::kFlash: return " flashes a bright light at";
    case shAttack::kFreezeRay: return " zaps a freeze ray at";
    case shAttack::kGammaRay: return " zaps a gamma ray at";
    case shAttack::kGaussRay: return " zaps a gauss ray at";
    case shAttack::kHeadButt: return " head butts";
    case shAttack::kHealingRay: return " zaps a healing ray at";
    case shAttack::kHeatRay: return " zaps a heat ray at";
    case shAttack::kKick: return " kicks";
    case shAttack::kLaser: return " zaps a laser beam at";
    case shAttack::kMentalBlast: return " blasts the mind of";
    case shAttack::kPoisonRay: return " zaps a poison ray at";
    case shAttack::kPunch: return " punches";
    case shAttack::kQuill: return " quills";
    case shAttack::kRail: return " rails";
    case shAttack::kRestorationRay: return " zaps a restoration ray at";
    case shAttack::kSeize: return " rifles through the pack of";
    case shAttack::kShot: return " shoots";
    case shAttack::kSlash: return " slashes";
    case shAttack::kSlime: return " slimes";
    case shAttack::kSmash: return " smacks";
    case shAttack::kStab: return " stabs";
    case shAttack::kStasisRay: return " zaps a stasis ray at";
    case shAttack::kSting: return " stings";
    case shAttack::kSue: return " sues";
    case shAttack::kSword: return " slices";
    case shAttack::kTailSlap: return "'s tail whips";
    case shAttack::kTouch: return " touches";
    case shAttack::kTransporterRay: return " zaps a transporter ray at";
    case shAttack::kZap: return " zaps";
    }
    return "hits you";
}       





/* returns: the bonus to AC afforded by the supplied cover percentage */
static int
CoverACBonus (int cover)
{
    if (cover > 0) {
        /* expanded interpretation of SRD allows for more ranges of cover */
        if (cover <= 15) { return 1; }
        else if (cover <= 25) { return 2; }
        else if (cover <= 40) { return 3; }
        else if (cover <= 50) { return 4; }
        else if (cover <= 60) { return 5; }
        else if (cover <= 70) { return 6; }
        else if (cover <= 75) { return 7; }
        else if (cover <= 80) { return 8; }
        else if (cover <= 85) { return 9; }
        else if (cover <= 90) { return 10; }
        else { return cover - 80; }
    } else {
        return 0;
    }
}


/* returns: 1 if the attack misses due to visibility problem
            0 otherwise
*/
static int
VisibilityMiss (int vis)
{
    if (vis < 100) {
        int misschance;
        if (vis >= 75) { misschance = 10; }
        else if (vis >= 50) { misschance = 20; }
        else if (vis >= 25) { misschance = 30; }
        else if (vis >= 10) { misschance = 40; }
        else { misschance = 50; }

        if (RNG (100) < misschance) {
            return 1;
        }
    }
    return 0;
}


/* roll to save against the attack
   returns: 0 if save failed, true if made
*/

int
shCreature::reflexSave (shAttack *attack, int DC)
{
    int result = RNG (1, 20) + mReflexSaveBonus + ABILITY_MODIFIER (getAgi ());

    if (isAsleep ()) {
        return 0;
    }
    if (Hero.isLucky ()) {
        if (isHero () || isPet ()) {
            result += RNG (1, 7);
        } else {
            result -= RNG (1, 7);
        }
    } 
    
    return result >= DC;
}



/* roll to hit the target 
   returns: -x for a miss (returns the amount missed by)
            1 for a hit
            2+ for a critical (returns the appropriate damage multiplier)
*/

int
shCreature::attackRoll (shAttack *attack, shObject *weapon, 
                        int attackmod, int AC, shCreature *target)
{
    int result = RNG (1, 20);
    int dmul = 1;
    int threatrange = weapon ? weapon->getThreatRange (target) : 20;
    int critmult = weapon ? weapon->getCriticalMultiplier () : 2;
    char buf[64];
    
    target->the (buf, 64);

    if (1 == result) { /* rolling a 1 always results in a miss */
        I->diag ("attacking %s: rolled a 1 against AC %d.", buf, AC); 
        return -99;
    }
    if (20 == result ||
        result + attackmod >= AC) 
    {   /* hit */
        if ((!target->isImmuneToCriticalHits ()) && 
            (result >= threatrange))
        {   /* critical threat */
            int threat = RNG (1, 20);
            if ((1 != threat) &&
                ((20 == threat) || (threat + attackmod >= AC)))
            {   /* critical hit */
                dmul = critmult;
                I->diag ("attacking %s: rolled %d, then %d+%d=%d against "
                         "AC %d: critical hit, damage multiplier %d!", 
                         buf, result, threat, attackmod, threat + attackmod, 
                         AC, dmul);
            } else {
                I->diag ("attacking %s: rolled %d, then %d+%d=%d against AC %d", 
                         buf, result, threat, attackmod, threat + attackmod, AC);
            }
        } else {
            I->diag ("attacking %s: rolled a %d+%d=%d against AC %d", 
                     buf, result, attackmod, result + attackmod, AC);
        }
        return dmul;
    }
    I->diag ("attacking %s: rolled a %d+%d=%d against AC %d",
             buf, result, attackmod, result + attackmod, AC);
    return result + attackmod - AC;
}



/* works out the ranged attack against the target
   returns: 1 if the target is eliminated (e.g. it dies, teleports away, etc.)
            0 if the target is hit but not eliminated
           -1 if the attack was a complete miss
           -2 if the attack was reflected
*/

int
shCreature::resolveRangedAttack (shObject *weapon,
                                 shAttack *attack,
                                 int attackmod,
                                 shCreature *target)
{
    int AC;
    int flatfooted = 0;   /* set to 1 if the target is unable to react to the
                             attack due to unawareness or other disability */
    int cover;
    int vis;

    char n_attacker[64];
    char an_attacker[64];
    char n_target[64];
    char n_weapon[64];

    int range = distance (target, this);
    int wrange = attack->mRange;
    int maxrange;

    int dbonus = 0;
    int dmul = -1;

    int cantsee = 1;

    if (isHero ()) {
        strcpy (n_attacker, "you");
        cantsee = 0;
    } else if (Hero.canSee (this)) {
        the (n_attacker, 64);
        cantsee = 0;
    } else {
        strcpy (n_attacker, "something");
    }
    if (target->isHero ()) {
        strcpy (n_target, "you");
        cantsee = 0;
    } else if (Hero.canSee (target)) {
        target->the (n_target, 64);
        cantsee = 0;
    } else {
        strcpy (n_target, "something");
    }

    an (an_attacker, 64);

/* this message is confusing
    if (cantsee) {
        I->p ("You hear the sounds of combat.");
    }
*/
    if (weapon) {
        weapon->the (n_weapon, 64);
        dbonus = weapon->mEnhancement;
        attackmod += ((shWeaponIlk *) weapon->mIlk) -> mToHitBonus;
        if (mGoggles && mGoggles->isA ("pair of targetter goggles")) {
            attackmod += 2;
            dbonus += 2;
        }
    } else if (&OpticBlastAttack == attack) {
        strcpy (n_weapon, "the laser blast");
    } else {
        strcpy (n_weapon, "it");
    }

    if (!attack->isMeleeAttack () && 
        target->hasShield () &&
        target->countEnergy ())
    {   /* almost always hits the targets shield and the target gets no 
           benefit from concealment */
        attackmod += 100;
    } else if (target->mConcealment && RNG (100) < target->mConcealment) {
        /* should telepathy counter concealment? */
        I->debug ("missed due to concealment");
        goto youmiss;
    }

    vis = canSee (target, &cover);

    if (attack->isMissileAttack ()) {
        maxrange = 10 * wrange; /* for thrown weapons, it's actually 2d4 
                                   squares.  see throwObject() for details */
        if (weapon->isThrownWeapon ()) {
            attackmod += getWeaponSkillModifier (weapon->mIlk);
            attackmod += weapon->mEnhancement;
            attackmod -= 2 * weapon->mDamage;
        } else {
            /* improvised missile weapon */
            attackmod += ABILITY_MODIFIER (getDex ());
            //FIX: assess stiff penalties for large or impractical missiles
        }
    } else {
        /* some kind of aimed weapon or a psionic attack */
        maxrange = 10 * wrange;
        attackmod += getWeaponSkillModifier (weapon ? weapon->mIlk : NULL);
        if (weapon) {
            attackmod += weapon->mEnhancement;
            attackmod -= 2 * weapon->mDamage;
            if (weapon->isOptimized ()) {
                attackmod += 2;
            }
        }
    }

    
    if (0 == target->canSee (this)) {
        attackmod += 2;
        flatfooted = 1;
    }
    if (target->isStunned ()) {
        flatfooted = 1;
        attackmod += 2;
    }
    if (target->isAsleep ()) {
        flatfooted = 1;
        attackmod += 4;
    }
        
    if (range > maxrange) { 
        goto youmiss;
    }

    attackmod -= 2 * (range / wrange);

    if (attack->isTouchAttack ()) {
        AC = target->getTouchAC (flatfooted, this);
    } else {
        AC = target->getAC (flatfooted, this);
    }
    if (cover >= 100) {
        /* normally it would be impossible to see the target if there is
           100% cover, unless the cover is transparent (e.g. a window) */
        goto youmiss;
    } else {
        AC += CoverACBonus (cover);
    }

    if (VisibilityMiss(vis)) {
        goto youmiss;
    }

    /* roll to hit */
    
    dmul = attackRoll (attack, weapon, attackmod, AC, target);
    
    if (dmul <= 0) {
        //FIX: determine if the attack hit armor or just plain missed
        dmul = -1;
        goto youmiss;
    }

    /* successful hit */

    if (attack->isMissileAttack () && weapon) {
        //I->p ("%s is hit!", n_target);
        weapon->impact (target, 
                        vectorDirection (mX, mY, target->mX, target->mY),
                        this);
        exerciseWeaponSkill (weapon, 1);
        return 0;  /* FIX */
    }

    if (target->reflectAttack (attack)) {
        return -2;
    }
    if (isHero ()) {
        if (dmul > 1) {
            I->p ("You hit %s!", n_target);
        } else {
            I->p ("You hit %s.", n_target);
        }
        if (weapon) {
            /* FIX: doesn't exercise unarmed combat. 
               (anyways, exercise has been phased out for now...) */
            exerciseWeaponSkill (weapon, 1);
        }
        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            I->p ("%s is %s!", n_target, target->deathVerb ());
            if (!target->isHero ()) {
                Hero.earnXP (target->mCLevel);
            }
            target->die (kSlain);
            return 1;
        } else {
            target->newEnemy (this);
            target->interrupt ();
            return 0;
        }
    } else if (target->isHero ()) {
        if (dmul > 1) {
            I->p ("You are hit!");
        } else {
            I->p ("You are hit.");
        }
        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            target->die (kSlain, this);
            return 1;
        }
        target->interrupt ();
        return 0;
    } else {
        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            if (!cantsee) {
                I->p ("%s is %s!", n_target, target->deathVerb ());
            }
            target->die (kSlain);
            return 1;
        } else if (dmul > 1) {
            if (!cantsee) {
                I->p ("%s is hit!", n_target);
            }
        } else {
            if (!cantsee) {
                I->p ("%s is hit.", n_target);
            }
        }
        if (isPet ()) {
            /* monsters will tolerate friendly-fire */
            target->newEnemy (this);
        }
        target->interrupt ();
        return 0;
    }

 youmiss:
    if (attack->isMissileAttack ()) {
        if (target->isHero ()) {
            I->p ("%s misses you!", n_weapon);
        } else {
            I->p ("%s misses %s", n_weapon, n_target);
        }
    } else if (&OpticBlastAttack == attack) {
        if (isHero () && target->mHidden <= 0) {
            I->p ("Your laser beam misses %s.", n_target);
        } 
    } else {
        if (isHero () && target->mHidden <= 0) {
            I->p ("Your shot misses %s.", n_target);
        } 
/* 
          else if (target->isHero ()) {
            I->p ("%s misses.", n_attacker);
        } else {
            I->p ("%s misses %s", n_attacker, n_target);
        }
*/
    }
    target->interrupt ();
    target->newEnemy (this);

    return -1;
}


/*  work out the result of firing the weapon in the given direction
    returns: ms elapsed, -2 if attacker dies
*/
int
shCreature::shootWeapon (shObject *weapon, shDirection dir)
{
    int numrounds;
    int x, y;
    int firsttarget = 1;
    shAttack *attack;
    int setemptyraygun = 0;

    assert (weapon);

    if (weapon->isA (kWeapon)) {
        attack = & ((shWeaponIlk *) weapon->mIlk) -> mAttack;
        numrounds = expendAmmo (weapon);
    } else if (weapon->isA (kRayGun)) {
        attack = & ((shRayGunIlk *) weapon->mIlk) -> mAttack;
        if (weapon->mCharges) {
            numrounds = 1;
            --weapon->mCharges;
            if (!weapon->mCharges && weapon->isChargeKnown ()) {
                ++setemptyraygun;
            }
        } else {
            numrounds = 0;
        }
    }

    if (0 == numrounds) {
        if (isHero ()) {
            if (weapon->isA (kRayGun)) {
                weapon->mIlk = findAnIlk (&RayGunIlks, "empty ray gun");
                weapon->setChargeKnown ();
                weapon->setIlkKnown ();
                weapon->setAppearanceKnown ();
                I->p ("Nothing happens.");
            } else {
                I->p ("You're out of ammo!");
            } 
        }
        return 200; /* this wastes some time */
    }

    if (!isHero ()) {
        int knownappearance = weapon->isAppearanceKnown ();
        weapon->setAppearanceKnown ();
        if (Hero.canSee (this)) {
            char buf[80];
            char buf2[80];
            the (buf, 80);
            weapon->her (buf2, 80, this);
            I->p ("%s shoots %s!", buf, buf2);
        } else {
            char buf2[80];
            weapon->an (buf2, 80);
            I->p ("You hear someone shooting %s.", buf2);
            if (!knownappearance) {
                weapon->resetAppearanceKnown ();
            }
            if (!Hero.isBlind () && Level->isInLOS (mX, mY)) {
                /* muzzle flash gives away position */
                Level->feelSq (mX, mY);
            }
        }
    } else if (!isBlind ()) {
        /* some weapons identify themselves when shot */
        if (weapon->isA ("heat ray gun") ||
            weapon->isA ("freeze ray gun") ||
            weapon->isA ("disintegration ray gun") ||
            weapon->isA ("stasis ray gun") ||
            weapon->isA ("poison ray gun"))
        {
            weapon->setIlkKnown ();
        }
    }


    if (weapon->isBuggy () && !RNG (5)) {
        if (weapon->isA (kRayGun) && RNG (3)) {
            int died;
            if (isHero ()) {
                I->p ("Your ray gun explodes!");
            } else if (Hero.canSee (this)) {
                char buf[80];
                the (buf, 80);
                I->p ("%s's ray gun explodes!", buf);
            } else {
                I->p ("You hear an explosion");
            }
            /* this is tricky, hope I got it right:
               1. delete the weapon first, so no risk of double-deletion due to
                  it somehow getting destroyed by a secondary explosion effect
               2. kludgily borrow and modify the weapon's own attack structure
                  to save typing in new exploding ray gun attacks
               3. remember to return -2 if the attacker is killed in the 
                  explosion, because higher level code will handle deletion
            */
            removeObjectFromInventory (weapon);
            delete weapon;
            attack->mEffect = shAttack::kBurst; 
            died = mLevel->areaEffect(attack, mX, mY, kNoDirection, this);
            attack->mEffect = shAttack::kBeam;
            return died ? -2 : 1000;
        }
    
        if (isHero ()) {
            weapon->setBugginessKnown ();
            I->p ("Your weapon misfires!");
        } else if (Hero.canSee (this)) {
            char buf[80];
            the (buf, 80);
            I->p ("%s's weapon misfires!", buf);
            weapon->setBugginessKnown ();
        } else {
            I->p ("You hear a weapon misfire.");
        }
        return 1000;
    }

    switch (attack->mEffect) {
    case shAttack::kSingle:
        if (kOrigin == dir) {
            if (isHero ()) {
                I->p ("You shoot yourself.");
            }
            if (sufferDamage (attack, this, 0, 1)) {
                die (kSuicide);
                return -2;
            }
            return attack->mAttackTime;
        }
        else if (kUp == dir) {
            if (isHero ()) {
                I->p ("You shoot at the ceiling.");
            }
            return attack->mAttackTime;
        } else if (kDown == dir) {
            if (isHero ()) {
                I->p ("You shoot at the floor.");
            }
            return attack->mAttackTime;
        }
        if (-1 == numrounds) { /* pea shooter */
            numrounds = 1;
        }
        
        while (numrounds--) {
            int timeout = 100;
            x = mX;
            y = mY;
            firsttarget = 1;
            while (Level->moveForward (dir, &x, &y) 
                   && --timeout) 
            {
                if (Level->isOccupied (x, y)) {
                    shCreature *c = Level->getCreature (x, y);
                    int maintarget = isHero () ? firsttarget : c->isHero (); 
                    int r = resolveRangedAttack (weapon, attack,
                                                 maintarget ? 0 : -4,
                                                 Level->getCreature (x, y));
                    firsttarget = 0;
                    if (-2 == r) {
                        /* 
                        dir = uTurn (dir);
                        continue;
                        */
                        break;
                    } else if (r >= 0) {
                        break;
                    }
                }
                if (Level->isOcclusive (x, y)) {
                    /* TODO: shoot the obstacle */
                    break;
                }
            }
        }
        break;
    case shAttack::kBeam:
        if (Hero.canSee (this)) {
            weapon->setIlkKnown ();
        }
        Level->areaEffect (attack, mX, mY, dir, this);
        break;
    default:
        I->p ("Unkown weapon type!!");
        break;
    }

    if (setemptyraygun) {
        weapon->mIlk = findAnIlk (&RayGunIlks, "empty ray gun");
    }
        
    return attack->mAttackTime;
}


void
shCreature::projectile (shObject *obj, int x, int y, shDirection dir, 
                        shAttack *attack, int range)
{
    int firsttarget = 1;
    while (range--) {
        shFeature *f;

        if (!mLevel->moveForward (dir, &x, &y)) {
            mLevel->moveForward (uTurn (dir), &x, &y);
            obj->impact (x, y, dir, this);
            return;
        }

        if (mLevel->isOccupied (x, y)) {
            shCreature *c = mLevel->getCreature (x, y);
            int maintarget = isHero () ? firsttarget : c->isHero (); 
            int r = resolveRangedAttack (obj, attack, maintarget ? 0 : -4, c);
                                         
            firsttarget = 0;
            if (r >= 0) {
                /* a hit - resolveRangedAttack will have called obj->impact */
                return;
            } else if (Hero.canSee (c)) {
                /* a miss - assuming we were aiming at this creature, the 
                   object shouldn't land too far away */
                range = mini (range, RNG (1, 3) - 1);
                if (-1 == range) { /* land in the square in front */
                    mLevel->moveForward (uTurn (dir), &x, &y);
                    obj->impact (x, y, dir, this);
                }
            }
        }

        f = mLevel->getFeature (x, y);
        if (f) {
            switch (f->mType) {
            case shFeature::kDoorHiddenVert:
            case shFeature::kDoorHiddenHoriz:
            case shFeature::kDoorBerserkClosed:
            case shFeature::kDoorClosed:
            case shFeature::kComputerTerminal:
            case shFeature::kPortal: 
                /* the thrown object will hit these solid features */
                obj->impact (f, dir, this);
                return;
            case shFeature::kStairsUp:
            case shFeature::kStairsDown:
            case shFeature::kRadTrap:
            case shFeature::kDoorOpen:
            case shFeature::kDoorBerserkOpen:
            case shFeature::kMaxFeatureType: 
                /* these features it will fly right past */
                break;
            }
        }
        
        if (mLevel->isObstacle (x, y)) {
            obj->impact (x, y, dir, this);
            return;
        }
    }
    /* maximum range */
    obj->impact (x, y, dir, this);
}



/*  work out the result of throwing the object in the given direction
    returns: ms elapsed 
*/
int
shCreature::throwObject (shObject *obj, shDirection dir)
{
    shAttack *attack;
    int maxrange;

    if (isHero ()) {
        Hero.usedUpItem (obj, obj->mCount, "thow");
        obj->resetUnpaid ();
    }
    if (obj->isThrownWeapon ()) {
        attack = & ((shWeaponIlk *) obj->mIlk) -> mAttack;
    } else {
        attack = &ImprovisedMissileAttack;
    }

    if (!isHero () && Hero.canSee (this)) {
        char buf1[60], buf2[60];

        the (buf1, 60);
        obj->an (buf2, 60);
        I->p ("%s throws %s!", buf1, buf2);
    }

    if (kUp == dir) {
        if (isHero ()) {
            I->p ("It bounces off the ceiling and lands on your head!");
        }
        obj->impact (this, kDown, this);
        return attack->mAttackTime;
    } else if (kDown == dir) {
        obj->impact (mX, mY, kDown, this);
        return attack->mAttackTime;
    }

    /* 5 range increments @ 5 ft per increment */
    maxrange = 4 + ABILITY_MODIFIER (getStr ()) + NDX (2, 4);  

    projectile (obj, mX, mY, dir, attack, maxrange);

    return attack->mAttackTime;
}


shAttack KickedWallDamage =
    shAttack (NULL, shAttack::kSmash, shAttack::kOther, 0, kConcussive, 1, 2);

int
shHero::kick (shDirection dir)
{
    int x = mX;
    int y = mY;
    shFeature *f;
    char buf[80];

    feel (x, y);

    if (kUp == dir) {
        I->p ("You kick the air.");
        return FULLTURN;
    }

    if (!mLevel->moveForward (dir, &x, &y)) {

    } else if (kDown != dir && mLevel->isOccupied (x, y)) {
        shCreature *c = Level->getCreature (x, y);
        /* treat as unarmed attack */
        resolveMeleeAttack (NULL, mIlk->mAttacks.get (0), c);
    } else if (mLevel->countObjects (x, y)) {
        shObjectVector *v = mLevel->getObjects (x, y);
        shObject *obj = v->get (0);
        int maxrange = maxi (2, ABILITY_MODIFIER (getStr ()) + NDX (2, 4));  

        v->remove (obj);
        if (0 == v->count ()) {
            delete v;
            mLevel->setObjects (x, y, NULL);
        }
        obj->the (buf, 80);

        if (kDown == dir) {
            I->p ("You stomp on %s.", buf);
            if (obj->isUnpaid ()) {
                usedUpItem (obj, obj->mCount, "kick");
                obj->resetUnpaid ();
            }
            obj->impact (x, y, kDown, this);
        } else {
            if (obj->isUnpaid ()) {
                usedUpItem (obj, obj->mCount, "kick");
                obj->resetUnpaid ();
            }
            projectile (obj, x, y, dir, &ImprovisedMissileAttack, maxrange);
        }
    } else if ((f = mLevel->getFeature (x, y))) {
        int score = 
            sportingD20 () + 
            getWeaponSkillModifier (NULL);
        switch (f->mType) {
        case shFeature::kDoorHiddenVert:
        case shFeature::kDoorHiddenHoriz:
            score += f->mSportingChance++;
            if (score >= 20 && 
                !(shFeature::kLocked & f->mDoor)) 
            {
                I->p ("Your kick opens a secret door!");
                f->mType = shFeature::kDoorOpen;
                f->mSportingChance = 0;
            } else if (score > 18) {
                I->p ("Your kick uncovers a secret door!");
                f->mType = shFeature::kDoorClosed;
                f->mSportingChance = 0;
            } else {
                I->p ("You kick the wall!");
                if (sufferDamage (&KickedWallDamage)) {
                    die (kKilled, "kicking a wall");
                }
            }
            break;
        case shFeature::kDoorBerserkClosed:
        case shFeature::kDoorClosed:
            score += f->mSportingChance;
            f->mSportingChance += RNG (1, 3);
            if (score >= (shFeature::kLocked & f->mDoor ? 22 : 20)) {
                f->mType = shFeature::kDoorOpen;
                f->mDoor &= ~shFeature::kLocked;
                if (f->keyNeededForDoor ()) {
                    I->p ("You smash the lock as you kick the door open!");
                    f->mDoor |= shFeature::kLockBroken;
                } else {
                    I->p ("You kick the door open!");
                }
                f->mSportingChance = 0;
            } else {
                I->p ("The door shudders.  Ow!");
                if (sufferDamage (&KickedWallDamage)) {
                    die (kKilled, "kicking a door");
                }
            }
            break;
            
        case shFeature::kComputerTerminal:
        case shFeature::kPortal: 
        case shFeature::kStairsUp:
        case shFeature::kStairsDown:
        case shFeature::kRadTrap:
        case shFeature::kVat:
        case shFeature::kMaxFeatureType: 
            f->the (buf, 80);
            I->p ("You kick %s!", buf);
            if (sufferDamage (&KickedWallDamage)) {
                die (kKilled, "kicking an obstacle");
            }
        case shFeature::kDoorOpen:
        case shFeature::kDoorBerserkOpen:
        default:
            I->p ("You kick the air!");
            break;

        }
    } else if (mLevel->isObstacle (x, y)) {
        I->p ("You kick the wall!");
        if (sufferDamage (&KickedWallDamage)) {
            die (kKilled, "kicking a wall");
        }       
    } else {
        I->p ("You kick the air!");
    }
    feel (x, y);
    return FULLTURN;
}



/* work out the melee attack against the target
   returns: 1 if the target is eliminated (e.g. it dies, telports away, etc.)
            0 if the target is hit but not eliminated
           -1 if the attack was a complete miss
           -2 if the attacker dies
*/

int
shCreature::resolveMeleeAttack (shObject *weapon,
                                shAttack *attack,
                                shCreature *target)
{
    int attackmod;
    int AC;
    int flatfooted = 0;   /* set to 1 if the target is unable to react to the
                             attack due to unawareness or other disability */
    int cover;
    int vis;

    char n_attacker[64];
    char an_attacker[64];
    char n_target[64];

    int dbonus;
    int dmul;
    int cantsee = 1;

    if (isHero ()) {
        strcpy (n_attacker, "you");
        cantsee = 0;
    } else if (Hero.canSee (this)) {
        the (n_attacker, 64);
        cantsee = 0;
    } else {
        strcpy (n_attacker, "something");
    }
    if (target->isHero ()) {
        strcpy (n_target, "you");
        cantsee = 0;
    } else if (Hero.canSee (target)) {
        target->the (n_target, 64);
        cantsee = 0;
    } else {
        strcpy (n_target, "something");
    }
    an (an_attacker, 64);

    if (target->mConcealment && RNG (100) < target->mConcealment) {
        I->debug ("missed due to concealment");
        goto youmiss;
    }

    vis = canSee (target, &cover);

    if (isFrightened () && vis) {
        if (isHero ()) {
            I->p ("You are too afraid to attack %s!", n_target);
        }
        return -1;
    }
    if (target->isScary () && isHero () &&
        !isBlind () && !isConfused () && !isStunned () &&
        !willSave (target->getPsionicDC (3))) 
    {   /* brain shield doesn't help in this case... */
        I->p ("You are suddenly too afraid to %s %s!", 
              weapon ? "attack" : "touch", n_target);
        makeFrightened (1000 * NDX (2, 10));
        return -1;
    } 

    if (weapon) {
        if (weapon->isMeleeWeapon ()) {
            attackmod = getWeaponSkillModifier (weapon->mIlk);
            attackmod += weapon->mEnhancement;
            attackmod -= 2 * weapon->mDamage;
            dbonus = weapon->mEnhancement;
            dbonus -= weapon->mDamage;
            if (!attack->isSpecialAttack ()) {
                dbonus += ABILITY_MODIFIER (getStr ());
            }
        } else {
            /* improvised melee weapon */
            attackmod = ABILITY_MODIFIER (getStr ()) - 4;
            attackmod -= 2 * weapon->mDamage;
            dbonus = ABILITY_MODIFIER (getStr ()) / 2;
            dbonus -= weapon->mDamage;
        }
    } else {
        /* basic attack */
        attackmod = getWeaponSkillModifier (NULL);
        if (attack->isSecondaryAttack ()) {
            dbonus = ABILITY_MODIFIER (getStr ());
            if (dbonus > 0) {
                dbonus /= 2;
            }
        } else {
            dbonus = ABILITY_MODIFIER (getStr ());
        }
    }

    if (0 == target->canSee (this)) {
        attackmod += 2;
        flatfooted = 1;
    }
    if (target->isStunned ()) {
        flatfooted = 1;
        attackmod += 2;
    }
        
    if (target->isFlankedBy (this)) {
        attackmod += 2;
    }
    if (isProne ()) {
        attackmod -= 4;
    }
    if (target->isAsleep ()) {
        flatfooted = 1;
        attackmod += 8;
    }

    if (attack->isTouchAttack ()) {
        AC = target->getTouchAC (flatfooted, this);
    } else {
        AC = target->getAC (flatfooted, this);
    }

    if (cover >= 100) {
        /* normally it would be impossible to see the target if there is
           100% cover, unless the cover is transparent (e.g. a window) */
        goto youmiss;
    } else {
        AC += CoverACBonus (cover);
    }

    if (VisibilityMiss (vis)) {
        goto youmiss;
    }

    /* roll to hit */

    dmul = attackRoll (attack, weapon, attackmod, AC, target);

    if (dmul <= 0) {
        dmul = -1;
        goto youmiss;
    }

    /* successful hit */

    if (isHero ()) {

        if (target->isMonolith () && attack == mIlk->mAttacks.get (0)) 
        {
            I->p ("You touch the monolith!");
            I->p ("You feel more experienced!");
            Hero.earnXP (-1);
            I->p ("The monolith mysteriously vanishes!");
            target->die (kSuicide);
            return 1;
        }

        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            I->p ("You %s %s!", target->deathVerb (1), n_target);
            if (!target->isHero ()) {
                Hero.earnXP (target->mCLevel);
            }
            target->die (kSlain);
            exerciseWeaponSkill (weapon, 1);
            return 1;
        } else if (dmul > 1) {
            I->p ("You hit %s!", n_target);
        } else {
            I->p ("You hit %s.", n_target);
        }
        exerciseWeaponSkill (weapon, 1);
        target->newEnemy (this);
        target->interrupt ();
        return 0;
    } else if (target->isHero ()) {
        I->p ("%s%s!", n_attacker, monHitsYouMesg (attack));
        if (shAttack::kFaceHug == attack->mType && !RNG (3)) {
            I->p ("The facehugger impregnates you with an alien embryo!");
            Hero.setStoryFlag ("impregnation", 1);
            die (kSlain);
            return -2;
        }
        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            target->die (kSlain, this);
            return 1;
        }
        return 0;
    } else {
        if (!cantsee) {
            I->p ("%s%s %s!", n_attacker, monHitsMonMesg (attack), n_target);
        }
        if (target->sufferDamage (attack, this, dbonus, dmul)) {
            if (!cantsee) {
                I->p ("%s is %s!", n_target, target->deathVerb ());
            }
            target->die (kSlain);
            return 1;
        }
        target->newEnemy (this);
        target->interrupt ();
        return 0;
    }

 youmiss:
    if (target->mHidden <= 0) {
        if (isHero ()) {
            I->p ("You miss %s.", n_target);
        } else if (target->isHero ()) {
            I->p ("%s misses.", n_attacker);
        } else if (!cantsee) {
            I->p ("%s misses %s", n_attacker, n_target);
        }
        target->newEnemy (this);
    }
    target->interrupt ();
    return dmul;
}


/*  work out the result of melee attacking in the given direction
    returns: ms elapsed 
*/
int
shHero::meleeAttack (shObject *weapon, shDirection dir)
{
    int x = mX;
    int y = mY;
    shAttack *attack;  
    shCreature *target;

    if (!Level->moveForward (dir, &x, &y)) {
        /* impossible to attack off the edge of the map? */
        I->p ("There's nothing there!");
        return 0;
    }

    if (NULL == weapon) {
        if (0 == getStoryFlag ("strange weapon message")) {
            I->p ("You start pummeling your foes with your bare hands.");
            setStoryFlag ("strange weapon message", 1);
        }
        attack = mIlk->mAttacks.get (0);
    } else if (weapon->isMeleeWeapon ()) {
        attack = & ((shWeaponIlk *) weapon->mIlk) -> mAttack;
    } else {
        if (0 == getStoryFlag ("strange weapon message")) {
            char buf[80];
            weapon->your (buf, 80);
            I->p ("You start smashing enemies with %s.", buf);
            setStoryFlag ("strange weapon message", 1);
        }
        attack = &ImprovisedObjectAttack;       
    }

    target = Level->getCreature (x, y);
    if (NULL == target) {
        I->p ("You attack thin air!");
    } else {
        resolveMeleeAttack (weapon, attack, target);
    }
    feel (x, y);
    
    return attack->mAttackTime;
}


/*
  RETURNS:  1 if target is eliminated (dies, teleports, etc)
            0 if target was attacked
           -1 if target was missed
           -2 if the attacking monster dies
*/
int
shMonster::meleeAttack (shObject *weapon, shAttack *attack, int x, int y)
{
    shCreature *target;
    char buf[64];

    target = mLevel->getCreature (x, y);
    if (NULL == target) {
        if (Hero.canSee (this)) {
            the (buf, 64);
            I->p ("%s attacks thin air!", buf);
        }
        return -1;
    } 
    if (Hero.isBlind ()) {
        Hero.feel (mX, mY);
    }
    return resolveMeleeAttack (weapon, attack, target);

}


void
shObject::impact (int x, int y, shDirection dir, shCreature *thrower)
{
    if (Level->isObstacle (x, y)) { /* bounce back one square */
        Level->moveForward (uTurn (dir), &x, &y);
    }

    if (isThrownWeapon ()) {
        shAttack *atk = & ((shWeaponIlk *) mIlk) -> mAttack;
        if (shAttack::kSingle != atk->mEffect) { /* a grenade! */
            //I->p ("it explodes!");
            Level->areaEffect (atk, x, y, dir, thrower);
            delete this;
            return;
        }
    } 

    if (sufferDamage (&GroundCollisionAttack, x, y)) {
        if (Hero.canSee (x, y)) {
            char buf[64];
            an (buf, 64);
            I->p ("%s shatters!", buf);
            delete this;
        }
    } else {
        Level->putObject (this, x, y);
    }
}


void
shObject::impact (shCreature *c, shDirection dir, shCreature *thrower)
{
    char cbuf[64], wbuf[64];
    int x, y;
    shAttack *atk;

    the (wbuf, 64);
    c->the (cbuf, 64);
    x = c->mX;
    y = c->mY;
    
    I->p ("%s hits %s.", wbuf, cbuf);

    if (isThrownWeapon ()) {
        atk = & ((shWeaponIlk *) mIlk) -> mAttack;
    } else {
        atk = &ImprovisedMissileAttack;
    }

    if (shAttack::kSingle == atk->mEffect) {
        if (c->sufferDamage (atk, thrower)) {
            if (thrower->isHero () && !c->isHero ()) {
                I->p ("%s is %s!", c, c->deathVerb ());
                Hero.earnXP (c->mCLevel);
            }
            c->die (kSlain);
        } else {
            if (thrower->isHero () ||
                thrower->isPet ())
            {
                c->newEnemy (thrower);
            }
            c->interrupt ();
        }
        Level->putObject (this, x, y);
    } else { /* a grenade */
        //I->p ("it explodes!");
        Level->areaEffect (atk, c->mX, c->mY, dir, thrower);
        delete this;
    }

}


void
shObject::impact (shFeature *f, shDirection dir, shCreature *thrower)
{
    int x = f->mX;
    int y = f->mY;

    if (isThrownWeapon ()) {
        shAttack *atk = & ((shWeaponIlk *) mIlk) -> mAttack;
        if (shAttack::kSingle != atk->mEffect) { /* a grenade! */
            //I->p ("it explodes!");
            if (f->isObstacle () && atk->mRadius > 0) {
                /* bounce back one square */
                Level->moveForward (uTurn (dir), &x, &y);
            }
            Level->areaEffect (atk, x, y, dir, thrower);
            delete this;
            return;
        }
    } 
    if (sufferDamage (&GroundCollisionAttack, x, y)) {
        if (Hero.canSee (x, y)) {
            char buf[64];
            an (buf, 64);
            I->p ("%s shatters!", buf);
            delete this;
        }
    } else {
        if (f->isObstacle ()) {
            /* bounce back one square */
            Level->moveForward (uTurn (dir), &x, &y);
        }
        Level->putObject (this, x, y);
    }

}


/* returns: 1 if the feature should block further effect
            0 otherwise
 */
int
shMapLevel::areaEffectFeature (shAttack *atk, int x, int y,
                               shCreature *attacker)
{
    shFeature *f = getFeature (x, y);
    if (!f) return 0;
    int destroy = 0;
    int block = 1;

    switch (f->mType) {
    case shFeature::kDoorClosed:
        switch (atk->mType) {
        case shAttack::kDisintegrationRay:
            if (Hero.canSee (x, y)) {
                I->p ("The door is annihilated!"); 
            } else {
                I->p ("You hear a loud bang!");
            }
            destroy = 1; 
            break;
        case shAttack::kGaussRay:
            f->mDoor &= ~shFeature::kAutomatic;
            f->mDoor &= ~shFeature::kBerserk;
            if (f->isLockedDoor ()) {
                f->mDoor &= ~shFeature::kLocked;
                f->mDoor |= shFeature::kLockBroken;
            }
            block = 0;
            break;
        case shAttack::kHeatRay:
            if (Hero.canSee (x, y)) {
                I->p ("The heat ray melts a hole through the door!");
            }
            destroy = 1;
            break;
        case shAttack::kGammaRay: /* no message */
        case shAttack::kTransporterRay:
        case shAttack::kStasisRay:
        case shAttack::kHealingRay:
        case shAttack::kRestorationRay:
        case shAttack::kFlash:
            break;
        case shAttack::kPoisonRay:
        case shAttack::kFreezeRay:
        default:
            if (Hero.canSee (x, y)) {
                I->p ("The door absorbs the %s.", atk->noun ());
            }
            break;
        }
        break;
    case shFeature::kDoorHiddenVert:
    case shFeature::kDoorHiddenHoriz:
        switch (atk->mType) {
        case shAttack::kDisintegrationRay:
            if (Hero.canSee (x, y)) {
                I->p ("The wall is annihilated!"); 
            } else {
                I->p ("You hear a loud bang!");
            }
            destroy = 1; 
            break;
        case shAttack::kGaussRay:
            if (f->isLockedDoor ()) {
                f->mDoor &= ~shFeature::kLocked;
                f->mDoor |= shFeature::kLockBroken;
            }
            block = 0;
            break;
        case shAttack::kHeatRay:
            if (Hero.canSee (x, y)) {
                I->p ("The heat ray melts a hole through a secret door!");
            }
            destroy = 1;
            break;
        case shAttack::kPoisonRay:
        case shAttack::kFreezeRay:
        case shAttack::kGammaRay:       
        case shAttack::kStasisRay:
        case shAttack::kHealingRay:     
        case shAttack::kRestorationRay:
        default:
            /* silently absorbed */
            break;
        }
        break;
    default:
        /* no effect */
        block = 0;
        break;
    }

    if (destroy) {
        Level->removeFeature (f);
        Level->computeVisibility ();
    }
    
    return block;
}


void
shMapLevel::areaEffectObjects (shAttack *atk, int x, int y, 
                               shCreature *attacker)
{

}


/* returns: -2 if attacker dies, 
             0 if effect should keep going, 
             1 if effect should stop
 */

int
shMapLevel::areaEffectCreature (shAttack *atk, int x, int y, 
                               shCreature *attacker)
{
    const int buflen = 64;
    char buf[buflen];
    const char *msg = beamHitsMesg (atk);
    int died = 0;
    int block = 0;
    int dc = 15;
    int divider = 1;
    shCauseOfDeath howdead = kSlain;

    shCreature *c = getCreature (x, y);
    if (!c) return 0;
    c->the (buf, buflen);

    if (Hero.canSee (x, y)) {
        drawSq (x, y);
    }
    I->cursorOnHero ();
    
    if (atk->mType == shAttack::kTransporterRay) {
        int self = c == attacker;
        if (!self) c->newEnemy (attacker);
        died = c->transport (-1, -1, 100);
        if (1 == died && self) return -2;
        return 0;
    } 

    if (attacker) {
        dc += attacker->mBAB;
    } 

    if (c->hasShield () && 
        c->countEnergy ()) 
    { /* the shield is big, so it's always hit */
        block = 1;
    } else if (atk->mType == shAttack::kFlash) {
        /* no save possible */
    } else if (c != attacker && c->reflexSave (atk, dc)) {
        msg = NULL;
        if (atk->mType == shAttack::kBlast) {
            /* save for half damage */
            ++divider;
            if (c->isHero ()) {
                I->p ("You duck some of the blast.");
            } else {
                I->p ("%s ducks some of the blast.", buf);
            }
        } else {
            /* save for no damage */
            if (c->isHero ()) {
                I->p ("You dodge the %s", atk->noun ());
            } else {
                if (Hero.canSee (c)) {
                    I->p ("%s dodges the %s.", buf, atk->noun ());
                }
                if (attacker && 
                    (attacker->isHero () || attacker->isPet ()))
                {
                    c->newEnemy (attacker);
                }
            }
            return 0;
        }
    }

    if (c->isHero ()) {
        if (msg) I->p ("%s you.", msg);
        if (Hero.sufferDamage (atk)) {
            if (attacker == c) {
                died = -2;
            }
            if (atk->mType == shAttack::kDisintegrationRay) {
                c->die (kAnnihilated, "a disintegration ray");
                block = 1;
            } else {
                char deathbuf[40];
                snprintf (deathbuf, 40, "a %s", atk->noun ());
                c->die (kSlain, deathbuf);
            }
        }
    } else {
        if (Hero.canSee (c)) {
            if (msg) I->p ("%s %s.", beamHitsMesg (atk), buf);
        } else {
        }
        if (c->sufferDamage (atk)) {
            if (attacker && attacker->isHero () && !c->isHero ()) {
                Hero.earnXP (c->mCLevel);
            }
            if (Hero.canSee (c)) {
                if (shAttack::kDisintegrationRay == atk->mType) {
                    howdead = kAnnihilated;
                }
                I->p ("%s is %s!", buf,
                      shAttack::kDisintegrationRay == atk->mType ? 
                      "annihilated" :
                      (shAttack::kGaussRay == atk->mType && c->isRobot ()) ?
                      /* now robots are always "disabled" instead of dying, 
                         but just in case I change it back to "destroyed"... */
                      "disabled" : 
                      c->deathVerb ());
            }
            if (attacker == c) {
                died = -2;
            }
            c->die (howdead);
        } else {
            if (attacker && 
                (attacker->isHero () || attacker->isPet ()))
            {
                c->newEnemy (attacker);
            }
            c->interrupt ();
        }
        if (shAttack::kDisintegrationRay == atk->mType) {
            block = 1;
        }
    }
    //I->smallPause ();

    usleep (50000);
    drawSpecialEffect (x, y, beamSpecialEffect (atk));
    return died ? -2 : block;
}


/* returns -2 if attacker dies */
int
shMapLevel::areaEffect (shAttack *atk, int x, int y, 
                        shDirection dir, shCreature *attacker)
{
    int u, v;
    int i;
    int died = 0;

    /* General rule: affect any features first, then creatures,
       because the death of a monster might cause an automatic door to
       close and get blown up, when it shouldn't have been affected.
    */

    switch (atk->mEffect) {
    case shAttack::kSingle: /*something is wrong */
        abort ();
    case shAttack::kBeam:
        for (i = atk->mRange + RNG (1, 6); i > 0; i--) {
            if (!moveForward (dir, &x, &y)) {
                break;
            }
            if (kUp == dir && attacker->isHero ()) {
                I->p ("The ceiling absorbs the %s.", atk->noun ());
                break;
            } 
            if (!isFloor (x, y)) {
                break;
            }
            drawSpecialEffect (x, y, beamSpecialEffect (atk));
            I->refreshScreen ();
            if (areaEffectFeature (atk, x, y, attacker)) {
                break;
            }

            if (kDown != dir && 
                -2 == areaEffectCreature (atk, x, y, attacker)) 
            {
                died = -2;
            }
            areaEffectObjects (atk, x, y, attacker);
            if (kDown == dir && 
                shAttack::kDisintegrationRay == atk->mType) 
            {
                if (!Level->getFeature (x, y)) {
                    Level->addTrap (x, y, shFeature::kHole);
                    Level->checkTraps (x, y, 100);
                }
            }
            if (kOrigin == dir || kNoDirection == dir || 
                kUp == dir || kDown == dir)
            {
                break;
            }
        }
        usleep (100000);
        return died;
    case shAttack::kCone:
    case shAttack::kSpread:
        //TODO
        return 0;
    case shAttack::kBurst:
        for (u = x - atk->mRadius; u <= x + atk->mRadius; u++) {
            for (v = y - atk->mRadius; v <= y + atk->mRadius; v++) {
                if (distance (u, v, x, y) <= 5 * atk->mRadius + 2
                    && isFloor (u,v) 
                    && existsLOS (x, y, u, v)) 
                {
                    if (isInLOS (u, v)) {
                        drawSpecialEffect (u, v,
                                           shAttack::kFlash == atk->mType ? kRadiationEffect 
                                                                          : kExplosionEffect);
                    }
                    areaEffectFeature (atk, u, v, attacker);
                }
            }
        }
        I->refreshScreen ();
        for (u = x - atk->mRadius; u <= x + atk->mRadius; u++) {
            for (v = y - atk->mRadius; v <= y + atk->mRadius; v++) {
                if (distance (u, v, x, y) <= 5 * atk->mRadius + 2
                    && existsLOS (x, y, u, v)) 
                {
                    if (-2 == areaEffectCreature (atk, u, v, attacker)) {
                        died = -2;
                    }
                    areaEffectObjects (atk, u, v, attacker);
                }
            }
        }
        usleep (100000);
        return died;
    }
    return 0;
}
