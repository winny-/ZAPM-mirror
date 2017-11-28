#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Interface.h"

shVector <shObjectIlk *> ImplantIlks;

const char *
describeImplantSite (shImplantIlk::Site site)
{
    switch (site) {
    case shImplantIlk::kFrontalLobe: return "frontal lobe";
    case shImplantIlk::kTemporalLobe: return "temporal lobe";
    case shImplantIlk::kParietalLobe: return "parietal lobe";
    case shImplantIlk::kOccipitalLobe: return "occipital lobe";
    case shImplantIlk::kCerebellum: return "cerebellum";
    case shImplantIlk::kLeftEar: return "left ear";
    case shImplantIlk::kRightEyeball: return "right eyeball";
    default:
        return "brain";
    }
}

void
initializeImplants ()
{
    int n = 0;

    new shImplantIlk ("health monitor", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      kHealthMonitoring, -3, 300, 50);
    n++;

    new shImplantIlk ("radiation processor", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      kRadiationProcessing, -3, 300, 50);
    n++;

    /* +10 poison resistance */
    new shImplantIlk ("poison resistor", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      0, -3, 300, 50);
    n++;

    new shImplantIlk ("babel fish", "fish", "yellow fish",
                      kYellow, kFleshy, 0, shImplantIlk::kAnyEar,
                      kTranslation, 0, 300, 50);

    /* grants enhancement to intelligence */
    new shImplantIlk ("cerebral coprocessor", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kEnhanceable, shImplantIlk::kAnyBrain,
                      0, -3, 300, 50);
    n++;

    /* grants enhancement to agility */
    new shImplantIlk ("reflex coordinator", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kEnhanceable, shImplantIlk::kAnyBrain,
                      0, -3, 300, 50);
    n++;

    /* grants enhancement to strength */
    new shImplantIlk ("adrenaline generator", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kEnhanceable, shImplantIlk::kAnyBrain,
                      0, -3, 300, 50);
    n++;

    new shImplantIlk ("cortex crossover", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kUsuallyBuggy, shImplantIlk::kAnyBrain,
                      kCrazyIvan, -3, 400, 50);
    n++;

    new shImplantIlk ("narcoleptor", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kUsuallyBuggy, shImplantIlk::kAnyBrain,
                      kNarcolepsy, -3, 400, 50);
    n++;

    /* regenerates hit points */
    new shImplantIlk ("tissue regenerator", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      kAutoRegeneration, -3, 400, 50);
    n++;

    new shImplantIlk ("psionic amplifier", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n].mColor, kFleshy,
                      kEnhanceable, shImplantIlk::kAnyBrain,
                      0, 0, 400, 50);
    n++;


/*
    new shImplantIlk ("search skillsoft", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n++].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      kAutoSearching, -3, 200, 50);

    new shImplantIlk ("repair skillsoft", "bionic implant", 
                      ImplantData[n].mDesc, ImplantData[n++].mColor, kFleshy,
                      0, shImplantIlk::kAnyBrain,
                      0, -3, 200, 50);
*/
                      
}


//constructor:
shImplantIlk::shImplantIlk (const char *name, const char *vaguename, 
                            const char *appearance, 
                            shColor color, 
                            shMaterialType material, int flags, 
                            Site site, int intrinsics,
                            int psimodifier, int cost, int prob)
{
    ImplantIlks.add (this);
    mType = kImplant;
    mParent = NULL;
    mName = name;
    mVagueName = vaguename;
    mAppearance = appearance;
    mGlyph.mChar = ObjectGlyphs[mType].mChar | ColorMap[color];
    mCost = cost;
    mMaterial = material;
    mFlags = flags;
    mProbability = prob;
    mWeight = 20;
    mSize = kDiminutive;
    mHardness = 5;
    mHP = 1;
    mSite = site;
    mWornIntrinsics |= intrinsics;
    mPsiModifier = psimodifier;
}



shObject *createImplant (char *desc,
                         int count, int bugginess, 
                         int enhancement, 
                         int charges)
{
    shImplantIlk *ilk;
    shObject *obj;

    ilk = (shImplantIlk *) (NULL == desc ? pickAnIlk (&ImplantIlks) 
                                         : findAnIlk (&ImplantIlks, desc));
    if (NULL == ilk) return NULL;

    obj = new shObject ();
    obj->mIlk = ilk;
    obj->mCount = 1;
    obj->mHP = ilk->mHP;
    
    if (-2 == bugginess) {
        int tmp = RNG (6);
        bugginess = (1 == tmp) ? 1 : (0 == tmp) ? -1 : 0;
        if (kUsuallyBuggy & ilk->mFlags && RNG (9)) {
            bugginess = -1;
        }
    }
    obj->setBugginess (bugginess);

    if (!obj->isEnhanceable ()) {
        obj->mEnhancement = 0;
    } else if (-22 != enhancement) {
        obj->mEnhancement = enhancement;
    } else if (obj->isOptimized ()) {
        obj->mEnhancement = RNG (1, 5);
    } else if (obj->isBuggy ()) {
        obj->mEnhancement = 0 - RNG (1, 5);
    } else { /* debugged */
        obj->mEnhancement = RNG (6) ? RNG (0, 2) + RNG (0, 3)
                                    : 0 - RNG (0, 5);
    }

    return obj;
}
