#include "Global.h"
#include "Object.h"
#include "Interface.h"
#include "Creature.h"
#include "Hero.h"

shGlyph ObjectGlyphs[kMaxObjectType];

char ObjectSymbols[] = { '9', '$', '+', '?', '!', '(', ']', 
                         ')', '=', '%', '&', '/' };



shObjectDescData JumpsuitData[30] =
{
    { "yellow jumpsuit", kBrightYellow },
    { "green jumpsuit", kBrightGreen },
    { "blue jumpsuit", kBrightBlue },
    { "black jumpsuit", kBlue },
};


shObjectDescData RayGunData[30] =
{
    { "yellow ray gun", kBrightYellow },
    { "indigo ray gun", kBlue },
    { "blue ray gun", kBrightBlue },
    { "red ray gun", kRed },
    { "cyan ray gun", kCyan },
    { "magenta ray gun", kMagenta },
    { "white ray gun", kWhite },
    { "green ray gun", kGreen },
    { "orange ray gun", kBrightRed },
    { "silver ray gun", kBrightCyan },
    { "gold ray gun", kYellow },
};


shObjectDescData ImplantData[30] = 
{ 
    { "nodular bionic implant", kGray },
    { "slippery bionic implant", kGreen },
    { "pulsating bionic implant", kBrightGreen },
    { "quivering bionic implant", kMagenta },
    { "stretchy bionic implant", kRed },
    { "perforated bionic implant", kBrightBlue },
    { "squishy bionic implant", kYellow },
    { "bumpy bionic implant", kBrightRed },
    { "glistening bionic implant", kBrightYellow },
    { "spongy bionic implant", kBrightCyan },
    { "gelatinous bionic implant", kCyan },
    { "smelly bionic implant", kBlue },
    { "stretchy bionic implant", kWhite },
    { "bulbous bionic implant", kBrightMagenta },
    { "gossamer bionic implant", kGreen }
};

shObjectDescData FloppyData[30] = 
{
    { "floppy disk labeled AAIO", kGray },
    { "floppy disk labeled ABDA", kGray },
    { "floppy disk labeled BAYARD WENZEL", kGray },
    { "floppy disk labeled CONFIG.SYS", kGray },
    { "floppy disk labeled EIT ME", kGray },
    { "floppy disk labeled EUTOW", kGray },
    { "floppy disk labeled FWQWGADZ", kGray },
    { "floppy disk labeled GINOH DIVAD", kGray },
    { "floppy disk labeled IHTFP", kGray },
    { "floppy disk labeled JUSTIN BAILEY", kGray },
    { "floppy disk labeled LV FTS", kGray },
    { "floppy disk labeled PAPOU", kGray },
    { "floppy disk labeled PHAM NUWEN", kGray },
    { "floppy disk labeled RIBBET", kGray },
    { "floppy disk labeled SOO BAWLZ", kGray },
    { "floppy disk labeled THX 1138", kGray },
    { "floppy disk labeled XA 35", kGray },
    { "floppy disk labeled XERTH Q3", kGray },
    { "floppy disk labeled YERXA", kGray },
    { "floppy disk labeled ZARRO BOOGS", kGray },
};

shObjectDescData CanisterData[30] =
{
    { "aluminum canister", kCyan },
    { "brass canister", kYellow },
    { "copper canister", kRed },
    { "glass canister", kBrightCyan },
    { "grooved canister", kGreen },
    { "insulated canister", kBrightGreen },
    { "iron canister", kBlue },
    { "magnesium canister", kWhite },
    { "plastic canister", kGray },
    { "pressurized canister", kBrightRed },
    { "reinforced canister", kBrightYellow },
    { "square canister", kGreen },
    { "steel canister", kCyan },
    { "tin canister", kYellow },
    { "titanium canister", kBrightCyan },
    { "vacuum sealed canister", kBrightBlue },
    { "squeezable canister", kMagenta }
};

shObjectDescData BeltData[5] =
{
    { "padded belt", kYellow },
    { "old belt", kBrightGreen },
};

void
randomizeIlkNames ()
{
    shuffle (CanisterData, 17, sizeof (shObjectDescData));
    shuffle (ImplantData, 15, sizeof (shObjectDescData));
    shuffle (FloppyData, 20, sizeof (shObjectDescData));
    shuffle (RayGunData, 11, sizeof (shObjectDescData));
    shuffle (JumpsuitData, 4, sizeof (shObjectDescData));
    shuffle (BeltData, 2, sizeof (shObjectDescData));
}




shEnergyType
shObject::vulnerability ()
{
    if (isA (kImplant)) {
        return kElectrical;
    }
    switch (mIlk->mMaterial) {
    case kVegetable:
    case kFleshy:
    case kPaper:
    case kCloth:
    case kLeather:
    case kWood:
    case kPlastic:
    case kWax:
    case kBone:
        return kBurning;

    case kIron:
    case kBrass:
    case kTin:
    case kBronze:
    case kLead:
    case kSteel:
    case kAluminum:
    case kCopper:
    case kSilver:
    case kElectrum:
        return kCorrosive;

    case kSilicon:
    case kGlass:
    case kCrystal:
        return kElectrical;

    case kLiquid:
    case kTitanium:
    case kGold:
    case kPlatinum:
    case kMithril:
    case kGem:
    case kMineral:
    case kRock:
    case kCarbonFiber:
    case kPlasteel:
    case kAdamantium:
    case kEndurium:
    default:
        return kNoEnergy;
    }
};




void
initializeObjects ()
{
    int i;
    for (i = 0; i < kMaxObjectType; i++) {
        initGlyph (&ObjectGlyphs[i], ObjectSymbols[i], kBlack, 0);
    }
    initializeMoney ();
    initializeTools ();
    initializeWeapons ();
    initializeRayGuns ();
    initializeArmor ();
    initializeCanisters ();
    initializeFloppyDisks ();
    initializeImplants ();
    initializeWreck ();
    initializeArtifacts ();
};


shThing::~shThing ()
{
    int i;
    for (i = 0; i < mEvents.count (); i++) {
        mEvents.get (i) -> cancel ();
    }
}

void
shThing::addEvent (shEvent *event)
{
    mEvents.add (event);
}


void
shThing::removeEvent (shEvent *event)
{
    mEvents.remove (event);
}



shEntity::~shEntity ()
{
}


void
shEntity::saveState (int fd)
{
}
    
void
shEntity::loadState (int fd)
{
}


int
shObjectIlk::isA (shObjectIlk *ilk)
{
    shObjectIlk *myilk = this;
    while (NULL != myilk) {
        if (myilk == ilk) {
            return 1;
        }
        myilk = myilk->mParent;
    }
    return 0;
}


int
shObject::isA (const char *ilkname)
{
    shObjectIlk *ilk = NULL;
    if (0 == strcmp (ilkname, "buckazoid")) ilk = &MoneyIlk;
    if (!ilk) ilk = findAnIlk (&WeaponIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&ArmorIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&CanisterIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&FloppyDiskIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&ToolIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&RayGunIlks, ilkname);
    if (!ilk) ilk = findAnIlk (&ImplantIlks, ilkname);
    if (!ilk) return 0;
    return isA (ilk);
}


void
shObject::name (char *newname)
{
    if (!newname) {
        char prompt[80];
        char buf[80];

        these (buf, 80);
        snprintf (prompt, 80, "Name %s:", buf);
        if (!I->getStr (buf, 80, prompt)) {
            I->p ("Never mind.");
            return;
        } else {
            newname = buf;
        }
    }
    if (mUserName) {
        free (mUserName);
    }
    if (1 == strlen (newname) && ' ' == *newname) { 
        mUserName = NULL;
    } else {
        mUserName = strdup (newname);
    }
}


void
shObject::nameIlk ()
{
    char prompt[80];
    char buf[80];
    char *tmp;

    tmp = mIlk->mUserName;
    mIlk->mUserName = NULL;
    getShortDescription (buf, 80);
    mIlk->mUserName = tmp;
        
    snprintf (prompt, 80, "Call %s:", buf);
    if (!I->getStr (buf, 80, prompt)) {
        I->p ("Never mind.");
        return;
    }
    if (mIlk->mUserName) {
        free (mIlk->mUserName);
    }
    if (1 == strlen (buf) && ' ' == *buf) { 
        mIlk->mUserName = NULL;
    } else {
        mIlk->mUserName = strdup (buf);
    }
}


int
shObject::getShortDescription (char *buf, int len)
{
    int l = 0;

    if (mIlk->mUserName) {
        l += snprintf (buf + l, len - l, 
                       isIlkKnown () ? mIlk->mName : mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
        l += snprintf (buf + l, len - l, " called %s", mIlk->mUserName);
    } else {
        l += snprintf (buf + l, len - l, 
                       isIlkKnown () ? mIlk->mName : mIlk->mAppearance);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
    }
    return l;
}


int
shObject::getDescription (char *buf, int len)
{
    int l = 0;
    char *fooproof = NULL;
    char *dmg;

    if (Hero.hasBugSensing ()) {
        setBugginessKnown ();
    }

    if (isBugginessKnown () && !isBugProof ()) {
        l += snprintf (buf + l, len - l, isBuggy () ? "buggy " : 
                                         isOptimized () ? "optimized " : 
                                         "debugged ");
    }

    switch (vulnerability ()) {
    case kCorrosive: fooproof = "rustproof"; dmg = "corroded"; break;
    case kBurning: fooproof = "fireproof"; dmg = "burnt"; break;
    default: dmg = "damaged"; 
    }

    switch (mDamage) {
    case 1: l += snprintf (buf + l, len - l, "%s ", dmg); break;
    case 2: l += snprintf (buf + l, len - l, "very %s ", dmg); break;
    case 3: l += snprintf (buf + l, len - l, "extremely %s ", dmg); break;
    default: 
        if (isA (kFloppyDisk) && isCrackedKnown () && isCracked ()) {
            fooproof = "cracked";
        }
        break;
    }

    if (isFooproof () && isFooproofKnown ()) {
        l += snprintf (buf + l, len - l, "%s ", fooproof); 
    }

    if (isEnhanceable () && isEnhancementKnown ()) {
        l += snprintf (buf + l, len - l, "%s%d ", mEnhancement < 0 ? "" : "+",
                       mEnhancement);
    }

    if (&WreckIlk == mIlk) {
        l += snprintf (buf + l, len - l, "disabled %s", mCorpseIlk->mName);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
    } else if (!isAppearanceKnown ()) {
        l += snprintf (buf + l, len - l, "%s", mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
    } else if (mIlk->mUserName) {
        l += snprintf (buf + l, len - l, 
                       isIlkKnown () ? mIlk->mName : mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
        l += snprintf (buf + l, len - l, " called %s", mIlk->mUserName);
    } else {
        l += snprintf (buf + l, len - l, 
                       isIlkKnown () ? mIlk->mName : mIlk->mAppearance);
        if (mCount > 1) {
            makePlural (buf, len);
            ++l;
        }
    }
    if (mUserName) {
        l += snprintf (buf + l, len - l, " named %s", mUserName);
    }

    if (isChargeable () && isChargeKnown () && !isA ("empty ray gun")) {
        l += snprintf (buf + l, len - 1, " (%d charg%s)", mCharges,
                       mCharges == 1 ? "e" : "es");
    }
    buf[len-1] = 0;
    return l;
}


int 
shObject::inv (char *buf, int buflen) 
{
    char tempbuf[80];
    int l;

    getDescription (tempbuf, 80);
    
    if (mCount > 1) {
        l = snprintf (buf, buflen, "%d %s", mCount, tempbuf);
    } else {
        l = snprintf (buf, buflen, "%s %s", 
                      isUnique () ? "the" :
                      isvowel (tempbuf[0]) ? "an" : "a", tempbuf);
    }
    if (isWorn ()) {
        if (isA (kImplant)) {
            l += snprintf (buf + l, buflen - l, " (installed in %s)",
                           describeImplantSite (mImplantSite));
        } else {
            l += snprintf (buf + l, buflen - l, " (worn)");
        }
    } else if (isWielded ()) {                    
        l += snprintf (buf + l, buflen - l, " (wielded");
        if (isSelectiveFireWeapon ()) {
            l += snprintf (buf + l, buflen - l, ", %s",
                           isToggled () ? "burst mode" : "single fire");
        }
        l += snprintf (buf + l, buflen - l, ")");
    }
    if (isActive ()) {
        l += snprintf (buf + l, buflen - 1, " (activated)");
    }
    if (isUnpaid ()) {
        l += snprintf (buf + l, buflen - 1, " (unpaid, $%d)", 
                       mCount * mIlk->mCost);
    }
    if (isToggled ()) {
        if (isA ("geiger counter")) {
            l += snprintf (buf + l, buflen - 1, " (clicking)");            
        }
    }
/*  if (GodMode) {
        l += snprintf (buf + l, buflen - l, " (%d g)", getMass ());
    }*/
    buf[buflen-1] = 0;
    return l;
}


int
shObject::isA (shObjectIlk *type)
{
    return mIlk->isA (type);
}


int
shObject::canMerge (shObject *obj)
{
    if (NULL == obj) {
        obj = this;
    }
    if (obj->mIlk != mIlk || &WreckIlk == mIlk) {
        return 0;
    }
    if ((mIlk->mFlags & kMergeable) &&
        (obj->mFlags == mFlags) &&
        (obj->mBugginess == mBugginess))
    {
        return 1;
    }
    return 0;
}


void
shObject::merge (shObject *obj)
{
    if (obj == this) return;
    mCount += obj->mCount;
    delete obj;
}


shObject *
shObject::split (int count)
{
    shObject *result;
    
    assert (count < mCount);

    mCount -= count;
    result = new shObject ();
    result->mIlk = mIlk;
    result->mCount = count;
    result->mHP = mHP;
    result->mBugginess = mBugginess;
    result->mFlags = mFlags;
    result->mCorpseIlk = mCorpseIlk;
    return result;
}


int
shObject::sufferDamage (shAttack *attack, int x, int y, 
                        int multiplier, int specialonly)
{
    int j;
    int damage;
    int totaldamage = 0;
    char buf[64];
    
    if (Hero.owns (this)) {
        your (buf, 64);
    } else {
        the (buf, 64);
    }

    for (j = 0; j < 2; j++) {
        shEnergyType energy = (shEnergyType) attack->mDamage[j].mEnergy;

        if (kNoEnergy == energy) {
            break;
        } else if (kCorrosive == energy) {
            if (kCorrosive == vulnerability ()) {
                if (isFooproof ()) {
                    setFooproofKnown ();
                    I->p ("Somehow, %s is not affected.", buf);
                    continue;
                }
                if (isOptimized () && RNG (4)) {
                    I->p ("Somehow, %s is not affected.", buf);
                    continue;
                }
                ++mDamage;
                I->debug ("corroded %s %d", buf, mDamage);
                if (Hero.canSee (x, y)) {
                    switch (mDamage) {
                    case 1:
                        I->p ("%s corrodes!", buf); break;
                    case 2:
                        I->p ("%s corrodes some more!", buf); break;
                    default:
                        mDamage = 3;
                        I->p ("%s is thoroughly corroded!", buf); break;
#if 0               /* this makes it too easy to dispose of a welded weapon */
                    default:
                        I->p ("%s dissolves completely!", buf);
                        return 1;
#endif
                    }
                }
            } else if (Hero.canSee (x, y)) {
                I->p ("%s is not affected.", buf);
            }
        } else if (kElectrical == energy) {
            if (kElectrical == vulnerability ()) {
                damage = NDX (multiplier * attack->mDamage[j].mNumDice,
                              attack->mDamage[j].mDieSides);
                totaldamage += damage;
                totaldamage -= mIlk->mHardness;
                if (totaldamage > mHP) {
                    if (Hero.canSee (x, y)) {
                        if (Hero.owns (this) && isWorn ()) {
                            I->p ("%s is ejected from your %s in a shower of "
                                  "sparks!", 
                                  buf, describeImplantSite (mImplantSite));
                            Hero.doff (this);
                            return 0;
                        }
                    }
                    return 1;
                } else {
                    /* either it explodes or not, no damage */
                    totaldamage = 0;
                }
            }
        } else if (kBurning == energy) {
            if (isA (kFloppyDisk)) {
                if (1 == mCount ) {
                    I->p ("%s melts!", buf);
                } else {
                    I->p ("One of %s melts!", buf);
                }
                return 1;
            }
        } else if (kBugging == energy) {
            if (isOptimized ()) setDebugged ();
            else setBuggy ();
        } else if (specialonly) {
            continue;
        } else {
            damage = NDX (multiplier * attack->mDamage[j].mNumDice,
                          attack->mDamage[j].mDieSides);
            totaldamage += damage;
        }
        multiplier = 1;
    }

    I->debug ("dealt %d damage to object %p", totaldamage, this);

    totaldamage -= mIlk->mHardness;
    if (totaldamage <= 0) {
        return 0;
    }

    mHP -= totaldamage;
    if (mHP <= 0) {
        mHP = 0;
        I->p ("%s is destroyed!", buf);
        return 1;
    }
    return 0;    
}


int
shObject::getPsiModifier ()
{
    if (kArmor == mIlk->mType) {
        return ((shArmorIlk *) mIlk) -> mPsiModifier;
    } else if (kImplant == mIlk->mType) {
        if (isA ("psionic amplifier")) {
            return mEnhancement;
        }
        return ((shImplantIlk *) mIlk) -> mPsiModifier;
    } else {
        return 0;
    }
}


int
shObject::getArmorBonus () 
{
    if (kArmor != mIlk->mType) {
        return 0;
    } else { 
        return maxi (0, ((shArmorIlk *) mIlk) -> mArmorBonus 
                        - mDamage + mEnhancement);
    }
}


int 
shObject::getThreatRange (shCreature *target)
{
    if (isA (kWeapon)) { 
        return ((shWeaponIlk *) mIlk) -> mThreatRange;
    }
    else { /* impossible to critical hit with improvised weapon */
        return 999;
    }   
}

int
shObject::getCriticalMultiplier ()
{
    if (isA (kWeapon)) { 
        return ((shWeaponIlk *) mIlk) -> mCriticalMultiplier;
    }
    else { /* impossible to critical hit with improvised weapon */
        return 1;
    }   
}


shObjectIlk *
pickAnIlk (shVector <shObjectIlk *> *ilks)
{
    int i, n, r, p;

    if (0 == ilks->count ()) {
        return NULL;
    }

    n = 0;
    for (i = 0; i < ilks->count (); i++) {
        p = ilks->get (i) -> mProbability;
        n += p < 0 ? 0 :p;
    }

    if (0 == n) { 
        /* some items have zero probability of random generation, but
           sometimes this function gets called by createObject () with
           a specific request to create some special item.
        */
        return ilks->get (RNG (ilks->count ()));
    }

    r = RNG (n);
    while (--i >= 0) {
        p = ilks->get (i) -> mProbability;
        n -= p < 0 ? 0 :p;
        if (r >= n) {
            return ilks->get (i);
        }
    }
    I->debug ("couldn't pick an ilk!!");
    return NULL;
}


shObjectIlk *
findAnIlk (shVector <shObjectIlk *> *ilks, const char *name, int abstractokay)
{
    int i;
    for (i = 0; i < ilks->count (); i++) {
        shObjectIlk *ilk = ilks->get (i);
        if (0 == strcmp (name, ilk->mName)) {
            if (!abstractokay || ilk->mProbability < 0) {
                /* an abstract ilk; find a sub-ilk */
                shVector <shObjectIlk *> subilks;
                int i;

                for (i = 0; i < ilks->count (); i++) {
                    if (ilks->get (i) -> isA (ilk)) {
                        subilks.add (ilks->get (i));
                    }
                }
                return pickAnIlk (&subilks);
            }

            return ilk;
        }
    }
    return NULL;
}


shObjectIlk *
findAnAbstractIlk (shVector <shObjectIlk *> *ilks, const char *name)
{
    return findAnIlk (ilks, name, 1);
}


/* generate random object according to level specification:
   -1 level indicates shop
*/
shObject *
generateObject (int level)
{
    /* first what kind of object? */

    int type;
    
    while (1) {
        type = RNG (100);
        if (type < 20) {
            if (-1 != level) {
                return createMoney (NDX (level, 20 + 4 * level));
            }
        } else if (type < 35) {
            return createWeapon ();
        } else if (type < 45) {
            return createArmor ();
        } else if (type < 60) {
            return createTool ();
        } else if (type < 75) {
            return createCanister (); 
        } else if (type < 93) {
            return createFloppyDisk ();
        } else if (type < 98) {
            return createImplant ();
        } else if (type < 100) {
            return createRayGun ();
        } else if (-1 == level) {
            continue;
        } else {
            return createMoney (1);
        }
    }

    return NULL;
}



static int InventorySortOrder [kMaxObjectType] =
{
    11, 0, 7, 9, 5, 10, 3, 1, 2, 4, 6, 8
};

/* used for sorting
   returns: -1, 0 or 1 depending on whether obj1 belongs before, with, or 
            after obj2 in an inventory list.  sorts on type
*/
int
compareObjects (shObject **obj1, shObject **obj2)
{
    int t1 = InventorySortOrder[(*obj1)->mIlk->mType];
    int t2 = InventorySortOrder[(*obj2)->mIlk->mType];
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return 0;
}


