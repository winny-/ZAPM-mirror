#include "Global.h"
#include "Object.h"
#include "Interface.h"
#include "Creature.h"
#include "Hero.h"

shGlyph ObjectGlyphs[kMaxObjectType];

char ObjectSymbols[] = { '9', '$', '+', '?', '!', '(', '[', 
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

shMedicalProcedureDescData MedicalProcedureData[10] =
{
    { "SNU-???", 0 },
    { "THERAC-25", 0 },
    { "JARVIK 7", 0 },
    { "PFAZER", 0 },
    { "PNLS DTH", 0 },
    { "NACSTAC", 0 }
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
    shuffle (MedicalProcedureData, kMedMaxService, sizeof (shMedicalProcedureDescData)); 
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
    initializeEnergy ();
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



/* FIXME: this doesn't work for parent ilks, because findAnIlk() might
          return a descendant ilk.
*/
int
shObject::isA (const char *ilkname)
{
    shObjectIlk *ilk = NULL;
    if (0 == strcmp (ilkname, "buckazoid")) ilk = &MoneyIlk;
    if (0 == strcmp (ilkname, "energy cell")) ilk = &EnergyCellIlk;
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
shObject::name (const char *newname)
{
    if (!newname) {
        char prompt[80];
        char buf[80];

        snprintf (prompt, 80, "Name %s:", these ());
        if (!I->getStr (buf, 40, prompt)) {
            I->p ("Never mind.");
            return;
        } 
        newname = buf;
    }
    if (mUserName) {
      free ((void *) mUserName);
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
    const char *tmp;
    const char *desc;

    tmp = mIlk->mUserName;
    mIlk->mUserName = NULL;
    desc = getShortDescription ();
    mIlk->mUserName = tmp;
        
    snprintf (prompt, 80, "Call %s:", desc);
    if (!I->getStr (buf, 40, prompt)) {
        I->p ("Never mind.");
        return;
    }
    if (mIlk->mUserName) {
      free ((void *) mIlk->mUserName);
    }
    if (1 == strlen (buf) && ' ' == *buf) { 
        mIlk->mUserName = NULL;
    } else {
        mIlk->mUserName = strdup (buf);
    }
}


const char *
shObject::getVagueDescription ()
{
    return mIlk->mVagueName;
}


const char *
shObject::getShortDescription ()
{
    int l = 0;
    char *buf = GetBuf ();

    if (mIlk->mUserName) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s",
                       isIlkKnown () ? mIlk->mName : mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
        l += snprintf (buf + l, SHBUFLEN - l, " called %s", mIlk->mUserName);
    } else {
        l += snprintf (buf + l, SHBUFLEN - l, "%s",
                       isIlkKnown () ? mIlk->mName : mIlk->mAppearance);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
    }
    return buf;
}


static const char *
energyFormat (shEnergyType t) {
    switch (t) {
    case kSlashing: 
    case kPiercing:
    case kConcussive:
    case kForce:
    case kLaser:
        return "%dd%d";
    case kBlinding: return NULL;
    case kBurning: return "%dd%d fire";
    case kConfusing: return NULL;
    case kCorrosive: return "%dd%d acid";
    case kElectrical: return "%dd%d shock";
    case kFreezing: return "%dd%d ice";
    case kMagnetic: return "%dd%d gauss";
    case kParalyzing: return NULL;
    case kPoisonous: return "%dd%d poison";
    case kRadiological: return NULL;
    case kPsychic: return NULL;
    case kStunning: return NULL;;
    default:
        return NULL;
    }
    return NULL;
}



const char *
shObject::getDescription ()
{
    int l = 0;
    const char *fooproof = NULL;
    const char *dmg;
    char *buf = GetBuf ();

    if (isBugginessKnown () && !isBugProof ()) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s",
                       isBuggy () ? "buggy " : 
                       isOptimized () ? "optimized " : "debugged ");
    }

    switch (vulnerability ()) {
    case kCorrosive: fooproof = "rustproof"; dmg = "corroded"; break;
    case kBurning: fooproof = "fireproof"; dmg = "burnt"; break;
    default: dmg = "damaged"; 
    }

    switch (mDamage) {
    case 1: l += snprintf (buf + l, SHBUFLEN - l, "%s ", dmg); break;
    case 2: l += snprintf (buf + l, SHBUFLEN - l, "very %s ", dmg); break;
    case 3: l += snprintf (buf + l, SHBUFLEN - l, "extremely %s ", dmg); break;
    default: 
        if (isA (kFloppyDisk) && isCrackedKnown () && isCracked ()) {
            fooproof = "cracked";
        }
        break;
    }

    if (isFooproof () && isFooproofKnown ()) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s ", fooproof); 
    }

    if (isEnhanceable () && isEnhancementKnown ()) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s%d ", 
                       mEnhancement < 0 ? "" : "+", mEnhancement);
    }

    if (&WreckIlk == mIlk) {
        l += snprintf (buf + l, SHBUFLEN - l, "disabled %s", mCorpseIlk->mName);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
    } else if (!isAppearanceKnown ()) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s", mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
    } else if (mIlk->mUserName) {
        l += snprintf (buf + l, SHBUFLEN - l, "%s",
                       isIlkKnown () ? mIlk->mName : mIlk->mVagueName);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
        l += snprintf (buf + l, SHBUFLEN - l, " called %s", mIlk->mUserName);
    } else {
        l += snprintf (buf + l, SHBUFLEN - l, "%s",
                       isIlkKnown () ? mIlk->mName : mIlk->mAppearance);
        if (mCount > 1) {
            makePlural (buf, SHBUFLEN);
            ++l;
        }
    }

    if (GodMode && isIlkKnown () && isA (kWeapon)) {
        shWeaponIlk *wilk = (shWeaponIlk *) mIlk;
        const char *format = 
            energyFormat ((shEnergyType) wilk->mAttack.mDamage[0].mEnergy);
        if (format) {
            l += snprintf (buf + l, SHBUFLEN - l, " ("); 
            if (wilk->mAmmoBurst > 1 && &EnergyCellIlk != wilk->mAmmoType)
                l += snprintf (buf + l, SHBUFLEN - l, "%dx", 
                               wilk->mAmmoBurst); 
            l += snprintf (buf + l, SHBUFLEN - l, format, 
                           wilk->mAttack.mDamage[0].mNumDice, 
                           wilk->mAttack.mDamage[0].mDieSides);
            format = 
                energyFormat ((shEnergyType) wilk->mAttack.mDamage[1].mEnergy);
            if (format) {
                l += snprintf (buf + l, SHBUFLEN - l, " + "); 
                l += snprintf (buf + l, SHBUFLEN -l, format, 
                               wilk->mAttack.mDamage[1].mNumDice, 
                               wilk->mAttack.mDamage[1].mDieSides);
            }
            l += snprintf (buf + l, SHBUFLEN - l, ")"); 
        }
    }

    if (mUserName) {
        l += snprintf (buf + l, SHBUFLEN - l, " named %s", mUserName);
    }

    if (isChargeable () && isChargeKnown () && !isA ("empty ray gun")) {
        l += snprintf (buf + l, SHBUFLEN - 1, " (%d charg%s)", mCharges,
                       mCharges == 1 ? "e" : "es");
    }
    buf[SHBUFLEN-1] = 0;
    return buf;
}


const char *
shObject::inv () 
{
    char *buf = GetBuf ();
    int l;

    if (mCount > 1) {
        l = snprintf (buf, SHBUFLEN, "%d %s", mCount, getDescription ());
    } else {
        const char *tmp = getDescription ();
        l = snprintf (buf, SHBUFLEN, "%s %s", 
                      isUniqueKnown () ? "the" : 
                      isvowel (tmp[0]) ? "an" : "a", tmp);
    }
    if (isWorn ()) {
        if (isA (kImplant)) {
            l += snprintf (buf + l, SHBUFLEN - l, " (implanted in %s)",
                           describeImplantSite (mImplantSite));
        } else {
            l += snprintf (buf + l, SHBUFLEN - l, " (worn)");
        }
    } else if (isWielded ()) {                    
        l += snprintf (buf + l, SHBUFLEN - l, " (wielded");
        if (isSelectiveFireWeapon ()) {
            l += snprintf (buf + l, SHBUFLEN - l, ", %s",
                           isToggled () ? "burst mode" : "single fire");
        }
        l += snprintf (buf + l, SHBUFLEN - l, ")");
    }
    if (isActive ()) {
        l += snprintf (buf + l, SHBUFLEN - 1, " (activated)");
    }
    if (isUnpaid ()) {
        l += snprintf (buf + l, SHBUFLEN - 1, " (unpaid, $%d)", 
                       mCount * mIlk->mCost);
    }
    if (isToggled ()) {
        if (isA ("geiger counter")) {
            l += snprintf (buf + l, SHBUFLEN - 1, " (clicking)");            
        }
    }
/*  if (GodMode) {
        l += snprintf (buf + l, SHBUFLEN - l, " (%d g)", getMass ());
    }*/
    buf[SHBUFLEN-1] = 0;
    return buf;
}


const char *
shObject::her (shCreature *owner)
{
    char *buf = GetBuf ();
    int l;
    const char *pronoun = "its";
    if (kFemale == owner->mGender) 
        pronoun = "her";
    else if (kMale == owner->mGender) 
        pronoun = "his";
    if (mCount > 1) {
        l = snprintf (buf, SHBUFLEN, "%s %d %s", 
                      pronoun, mCount, getDescription ());
    } else {
        l = snprintf (buf, SHBUFLEN, "%s %s", pronoun, getDescription ());
    }
    return buf;
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


static shObjectVector DeletedObjects;

void
shObject::merge (shObject *obj)
{
    if (obj == this) return;
    mCount += obj->mCount;
    /* HACK: don't delete the object right away; we might need to use
       it to print a message or something.  (For example, the case of
       a shopkeeper giving a quote about an object picked up in a
       shop.) */
    DeletedObjects.add (obj);
    //delete obj;
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


/* returns number of objects destroyed from stack */
int
shObject::sufferDamage (shAttack *attack, int x, int y, 
                        int multiplier, int specialonly)
{
    int j;
    int damage;
    int totaldamage = 0;
    int cansee = Hero.canSee (x, y);
    int numdestroyed = RNG (1,2);
    int nearby = distance (&Hero, x, y) <= 30;
    const char *theobj;

    if (Hero.owns (this)) {
        cansee = 1;
        theobj = your ();
    } else if (mOwner) {
        theobj = her (mOwner);
    } else {
        theobj = theQuick ();
    }

    if (numdestroyed > mCount) {
        numdestroyed = mCount;
    }

    for (j = 0; j < 2; j++) {
        shEnergyType energy = (shEnergyType) attack->mDamage[j].mEnergy;

        if (kNoEnergy == energy) {
            break;
        } else if (kCorrosive == energy) {
            if (!Hero.owns (this)) 
                cansee = 0; /* reduce messages */
            if (kCorrosive == vulnerability ()) {
                if (isFooproof ()) {
                    if (cansee) {
                        setFooproofKnown ();
                        I->p ("Somehow, %s is not affected.", theobj);
                    }
                    continue;
                }
                if (isOptimized () && RNG (4)) {
                    if (cansee) {
                        I->p ("Somehow, %s is not affected.", theobj);
                    }
                    continue;
                }
                if (mDamage < 3)
                    ++mDamage;
                I->debug ("corroded %s %d", theobj, mDamage);
                if (cansee) {
                    switch (mDamage) {
                    case 1:
                        I->p ("%s corrodes!", theobj); break;
                    case 2:
                        I->p ("%s corrodes some more!", theobj); break;
                    default:
                        I->p ("%s is thoroughly corroded!", theobj); break;
#if 0               /* this makes it too easy to dispose of a welded weapon */
                    default:
                        I->p ("%s dissolves completely!", theobj);
                        return 1;
#endif
                    }
                }
            } else if (cansee) {
                I->p ("%s is not affected.", theobj);
            }
        } else if (kElectrical == energy) {
            if (kElectrical == vulnerability ()) {
                damage = NDX (multiplier * attack->mDamage[j].mNumDice,
                              attack->mDamage[j].mDieSides);
                totaldamage += damage;
                totaldamage -= mIlk->mHardness;
                if (totaldamage > mHP) {
                    if (cansee) {
                        if (Hero.owns (this) && isWorn ()) {
                            I->p ("%s is ejected from your %s in a shower of "
                                  "sparks!", 
                                  theobj, describeImplantSite (mImplantSite));
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
                if (cansee) {
                    if (1 == mCount) {
                        I->p ("%s melts!", theobj);
                    } else if (1 == numdestroyed) {
                        I->p ("One of %s melts!", theobj);
                    } else if (numdestroyed == mCount) {
                        I->p ("%s melt!", theobj);
                    } else {
                        I->p ("Some of %s melt!", theobj);                        
                    }
                } else if (nearby) { /* FIXME: no msg needed when underwater*/
                    I->p ("You smell burning plastic.");
                }
                return numdestroyed;
            }
        } else if (kFreezing == energy) {
            if (isA (kCanister) && !isA ("canister of liquid nitrogen")) {
                if (cansee) {
                    if (1 == mCount ) {
                        I->p ("%s freezes and shatters!", theobj);
                    } else if (1 == numdestroyed) {
                        I->p ("One of %s freezes and shatters!", theobj);
                    } else if (numdestroyed == mCount) {
                        I->p ("%s freeze and shatter!", theobj);
                    } else {
                        I->p ("Some of %s freeze and shatter!", theobj);
                    }
                } else if (nearby) { /* FIXME: no msg when in vacuum */
                    I->p ("You hear something shatter.");
                }
                return numdestroyed;
            }
	} else if (kMagnetic == energy) {
            /* FIXME: clerkbot should get mad if this is hero's fault */
            mEnhancement /= 2;
            mBugginess = 0;
            if (isA (kFloppyDisk) && !(isA ("blank floppy disk"))) {
                mIlk = findAnIlk (&FloppyDiskIlks, "blank floppy disk");
                resetFlag (kFooproof);
            }
        } else if (kBugging == energy) {
            if (isOptimized ()) setDebugged ();
            else setBuggy ();
        } else if (specialonly) {
            continue;
        } else if (kConcussive == energy && kLeather == mIlk->mMaterial) {
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
        if (cansee) {
            I->p ("%s is destroyed!", theobj);
        }
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
        if (0 == strcasecmp (name, ilk->mName)) {
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
        } else if (type < 48) {
            return createEnergyCell ();
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


void
purgeDeletedObjects ()
{
    while (DeletedObjects.count ()) {
        delete DeletedObjects.removeByIndex(0);
    }
}
