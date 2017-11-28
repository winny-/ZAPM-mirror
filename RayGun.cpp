#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Creature.h"
#include "Hero.h"
shVector <shObjectIlk *> RayGunIlks;

shRayGunIlk *GenericRayGun = NULL;

void
initializeRayGuns ()
{
    int n = 0;

    GenericRayGun =
    new shRayGunIlk ("ray gun", "ray gun", RayGunData[n].mColor, 0, 
                     shAttack::kNoAttack, kNoEnergy, 6, 1,
                     200, 0);

    new shRayGunIlk ("empty ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kNoAttack, kNoEnergy, 6, 1,
                     200, 10);
    n++;

    new shRayGunIlk ("freeze ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kFreezeRay, kFreezing, 4, 8,
                     800, 10);
    n++;

    new shRayGunIlk ("disintegration ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kDisintegrationRay, kDisintegrating, 4, 8,
                     2000, 1);
    n++;

    new shRayGunIlk ("heat ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kHeatRay, kBurning, 4, 8,
                     800, 15);
    n++;

    new shRayGunIlk ("gauss ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kGaussRay, kMagnetic, 4, 8,
                     800, 12);
    n++;

    new shRayGunIlk ("poison ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kPoisonRay, kPoisonous, 1, 4,
                     800, 12);
    n++;

    new shRayGunIlk ("gamma ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 
                     shObject::kRadioactive,
                     shAttack::kGammaRay, kRadiological, 4, 8,
                     800, 10);
    n++;

    new shRayGunIlk ("stasis ray gun",
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kStasisRay, kParalyzing, 1, 6,
                     800, 5);
    n++;

    new shRayGunIlk ("transporter ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kTransporterRay, kTransporting, 4, 8,
                     800, 15);
    n++;

    new shRayGunIlk ("healing ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kHealingRay, kHealing, 4, 8,
                     800, 5);
    n++;

    new shRayGunIlk ("restoration ray gun", 
                     RayGunData[n].mDesc, RayGunData[n].mColor, 0,
                     shAttack::kRestorationRay, kRestoring, 4, 8,
                     800, 5);
    n++;
}

//constructor:
shRayGunIlk::shRayGunIlk (const char *name, 
                          const char *appearance,
                          shColor color,
                          int flags,
                          shAttack::Type atktype,
                          shEnergyType entype,
                          int numdice,
                          int dicesides,
                          int cost,
                          int prob)
{
    RayGunIlks.add (this);

    mType = kRayGun;
    mParent = GenericRayGun;
    mName = name;
    mVagueName = "ray gun";
    mAppearance = appearance;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[color];
    mCost = cost;
    mMaterial = kSteel;
    mFlags = flags | kAimed | kChargeable;
    mProbability = prob;
    mWeight = 500;
    mSize = kMedium;
    mHardness = 10;
    mHP = 3;
    mAttack.mType = atktype;
    mAttack.mEffect = shAttack::kBeam;
    mAttack.mRange = 6;
    mAttack.mRadius = 1;
    mAttack.mFlags = kAimed;
    mAttack.mAttackTime = FULLTURN;
    mAttack.mDamage[0].mEnergy = entype;
    mAttack.mDamage[0].mNumDice = numdice;
    mAttack.mDamage[0].mDieSides = dicesides;
    mAttack.mDamage[1].mEnergy = kNoEnergy;
    mAttack.mDamage[1].mNumDice = 0;
    mAttack.mDamage[1].mDieSides = 0;
}


shObject *
createRayGun (char *desc /* = NULL */,
              int bugginess /* = -2 */, 
              int charges /* = -22 */)
{
    shRayGunIlk *ilk;
    shObject *raygun;

    ilk = (shRayGunIlk *) (NULL == desc ? pickAnIlk (&RayGunIlks) 
                                        : findAnIlk (&RayGunIlks, desc));
    if (NULL == ilk) return NULL;

    raygun = new shObject ();
    raygun->mIlk = ilk;
    raygun->mCount = 1;
    raygun->mHP = ilk->mHP;
    raygun->mFlags = ilk->mFlags & 0xffffff00;

    if (-2 == bugginess) {
        int tmp = RNG (8);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
    }
    raygun->setBugginess (bugginess);

    if (raygun->isA ("empty ray gun")) {
        raygun->mCharges = 0;
    } else {
        raygun->mCharges = RNG (1, 6) + RNG (1, 6);
    }
    return raygun;
}


char *
shObjectIlk::getRayGunColor ()
{
    static char buf[40];
    char *p;

    strcpy (buf, mAppearance);
    for (p = &buf[0]; ' ' != *p; ++p);
    *p = 0;
    return &buf[0];
}



/* returns time elapsed */

int
loadRayGun (shObject *gun)
{
    shObjectVector v;
    shObject *can;

    selectObjects (&v, Hero.mInventory, kCanister);
    can = Hero.quickPickItem (&v, "reload with", 0);

    if (!can) {
        return 0;
    }
    if        (can->isA ("canister of liquid nitrogen")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "freeze ray gun");
    } else if (can->isA ("canister of napalm")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "heat ray gun");
    } else if (can->isA ("canister of antimatter")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "disintegration ray gun");
    } else if (can->isA ("canister of poison")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "poison ray gun");
    } else if (can->isA ("canister of plasma")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "gauss ray gun");
    } else if (can->isA ("canister of mutagen")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "gamma ray gun");
    } else if (can->isA ("canister of spice")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "transporter ray gun");
    } else if (can->isA ("canister of super glue")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "stasis ray gun");
    } else if (can->isA ("canister of healing") ||
               can->isA ("canister of full healing"))
    {
        gun->mIlk = findAnIlk (&RayGunIlks, "healing ray gun");
    } else if (can->isA ("canister of restoration")) {
        gun->mIlk = findAnIlk (&RayGunIlks, "restoration ray gun");
    } else {
        /* canisters that don't load ray guns right now:
           beer 
           nano cola
           water
           rad-away
           universal solvent
           speed - speed ray?
           gain ability
        */      
        I->p ("Your ray gun is ruined!");
        can->maybeName ();
        Hero.useUpOneObjectFromInventory (can);
        Hero.useUpOneObjectFromInventory (gun);
        return FULLTURN;
    }
    gun->mCharges = NDX (2, 6);
    I->p ("The light on the ray gun is %s now.", gun->mIlk->getRayGunColor ());

    if (can->isIlkKnown ()) {
        gun->setIlkKnown ();
    } else if (gun->isIlkKnown ()) {
        can->setIlkKnown ();
    } else {
        can->maybeName ();
    }

    Hero.useUpOneObjectFromInventory (can);
    gun->setAppearanceKnown ();

    return FULLTURN;
}
