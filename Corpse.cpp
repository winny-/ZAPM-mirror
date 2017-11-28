#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Interface.h"
#include "Monster.h"


void
initializeWreck ()
{
    WreckIlk.mType = kDevice;
    WreckIlk.mParent = NULL;
    WreckIlk.mName = "wreck";
    WreckIlk.mAppearance = "wreck";
    WreckIlk.mGlyph.mChar = '&' | ColorMap[kCyan];
    WreckIlk.mMaterial = kSteel;
    WreckIlk.mFlags = kMergeable | kIdentified | kBugProof;
    WreckIlk.mProbability = 0;
    WreckIlk.mWeight = 40000;
    WreckIlk.mSize = kMedium;
    WreckIlk.mHardness = 10;
    WreckIlk.mHP = 10;
    WreckIlk.mCost = 10;
}


shObject *
createCorpse (shCreature *m)
{
    if (m->isRobot ()) {
        shObject *wreck = new shObject ();
        
        wreck->mIlk = &WreckIlk;
        wreck->mCount = 1;
        wreck->mHP = WreckIlk.mHP;
        wreck->mCorpseIlk = ((shMonster *) m)->mIlk;
        return wreck;
    }
    return NULL;
}



