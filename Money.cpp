#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Interface.h"

void
initializeMoney ()
{
    MoneyIlk.mType = kMoney;
    MoneyIlk.mParent = NULL;
    MoneyIlk.mName = "buckazoid";
    MoneyIlk.mVagueName = "buckazoid";
    MoneyIlk.mAppearance = "buckazoid";
    MoneyIlk.mGlyph.mChar = '$' | ColorMap[kGreen];
    MoneyIlk.mMaterial = kPlastic;
    MoneyIlk.mFlags = kMergeable | kIdentified | kBugProof;
    MoneyIlk.mProbability = 1;
    MoneyIlk.mWeight = 1;
    MoneyIlk.mSize = kFine;
    MoneyIlk.mHardness = 10;
    MoneyIlk.mHP = 1;
    MoneyIlk.mCost = 1;
}

shObject *
createMoney (int count)
{
    shObject *money = new shObject ();

    if (count < 1) { count = 1; }

    money->mIlk = &MoneyIlk;
    money->mCount = count;
    money->mHP = MoneyIlk.mHP;
    money->mBugginess = 0;
    money->identify ();

    return money;
}



