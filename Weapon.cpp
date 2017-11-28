#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Creature.h"
#include <math.h>
shVector <shObjectIlk *> WeaponIlks;
//shVector <shWeaponIlk *> WeaponsGroupedByType[kMaxWeaponType];

void
initializeWeapons ()
{
    ImprovisedMissileAttack.mRange = 5; 

    new shWeaponIlk ("elven dagger", "dagger", "wooden dagger", 
                     kCyan, kMeleeWeapon, NULL, kSteel,
                     kMelee | kMissile, 400, kTiny, 10, 1, 
                     NULL, 0, 0,
                     shAttack::kStab, shAttack::kSingle, 10, 0, kPiercing, 
                     1, 4, FULLTURN, 
                     kNoEnergy, 0, 10 BZ, 5);
    
    new shWeaponIlk ("anal probe", "anal probe", "anal probe", 
                     kGray, kMeleeWeapon, NULL, kSteel,
                     kIdentified | kMelee, 400, kTiny, 10, 1, 
                     NULL, 0, 0,
                     shAttack::kAnalProbe, shAttack::kSingle, 10, 0, kViolating, 
                     10, 10, FULLTURN, 
                     kNoEnergy, 0, 10 BZ, 0);
    
    new shWeaponIlk ("club", "club", "club", 
                     kBrown, kMeleeWeapon, NULL, kWood,
                     kIdentified | kMelee, 1200, kSmall, 5, 10, 
                     NULL, 0, 0,
                     shAttack::kClub, shAttack::kSingle, 0, 0, kConcussive, 
                     1, 6, FULLTURN, 
                     kNoEnergy, 0, 1 BZ, 5);

    new shWeaponIlk ("cattle prod", "cattle prod", "cattle prod", 
                     kGray, kMeleeWeapon, NULL, kSteel,
                     kMelee, 400, kSmall, 10, 2,
                     &EnergyCellIlk, 5, 0,
                     shAttack::kZap, shAttack::kSingle, 0, 0, kElectrical,
                     1, 4, FULLTURN, 
                     kStunning, 1, 50 BZ, 5);

    new shWeaponIlk ("mop", "mop", "mop",
                     kBrightGreen, kMeleeWeapon, NULL, kWood, 
                     kIdentified | kMelee, 900, kMedium, 5, 10, 
                     NULL, 0, 0,
                     shAttack::kSmash, shAttack::kSingle, 0, 0, kConcussive,
                     1, 8, FULLTURN, 
                     kNoEnergy, 0, 5 BZ, 5); 

    new shWeaponIlk ("pair of nunchucks", "pair of nunchucks", 
                     "pair of nunchucks",
                     kYellow, kMeleeWeapon, NULL, kWood,
                     kIdentified | kMelee, 500, kMedium, 10, 10, 
                     NULL, 0, 0,
                     shAttack::kClub, shAttack::kSingle, 0, 0, kConcussive,
                     2, 4, FULLTURN,
                     kNoEnergy, 0, 200 BZ, 10);

    new shWeaponIlk ("chainsaw", "chainsaw", "chainsaw",
                     kBrightYellow, kMeleeWeapon, NULL, kSteel, 
                     kIdentified | kMelee, 1200, kMedium, 10, 10, 
                     &EnergyCellIlk, 0, -2,
                     shAttack::kCut, shAttack::kSingle, 0, 0, kSlashing,
                     2, 6, FULLTURN, 
                     kNoEnergy, 0, 500 BZ, 5); 

    new shWeaponIlk ("katana", "sword", "antique sword", 
                     kGray, kSword, NULL, kSteel,
                     kMelee, 600, kSmall, 10, 2,
                     NULL, 0, 0,
                     shAttack::kSword, shAttack::kSingle, 0, 0, kSlashing,
                     1, 10, FULLTURN, 
                     kNoEnergy, 0, 1000 BZ, 10);

    new shWeaponIlk ("bat'leth", "bat'leth", "bat'leth",
                     kGray, kMeleeWeapon, NULL, kSteel,
                     kMelee, 1000, kLarge, 20, 2, 
                     NULL, 0, -1,
                     shAttack::kSlash, shAttack::kSingle, 0, 0, kSlashing,
                     2, 6, FULLTURN,
                     kNoEnergy, 0, 1200 BZ, 10);

    shWeaponIlk *lightsaber = 
    new shWeaponIlk ("light saber", "energy sword", "energy sword", 
                     kRed, kSword, NULL, kTitanium,
                     kMelee, 600, kMedium, 30, 2,
                     NULL, 1, 1,
                     shAttack::kSlash, shAttack::kSingle, 0, 0, 
                     kSlashing, 2, 6, FULLTURN, 
                     kBurning, 1, 
                     1500 BZ, 10);
    lightsaber->mWieldedIntrinsics |= kReflection;


    shWeaponIlk *grenade =
    new shWeaponIlk ("grenade", "grenade", "grenade", 
                     kCyan, kGrenade, NULL, kSteel,
                     kMissile | kMergeable | kTouchAttack, 300, kTiny, 5, 1, 
                     NULL, 0, 0,
                     shAttack::kBlast, shAttack::kBurst, 10, 1, kConcussive, 
                     3, 6, FULLTURN, 
                     kNoEnergy, 0, 25 BZ, 0);

    new shWeaponIlk ("concussion grenade", "grenade", "grenade", 
                     kCyan, kGrenade, grenade, kSteel,
                     kMissile | kMergeable | kTouchAttack, 300, kTiny, 5, 1, 
                     NULL, 0, 0,
                     shAttack::kBlast, shAttack::kBurst, 10, 1, kConcussive, 
                     3, 6, FULLTURN, 
                     kNoEnergy, 0, 25 BZ, 100);


    new shWeaponIlk ("frag grenade", "grenade", "grenade", 
                     kCyan, kGrenade, grenade, kSteel,
                     kMissile | kMergeable | kTouchAttack, 400, kTiny, 5, 1, 
                     NULL, 0, 0,
                     shAttack::kBlast, shAttack::kBurst, 10, 1, kConcussive, 
                     2, 6, FULLTURN, 
                     kPiercing, 2, 25 BZ, 50);

    new shWeaponIlk ("stun grenade", "grenade", "grenade",
                     kCyan, kGrenade, grenade, kSteel,
                     kMissile | kMergeable | kTouchAttack, 400, kTiny, 5, 1, 
                     NULL, 0, 0,
                     shAttack::kBlast, shAttack::kBurst, 10, 1, kStunning, 
                     3, 6, FULLTURN, 
                     kNoEnergy, 0, 25 BZ, 50);

    new shWeaponIlk ("rad grenade", "grenade", "grenade",
                     kCyan, kGrenade, grenade, kSteel,
                     kMissile | kMergeable | kTouchAttack, 400, kTiny, 5, 1, 
                     NULL, 0, 0,
                     shAttack::kFlash, shAttack::kBurst, 10, 3, kRadiological, 
                     4, 8, FULLTURN, 
                     kNoEnergy, 0, 25, 25);

    new shWeaponIlk ("football", "grenade", "prolate spheroid",
                     kBrown, kGrenade, NULL, kLeather,
                     kMissile, 400, kSmall, 3, 1,
                     NULL, 0, 0,
                     shAttack::kImpact, shAttack::kSingle, 10, 9, kConcussive, 
                     1, 6, FULLTURN,
                     kNoEnergy, 0, 25 BZ, 5);

    shWeaponIlk *shell =
    new shWeaponIlk ("shotgun shell", "shotgun shell", "shotgun shell", 
                     kYellow, kNoSkillCode, NULL, kLead,
                     kAmmo | kMergeable, 32, kFine, 1, 1, NULL, 0, 0,
                     shAttack::kShot, shAttack::kSingle, 0, 0,
                     kConcussive, 4, 6, FULLTURN,
                     kNoEnergy, 0, 1 BZ, 80);
        
    shWeaponIlk *bullet =
    new shWeaponIlk ("bullet", "bullet", "bullet", kYellow, kNoSkillCode, NULL, kSteel,
                     kAmmo | kMergeable, 10, kFine, 1, 1, NULL, 0, 0,
                     shAttack::kBullet, shAttack::kSingle, 0, 0,
                     kPiercing, 1, 8, FULLTURN,
                     kNoEnergy, 0, 1 BZ, 300);

    shWeaponIlk *rail =
    new shWeaponIlk ("railgun slug", "slug", "slug", 
                     kRed, kNoSkillCode, NULL, kDepletedUranium,
                     kAmmo | kMergeable, 100, kFine, 1, 1, NULL, 0, 0,
                     shAttack::kRail, shAttack::kSingle, 0, 0,
                     kPiercing, 3, 10, FULLTURN,
                     kNoEnergy, 0, 1 BZ, 25);

    new shWeaponIlk ("pea shooter", "energy pistol", "wimpy pistol", 
                     kCyan, kHandgun, NULL, kSteel,
                     kAimed, 800, kMedium, 10, 2, NULL, 0, 0,
                     shAttack::kBolt, shAttack::kSingle, 30, 0,
                     kForce, 1, 4, FULLTURN,
                     kNoEnergy, 0, 50 BZ, 50);

    new shWeaponIlk ("laser pistol", "energy pistol", "laser pistol", 
                     kCyan, kHandgun, NULL, kSteel,
                     kAimed | kIdentified , 800, kMedium, 10, 2, &EnergyCellIlk, 5, -1,
                     shAttack::kLaser, shAttack::kBeam, 30, 0, 
                     kLaser, 1, 8, FULLTURN,
                     kBlinding, 2, 50 BZ, 25);

    new shWeaponIlk ("blaster", "energy pistol", "blaster", 
                     kCyan, kHandgun, NULL, kSteel,
                     kAimed | kIdentified, 800, kMedium, 10, 2, &EnergyCellIlk, 5, -2,
                     shAttack::kBolt, shAttack::kSingle, 30, 0, 
                     kForce, 1, 8, FULLTURN,
                     kNoEnergy, 0, 50 BZ, 25);

    new shWeaponIlk ("phaser", "energy pistol", "ergonomic energy pistol", 
                     kCyan, kHandgun, NULL, kSteel,
                     kAimed | kIdentified, 800, kMedium, 10, 2, &EnergyCellIlk, 5, 1,
                     shAttack::kLaser, shAttack::kBeam, 30, 0, 
                     kLaser, 1, 8, FULLTURN,
                     kBlinding, 2, 50 BZ, 15);

    new shWeaponIlk ("laser rifle", "laser rifle", "laser rifle", 
                     kCyan, kLightGun, NULL, kSteel,
                     kAimed | kIdentified, 1200, kLarge, 10, 3, &EnergyCellIlk, 8, 1,
                     shAttack::kLaser, shAttack::kBeam, 60, 0,
                     kLaser, 1, 12, FULLTURN,
                     kBlinding, 3, 200 BZ, 75);

    new shWeaponIlk ("pistol", "pistol", "pistol", 
                     kBlue, kHandgun, NULL, kSteel,
                     kAimed, 800, kMedium, 10, 2, bullet, 1, -1,
                     shAttack::kBullet, shAttack::kSingle, 60, 0,
                     kPiercing, 1, 8, FULLTURN,
                     kNoEnergy, 0, 100 BZ, 25);

    new shWeaponIlk ("assault pistol", "pistol", "assault pistol",
                     kBlue, kHandgun, NULL, kSteel,
                     kAimed, 800, kMedium, 10, 2, bullet, 2, -1,
                     shAttack::kBullet, shAttack::kSingle, 60, 0,
                     kPiercing, 1, 8, FULLTURN,
                     kNoEnergy, 0, 100 BZ, 10);

    new shWeaponIlk ("shotgun", "shotgun", "shotgun", 
                     kBlue, kLightGun, NULL, kSteel,
                     kAimed, 1200, kLarge, 10, 3, shell, 1, 1,
                     shAttack::kShot, shAttack::kSingle, 10, 0,
                     kConcussive, 4, 6, FULLTURN,
                     kNoEnergy, 0, 150 BZ, 25);

    new shWeaponIlk ("sniper rifle", "rifle", "telescoped rifle", 
                     kBlue, kLightGun, NULL, kSteel,
                     kAimed, 1200, kMedium, 10, 2, bullet, 1, 3,
                     shAttack::kBullet, shAttack::kSingle, 180, 0,
                     kPiercing, 1, 10, FULLTURN,
                     kNoEnergy, 0, 300 BZ, 25);

    new shWeaponIlk ("pulse rifle", "rifle", "assault rifle", 
                     kBlue, kHeavyGun, NULL, kSteel,
                     kAimed | kSelectiveFire, 3000, kLarge, 10, 3, bullet, 3, 0, 
                     shAttack::kBullet, shAttack::kSingle, 60, 0, 
                     kPiercing, 1, 8, FULLTURN,
                     kNoEnergy, 0, 400 BZ, 20);

/*
    new shWeaponIlk ("boltgun", "pistol", "assault pistol",
                     kCyan, kHeavyGun, NULL, kSteel,
                     kAimed, 1000, kMedium, 10, 3, bullet, 3, 0,
                     shAttack::kBullet, shAttack::kSingle, 60, 0,
                     kPiercing, 1, 10, FULLTURN,
                     kNoEnergy, 0, 500, 25);
*/
    new shWeaponIlk ("railgun", "railgun", "railgun",
                     kRed, kHeavyGun, NULL, kSteel,
                     kIdentified | kAimed, 2000, kMedium, 10, 3, rail, 1, 0,
                     shAttack::kRail, shAttack::kSingle, 180, 0,
                     kPiercing, 3, 10, SLOWTURN,
                     kNoEnergy, 0, 800 BZ, 15);


    new shWeaponIlk ("chaingun", "heavy cannon", "heavy cannon", 
                     kYellow, kHeavyGun, NULL, kSteel,
                     kIdentified | kAimed, 3000, kLarge, 15, 5, bullet, 6, 0,
                     shAttack::kBullet, shAttack::kSingle, 60, 0,
                     kPiercing, 1, 10, FULLTURN,
                     kNoEnergy, 0, 800 BZ, 10);

    new shWeaponIlk ("laser cannon", "laser cannon", "laser cannon", 
                     kCyan, kHeavyGun, NULL, kSteel,
                     kAimed, 3000, kLarge, 15, 5, &EnergyCellIlk, 35, 1,
                     shAttack::kLaser, shAttack::kBeam, 60, 0,
                     kLaser, 4, 12, FULLTURN,
                     kBlinding, 1, 800 BZ, 10);
/*
    new shWeaponIlk ("plasma cannon", "heavy cannon", "heavy cannon", 
                     kBlue, NULL, kSteel,
                     kAimed, 5000, kLarge, 15, 5, &EnergyCellIlk, 40, 0,
                     shAttack::kZap, shAttack::kBeam, 0, 6,
                     kBurning, 4, 10, 0, 2000, 2000,
                     kNoEnergy, 0, 800, 10);
*/
/*
    new shWeaponIlk ("rock", "rock", "rock", kYellow, NULL, kRock, 
                     kMissile | kMergeable, 200, kTiny, 40, 1, NULL, 0, -1,
                     shAttack::kSmash, shAttack::kSingle, 10, 0,
                     kConcussive, 1, 3, 0, 800, 0, 
                     kNoEnergy, 0, 0, 30);
*/
}







shWeaponIlk::shWeaponIlk (const char *name, 
                          const char *vaguename,
                          const char *appearance, 
                          shColor color,
                          shSkillCode skill,
                          shWeaponIlk *parent,
                          shMaterialType material,
                          int flags,
                          int weight,
                          shThingSize size,
                          int hardness, 
                          int hp, 
                          shObjectIlk *ammo,
                          int ammoburst,
                          int hitbonus,
                          shAttack::Type atktype,
                          shAttack::Effect atkeffect,
                          int range,
                          int radius,
                          shEnergyType entype, 
                          int numdice,
                          int dicesides,
                          int atktime,
                          shEnergyType entype2,
                          int numdice2,
                          int cost,
                          int prob)
{
    WeaponIlks.add (this);

    mType = kAmmo & flags ? kProjectile : kWeapon;
    mParent = parent;
    mName = name;
    mVagueName = vaguename;
    mAppearance = appearance;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[color];
    mCost = cost;
    mMaterial = material;
    mFlags = flags;
    if (!(flags & kChargeable)) {
        mFlags |= kEnhanceable;
    }
    if (kAmmo & flags) {
        mFlags |= kBugProof;
        mFlags &= ~kEnhanceable;
    }
    mProbability = prob;
    mWeight = weight;
    mSize = size;
    mHardness = hardness;
    mHP = hp;
    mRange = range;
    mToHitBonus = hitbonus;
    mAmmoType = ammo;
    mAmmoBurst = ammoburst;
    mSkill = skill;
    mAttack.mType = atktype;
    mAttack.mEffect = atkeffect;
    mAttack.mRange = range;
    mAttack.mRadius = radius;
    mAttack.mFlags = flags;
    mAttack.mProb = 1;
    mAttack.mAttackTime = atktime;
    mAttack.mDamage[0].mEnergy = entype;
    mAttack.mDamage[0].mNumDice = numdice;
    mAttack.mDamage[0].mDieSides = dicesides;
    mAttack.mDamage[1].mEnergy = entype2;
    mAttack.mDamage[1].mNumDice = numdice2; 
    mAttack.mDamage[1].mDieSides = 6;

    mThreatRange = 20;
    mCriticalMultiplier = 2;
}


int
shObject::isAmmo (shObject *weapon)
{
    if (NULL == weapon) {
        return kAmmo & mIlk->mFlags;
    } else {
        shWeaponIlk *ilk = (shWeaponIlk *) weapon->mIlk;
        shObjectIlk *ammo = ilk->mAmmoType;
        return isA (ammo);
    }
}


/* returns: 1 if the creature has enough ammo to shoot the weapon once,
              or if the weapon doesn't need ammo; 0 o/w
 */
int
shCreature::hasAmmo (shObject *weapon)
{
    shWeaponIlk *ilk = (shWeaponIlk *) weapon->mIlk;
    shObjectIlk *ammo = weapon->isA (kRayGun) ? NULL : ilk->mAmmoType;
    int i;
    int n = 0;
    
    if (NULL == ammo) {
        if (weapon->isChargeable ()) { 
            if (weapon->mCharges) {
                return 1;
            } else {
                if (isHero ()) {
                    weapon->setChargeKnown ();
                }
                return 0;
            }
        }
        return 1;
    }
    n = ilk->mAmmoBurst;
    if (&EnergyCellIlk == ammo) {
        return countEnergy () >= n ? 1 : 0;
    }
    for (i = 0; n > 0 && i < mInventory->count (); i++) {
        shObject *obj = mInventory->get (i);
        if (obj->isA (ammo)) {
            n -= obj->mCount;
        }
    }
    return n <= 0 ? 1 : 0;
}    

    
//RETURNS: number of rounds taken on success; 0 if no ammo available;
//         -1 if none were needed
//         For weapons that use energy cells, returns 1 or 0 depending on
//          whether or not enough cells were consumed

int
shCreature::expendAmmo (shObject *weapon, int cnt /* = 0 */ )
{
    shWeaponIlk *ilk = (shWeaponIlk *) weapon->mIlk;
    shObjectIlk *ammo = ilk->mAmmoType;
    int i;
    int n = 0;
    
    if (0 == cnt) {
        if (weapon->isSelectiveFireWeapon () && !weapon->isToggled ()) {
            cnt = 1;
        } else {
            cnt = ilk->mAmmoBurst;
        }
    }
    if (weapon->isChargeable ()) {
        if (weapon->mCharges > cnt) {
            weapon->mCharges -= cnt;
        } else {
            cnt = weapon->mCharges;
            weapon->mCharges = 0;
        }
        return cnt;
    } else if (NULL == ammo) {
        return -1;
    }
    if (&EnergyCellIlk == ammo) {
        if (countEnergy () >= cnt) {
            loseEnergy (cnt);
            return 1;
        } else {
            return 0;
        }
    }

    for (i = 0; n < cnt && i < mInventory->count (); i++) {
        shObject *obj = mInventory->get (i);
        if (obj->isA (ammo)) {
            int x = useUpSomeObjectsFromInventory (obj, cnt - n);
            if (x) {
                n += x;
                --i;
            }
        }
    }
    return n;
}


int 
selectWeaponFireMode (shObject *gun)
{
    if (gun->isToggled ()) {
        I->p ("You switch your weapon to single fire mode.");
        gun->resetToggled ();
    } else {
        I->p ("You switch your weapon to burst mode.");
        gun->setToggled ();
    }
    return HALFTURN;
}



/* RETURNS: 

*/

shObject *
createWeapon (char *desc /* = NULL */,
              int count /* = -22 */, int bugginess /* = -2 */, 
              int enhancement /* = -22 */, int charges /* = -22 */)
{
    shWeaponIlk *ilk;
    shObject *weapon;

    ilk = (shWeaponIlk *) (NULL == desc ? pickAnIlk (&WeaponIlks) 
                                        : findAnIlk (&WeaponIlks, desc));
    if (NULL == ilk) return NULL;

    weapon = new shObject ();
    weapon->mIlk = ilk;

    if (weapon->isA (kProjectile)) {
        weapon->identify ();
        if (-22 != count) {
            weapon->mCount = count;
        } else {
            int dice = (int) (18.0 / (double) sqrt ((double) ilk->mWeight));
            weapon->mCount = NDX (dice, 10);
        }
    } else if (weapon->getIlkFlag (kMissile)) {
        weapon->mCount = -22 == count ? RNG (1, 4) : count;
    } else {
        weapon->mCount = 1;
    }

    weapon->mHP = ilk->mHP;

    if (-2 == bugginess) {
        int tmp = RNG (8);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
    }
    weapon->setBugginess (bugginess);

    weapon->mEnhancement = -22 == enhancement ? 0 : enhancement;

    if (weapon->isChargeable ()) {
        if (weapon->isA ("empty ray gun")) {
            weapon->mCharges = 0;
        } else {
            weapon->mCharges = RNG (1, 6) + RNG (1, 6);
        }
    }

    if (weapon->isSelectiveFireWeapon ()) {
        /* start out in burst mode */
        weapon->setToggled ();
    }

    return weapon;
}
