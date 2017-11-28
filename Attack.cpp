#include <stdlib.h>

#include "Global.h"
#include "Attack.h"
#include "Creature.h"
#include "Object.h"

static const char *AttackNouns[shAttack::kMaxAttackType] =
{ 
    "nothing",
    "bite",
    "blast",
    "bullet",
    "punch",
    "claw",
    "club",
    "kick",
    "laser beam",
    "quill",
    "slash",
    "stab",
    "zap"
};

static const char *SecondPersonAttackVerbs[shAttack::kMaxAttackType] =
{
    "don't attack",
    "bite",
    "blast",
    "shoot",
    "punch",
    "claw",
    "hit",
    "kick",
    "shoot",
    "shoot a quill at",
    "hit",
    "hit",
    "zap"
};

#if 0
static char *ThirdPersonAttackVerbs[shAttack::kMaxAttackType] =
{
    "doesn't attack",
    "bites",
    "blasts",
    "shoots",
    "punches",
    "claws",
    "hits",
    "kicks",
    "shoots",
    "shoots a quill at",
    "hits",
    "hits",
    "zaps"
};
#endif


const char *
shAttack::noun ()
{
    switch (mType) {
    case kNoAttack: return "non-attack";
    case kAcidBath: return "acid bath";
    case kAttach: return "attachment";
    case kBite: return "bite";
    case kBlast: return "explosion";
    case kBullet: return "bullet";
    case kBolt: return "bolt";
    case kBreatheBugs: return "cloud of bugs";
    case kBreatheFire: return "fiery breath";
    case kBreatheTime: return "time warp";
    case kBreatheTraffic: return "packet storm";
    case kBreatheViruses: return "cloud of viruses";
    case kClaw: return "claw";
    case kClub: return "club";
    case kChoke: return "choke";
    case kCook: return "cooker"; /* ugh */
    case kCrush: return "grasp";
    case kCut: return "cut";
    case kDisintegrationRay: return "disintegration ray";
    case kExplode: return "explosion";
    case kExtractBrain: return "extraction";
    case kFaceHug: return "grasp";
    case kFlash: return "flash";
    case kFreezeRay: return "freeze ray";
    case kGammaRay: return "gamma ray";
    case kGaussRay: return "gauss ray";
    case kHeadButt: return "head butt";
    case kHealingRay: return "healing ray";
    case kHeatRay: return "heat ray";
    case kKick: return "kick";
    case kLightningBolt: return "lightning bolt";
    case kLaser: return "laser beam";
    case kMentalBlast: return "mental blast";
    case kPoisonRay: return "poison ray";
    case kPunch: return "punch";
    case kRestorationRay: return "restoration ray";
    case kQuill: return "quill";
    case kShot: return "shot";
    case kSlash: return "slash";
    case kSlime: return "slime";
    case kSmash: return "smacks";
    case kSpawnPrograms: return "data burst";
    case kStab: return "stab";
    case kStasisRay: return "stasis ray";
    case kSting: return "sting";
    case kSword: return "sword";
    case kTailSlap: return "tail";
    case kTouch: return "touch";
    case kZap: return "zap";
    }
    return "attack";
}       




int
shAttack::verb2p (char *buf, int len)
{
    return snprintf (buf, len, "%s", SecondPersonAttackVerbs[mType]);
};


int
shAttack::verb3p (char *buf, int len)
{
    return snprintf (buf, len, "%s", AttackNouns[mType]);
};


int 
shAttack::getThreatRange (shObject *weapon, shCreature *target) 
{
    return weapon ? weapon->getThreatRange (target) : 20;
}

int 
shAttack::getCriticalMultiplier (shObject *weapon, shCreature *target) 
{
    return weapon ? weapon->getCriticalMultiplier () : 2;
}


shAttack ImprovisedObjectAttack =
    shAttack (NULL, shAttack::kSmash, shAttack::kSingle, kMelee,
              kConcussive, 1, 3);

shAttack ImprovisedMissileAttack =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kSingle, kMissile, 
              kNoEnergy, 1, 2);

shAttack KickedFootballAttack =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kSingle, kMissile,
              kNoEnergy, 1, 6);

shAttack GroundCollisionAttack =
    shAttack (NULL, shAttack::kSmash, shAttack::kOther, kMissile,
              kConcussive, 1, 6);

shAttack PitTrapDamage =
    shAttack (NULL, shAttack::kSmash, shAttack::kSingle, 0, 
              kConcussive, 1, 6);

shAttack AcidPitTrapDamage =
    shAttack (NULL, shAttack::kAcidBath, shAttack::kSingle, 0, 
              kCorrosive, 1, 4);

shAttack AcidBloodAttack = 
    shAttack (NULL, shAttack::kAcidSplash, shAttack::kSingle, 0, 
              kCorrosive, 2, 6);

shAttack ExplodingMonsterAttack =
    shAttack (NULL, shAttack::kBlast, shAttack::kBurst, 0,
              kConcussive, 3, 6);

shAttack BasicExplodingMonsterAttack =
    shAttack (NULL, shAttack::kBlast, shAttack::kBurst, 0,
              kConcussive, 3, 6);
