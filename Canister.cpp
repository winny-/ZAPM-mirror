#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"

shVector <shObjectIlk *> CanisterIlks;

shAttack AntimatterCollateralDamage = 
    shAttack (NULL, shAttack::kBlast, shAttack::kBurst, 0, kConcussive, 2, 6);
shAttack UniversalSolventDamage =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0,
              kCorrosive, 2, 6);
shAttack PlasmaDamage =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0, 
              kElectrical, 4, 6);
shAttack NapalmDamage =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0, 
              kBurning, 4, 6);
shAttack LiquidNitrogenDamage =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0, 
              kFreezing, 4, 6);
shAttack PoisonDamage =
    shAttack (NULL, shAttack::kNoAttack, shAttack::kOther, 0, 
              kPoisonous, 1, 3);

static void
abortion () 
{
    if (kDead != Hero.mState && 
        Hero.getStoryFlag ("impregnation")) 
    {
        I->p ("The parasitic alien inside you is killed!");
        Hero.setStoryFlag ("impregnation", 0);
    }
}


static int
quaffAntimatter (shObject *canister)
{
    I->p ("You drink antimatter.");
    canister->setIlkKnown ();
    Hero.die (kAnnihilated, "drinking a canister of antimatter");
    return FULLTURN;
}

#if 0
static int
explodeAntimatter (shObject *can)
{
    char buf[80];
    shCreature *victim = Level->getCreature (can->mX, can->mY);

    if (&Hero == victim) {
        shObject *obj;
        if (NULL != (obj = victim->mBodyArmor) ||
            NULL != (obj = victim->mHelmet) ||
            NULL != (obj = victim->mBelt) ||
            NULL != (obj = victim->mBoots))
        {
            obj->your (buf, 80);
            I->p ("%s is annihilated!");
            victim->removeObjectFromInventory (obj);
            delete obj;
            if (Hero.sufferDamage (&AntimatterCollateralDamage)) {
                Hero.die (kSlain, "a canister of antimatter");
            }
        }
        else {
            Hero.die (kAnnihilated, "a canister of antimatter");
        }
    }
    else if (NULL != victim) {
        victim->die (kAnnihilated, "a cannister of antimatter");
    }
    else {
        //TODO: annihilate features, any stuff that's lying on the ground...
    }
    
    if (Hero.canSee (can)) {
        can->setIlkKnown ();
    }
    return 0;
}
#endif

int
quaffBeer (shObject *can)
{
    can->setIlkKnown ();
    if (Hero.isFrightened ()) {
        Hero.resetFrightened ();
        I->p ("Liquid courage!");
    } else {
        I->p ("Mmmmm... beer!");
    }
    Hero.makeConfused (NDX (2, 50) * 1000);
    return FULLTURN;
}
    

int
quaffNanoCola (shObject *can)
{
    int amt = RNG (1, 6);
    can->setIlkKnown ();
    
    Hero.mChaDrain -= amt;
    Hero.mAbil.mCha += amt;
    I->p ("You feel invigorated!");
    return FULLTURN;
}


int
quaffSuperGlue (shObject *can)
{
    can->setIlkKnown ();
    I->p ("The canister sticks to your tongue!");
    I->p ("You look ridiculous!");
    Hero.setStoryFlag ("superglued tongue", 1);
    if (1 == Hero.sufferAbilityDamage (kCha, 4, 1)) {
        Hero.die (kEmbarassment);
    }
    return FULLTURN;
}


int
quaffUniversalSolvent (shObject *can)
{
    can->setIlkKnown ();
    I->p ("This burns!");
    if (Hero.sufferDamage (&UniversalSolventDamage)) {
        Hero.die (kKilled, "drinking a canister of universal solvent");
    }
    abortion ();
    return FULLTURN;
}


int
useUniversalSolvent (shObject *can)
{
    shDirection where = I->getDirection ();
    if (kNoDirection == where) {
        return 0;
    }
    if (kOrigin == where) {
        if (Hero.getStoryFlag ("superglued tongue")) {
            I->p ("You dissolve the super glue.");
            can->setIlkKnown ();
            Hero.resetStoryFlag ("superglued tongue");
            Hero.mAbil.setByIndex (kCha, 4 + Hero.mAbil.getByIndex (kCha));
            return FULLTURN;
        } else {
            I->p ("You dissolve some of your flesh.  Ouch!");
            can->setIlkKnown ();
            if (Hero.sufferDamage (&UniversalSolventDamage)) {
                Hero.die (kKilled, "bathing in universal solvent");
            }
            return FULLTURN;
        }
    }
    return 0;
}


int
quaffMutagen (shObject *can)
{
    can->setIlkKnown ();
    I->p ("Ick!  That must have been toxic!");
    Hero.getMutantPower ();
    Hero.mRad += NDX (3, 100);
    return FULLTURN;
}


int
quaffWater (shObject *can)
{
    can->setIlkKnown ();
    I->p ("This tastes like water.");
    return FULLTURN;
}


int
shCreature::healing (int hpmaxdice)
{
    int id = 0;
    if (isHero () && mHP < mMaxHP) {
        I->p ("Your wounds are rapidly healing!");
        id++;
    }
    if (isHero () && Hero.getStoryFlag ("brain incision")) {
        Hero.resetStoryFlag ("brain incision");
        I->p ("Your head wound is closed.");
        id++;
    }
    mHP += NDX (4, 8);
    if (mHP > mMaxHP) {
        mMaxHP += NDX (hpmaxdice, 3);
        mHP = mMaxHP;
        if (isHero () && hpmaxdice) {
            I->p ("You feel much healthier.");
            id++;
        }
    } 
    if (isSickened () && !sewerSmells ()) {
        resetSickened ();
        id++;
    }
    if (isViolated ()) {
        resetViolated ();
        id++;
    }
    if (isConfused ()) {
        resetConfused ();
        id++;
    }
    if (isStunned ()) {
        resetStunned ();
        id++;
    }
    checkTimeOuts ();
    return id;
}


int
quaffHealing (shObject *can)
{
    if (Hero.healing (1))
        can->setIlkKnown ();
    return FULLTURN;
}


int
shCreature::fullHealing (int hpmaxdice)
{
    int id = 0;
    if (isHero () && mHP < mMaxHP) {
        I->p ("Your wounds are fully healed!");
        id++;
    }
    if (isHero () && Hero.getStoryFlag ("brain incision")) {
        Hero.resetStoryFlag ("brain incision");
        I->p ("Your head wound is closed.");
        id++;
    }
    mHP += NDX (4, 8);
    if (mHP > mMaxHP) {
        mMaxHP += NDX (hpmaxdice, 6);
        mHP = mMaxHP;
        if (isHero () && hpmaxdice) {
            I->p ("You feel much healthier.");
            id++;
        }
    } 
    mHP = mMaxHP;
    if (isSickened ()) {
        resetSickened ();
        id++;
    }
    if (isViolated ()) {
        resetViolated ();
        id++;
    }
    if (isConfused ()) {
        resetConfused ();
        id++;
    }
    if (isStunned ()) {
        resetStunned ();
        id++;
    }
    checkTimeOuts ();
    return id;
}


int
quaffFullHealing (shObject *can)
{    
    if (Hero.fullHealing (2))
        can->setIlkKnown ();
    return FULLTURN;
}


int
quaffRestoration (shObject *can)
{
    if (Hero.restoration ()) {
        can->setIlkKnown ();
    } else {
        can->maybeName ();
    }
    return FULLTURN;
}


int
quaffBrain (shObject *can)
{
    can->setIlkKnown ();

    I->p ("Brain food!");
    if (!can->isBuggy () && Hero.mMaxAbil.mInt < 25) {
        ++Hero.mMaxAbil.mInt;
    }
    if (Hero.mAbil.mInt < Hero.mMaxAbil.mInt) {
        ++Hero.mAbil.mInt;
    }
    Hero.computeIntrinsics ();
    return FULLTURN;
}


int
quaffGainAbility (shObject *can)
{
    int i;
    int permute[7] = {1, 2, 3, 4, 5, 6, 7};

    if (can) {
        can->setIlkKnown ();
    }
    shuffle (permute, 7, sizeof (int));
    for (i = 0; i < ((can && can->isOptimized ()) ? 7 : 1); i++) {
        int a = Hero.mAbil.getByIndex (permute[i]);
        int m = Hero.mMaxAbil.getByIndex (permute[i]);
        if (m < 25) {
            switch (permute[i]) {
            case kStr: I->p ("You feel strong!"); break;
            case kCon: 
                I->p ("You feel tough!"); 
                if (ABILITY_MODIFIER (m) != ABILITY_MODIFIER (m + 1)) {
                    Hero.mHP += Hero.mCLevel;
                    Hero.mMaxHP += Hero.mCLevel;
                }
                break;
            case kAgi: I->p ("You feel agile!"); break;
            case kDex: I->p ("You feel deft!"); break;
            case kInt: I->p ("You feel smart!"); break;
            case kWis: I->p ("You feel wise!"); break;
            case kCha: I->p ("You feel charismatic!"); break;
            }
            Hero.mAbil.setByIndex (permute[i], a + 1);
            Hero.mMaxAbil.setByIndex (permute[i], m + 1);
        } else {
            switch (permute[i]) {
            case kStr: 
                I->p ("You're already as strong as you can get!"); break;
            case kCon: I->p ("You're already as tough as you can get!"); break;
            case kAgi: I->p ("You're already as agile as you can get!"); break;
            case kDex: 
                I->p ("You're already as dexterous as you can get!"); break;
            case kInt: I->p ("You're already as smart you can get!"); break;
            case kWis: I->p ("You're already as wise as you can get!"); break;
            case kCha: 
                I->p ("You're already as charismatic as you can get!"); break;
            }
        }
    }
    Hero.computeIntrinsics ();
    return FULLTURN;
}



int
quaffRadAway (shObject *can)
{
    if (Hero.mRad > 0) {
        Hero.mRad -= RNG (100, 300);
        if (Hero.mRad < 0) {
            Hero.mRad = 0;
        }
    }
    if (!Hero.mRad)
        I->p ("You feel purified.");
    else 
        I->p ("You feel less contaminated.");

    can->maybeName ();
    return FULLTURN;
}


int
quaffSpeed (shObject *can)
{
    int numdice = 12;
    if (can->isOptimized ()) { 
        numdice = 20;
    } else if (can->isBuggy ()) {
        numdice = 4;
    }
    if (Hero.isHosed ()) {
        Hero.resetHosed ();
        Hero.checkTimeOuts ();
    }
    Hero.makeSpeedy (1000 * NDX (numdice, 20));
    I->p ("You feel speedy!");
    can->setIlkKnown ();
    return FULLTURN;
}


int
quaffPlasma (shObject *can)
{
    can->setIlkKnown ();
    I->p ("It shocks you!");
    if (Hero.sufferDamage (&PlasmaDamage)) {
        Hero.die (kKilled, "drinking a canister of plasma");
    }
    abortion ();
    return FULLTURN;
}


int
quaffNapalm (shObject *can)
{
    can->setIlkKnown ();
    I->p ("It ignites!");
    if (Hero.sufferDamage (&NapalmDamage)) {
        Hero.die (kKilled, "drinking a canister of napalm");
    }
    abortion ();
    return FULLTURN;
}


int
quaffLNO (shObject *can)
{
    can->setIlkKnown ();
    I->p ("It freezes!");
    if (Hero.sufferDamage (&LiquidNitrogenDamage)) {
        Hero.die (kKilled, "drinking a canister of liquid nitrogen");
    }
    abortion ();
    return FULLTURN;
}


int
quaffPoison (shObject *can)
{
    can->setIlkKnown ();
    I->p ("This stuff must be poisonous.");
    if (Hero.sufferDamage (&PoisonDamage)) {
        Hero.die (kKilled, "drinking a canister of poison");
    }
    abortion ();
    return FULLTURN;
}


int
quaffSpice (shObject *can)
{
    can->setIlkKnown ();
    I->p ("You fold space.");
    if (1 == Hero.transport (-1, -1, 100)) {
        Hero.die (kKilled, "folding space");
    }
    return FULLTURN;
}


shCanisterIlk *GenericCanister;


void
initializeCanisters ()
{
    int n = 0;

    GenericCanister = 
      new shCanisterIlk ("canister", "canister", 0, NULL, NULL, NULL, 0, 0);


    new shCanisterIlk ("canister of beer", CanisterData[n++].mDesc, 0,
                       NULL, &quaffBeer, NULL, 5, 100);
    new shCanisterIlk ("canister of super glue", CanisterData[n++].mDesc, 0,
                       &makeRepair, &quaffSuperGlue, NULL, 5, 50);
    new shCanisterIlk ("canister of nano cola", CanisterData[n++].mDesc, 0,
                       NULL, &quaffNanoCola, NULL, 5, 100);
    new shCanisterIlk ("canister of water", "clear canister", 0,
                       NULL, &quaffWater, NULL, 5, 5);


    /* no doozies in the 50 buckazoid category */
    new shCanisterIlk ("canister of Rad-Away", CanisterData[n++].mDesc, 0,
                       NULL, &quaffRadAway, NULL, 50, 75);
    new shCanisterIlk ("canister of restoration", CanisterData[n++].mDesc, 0,
                       NULL, &quaffRestoration, NULL, 50, 50);
    new shCanisterIlk ("canister of healing", CanisterData[n++].mDesc, 0,
                       NULL, &quaffHealing, NULL, 50, 125);


    new shCanisterIlk ("canister of liquid nitrogen", 
                       CanisterData[n++].mDesc, 0,  
                       NULL, &quaffLNO, NULL, 75, 50);
    new shCanisterIlk ("canister of napalm", CanisterData[n++].mDesc, 0,
                       NULL, &quaffNapalm, NULL, 75, 50);


    new shCanisterIlk ("canister of universal solvent", 
                       CanisterData[n++].mDesc, 0,
                       &useUniversalSolvent, &quaffUniversalSolvent, 
                       NULL, 100, 25);
    new shCanisterIlk ("canister of speed", CanisterData[n++].mDesc, 0,
                       NULL, &quaffSpeed, NULL, 100, 50);
    new shCanisterIlk ("canister of poison", CanisterData[n++].mDesc, 0,
                       NULL, &quaffPoison, NULL, 100, 50);
    new shCanisterIlk ("canister of plasma", CanisterData[n++].mDesc, 0, 
                       NULL, &quaffPlasma, NULL, 100, 50);
    

    new shCanisterIlk ("canister of mutagen", CanisterData[n++].mDesc, 
                       shObject::kRadioactive,
                       NULL, &quaffMutagen, NULL, 200, 50);
    new shCanisterIlk ("canister of full healing", CanisterData[n++].mDesc, 
                       0, NULL, &quaffFullHealing, NULL, 200, 50);
    new shCanisterIlk ("canister of gain ability", CanisterData[n++].mDesc, 0,
                       NULL, &quaffGainAbility, NULL, 200, 50);
    new shCanisterIlk ("canister of spice", CanisterData[n++].mDesc, 0,
                       NULL, &quaffSpice, NULL, 200, 50);


    new shCanisterIlk ("brain cylinder", "brain in a jar", kIdentified,
                       NULL, &quaffBrain, NULL,
                       1000, 10);

    new shCanisterIlk ("canister of antimatter", CanisterData[n++].mDesc, 0,
                       NULL, &quaffAntimatter, NULL,
                       1000, 10);

}


shCanisterIlk::shCanisterIlk (const char *name, 
                              const char *appearance, 
                              int flags,
                              shCanisterUseFunc *usefunc,
                              shCanisterFunc *quafffunc,
                              shCanisterFunc *explodefunc,
                              int cost,
                              int prob)
{
    CanisterIlks.add (this);

    mType = kCanister;
    mParent = GenericCanister;
    mName = name;
    mVagueName = "canister";
    mAppearance = appearance;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[kMagenta];
    mMaterial = kGlass;
    mFlags = flags | kMergeable;
    mProbability = prob;
    mWeight = 400;
    mSize = kTiny;
    mHardness = 1;
    mHP = 1;
    mUseFunc = usefunc;
    mQuaffFunc = quafffunc;
    mCost = cost;
    mExplodeFunc = explodefunc;
}


int
useCanister (shObject *can)
{    
    shCanisterIlk *ilk;
    int t;

    assert (can->isA (kCanister));
    ilk = (shCanisterIlk *) can->mIlk;
    
    if (NULL == ilk->mUseFunc) {
        I->p ("You don't know how to use it.");
        return 0;
    } else {
        t = (ilk->mUseFunc) (can);
        if (t) {
            if (Hero.isInShop ()) {
                Hero.usedUpItem (can, 1, "use");
            }
            Hero.useUpOneObjectFromInventory (can);
        }
        return t;
    }
}


int
quaffCanister (shObject *can)
{
    shCanisterIlk *ilk;
    int t;

    assert (can->isA (kCanister));
    ilk = (shCanisterIlk *) can->mIlk;

    if (Hero.isInShop ()) {
        Hero.usedUpItem (can, 1, "drink");
    }
    t =  (ilk->mQuaffFunc) (can);
    Hero.useUpOneObjectFromInventory (can);
    return t;
}


shObject *
createCanister (char *desc,
                int count, int bugginess, int enhancement, int charges)
{
    shCanisterIlk *ilk;
    shObject *canister;

    ilk = (shCanisterIlk *) (NULL == desc ? pickAnIlk (&CanisterIlks) 
                                          : findAnIlk (&CanisterIlks, desc));
    if (NULL == ilk) return NULL;
    canister = new shObject ();

    if (count < 1) count = 1;

    if (-2 == bugginess) {
        int tmp = RNG (8);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
    }

    canister->mIlk = ilk;
    canister->mFlags = ilk->mFlags & 0xffffff00;
    canister->mCount = count;
    canister->setBugginess (bugginess);
    canister->mHP = ilk->mHP;
    canister->setFooproofKnown ();
    canister->setEnhancementKnown ();
    return canister;
}



