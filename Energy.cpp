#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Interface.h"

void
initializeEnergy ()
{
    EnergyCellIlk.mType = kEnergyCell;
    EnergyCellIlk.mParent = NULL;
    EnergyCellIlk.mName = "energy cell";
    EnergyCellIlk.mVagueName = "energy cell";
    EnergyCellIlk.mAppearance = "energy cell";
    EnergyCellIlk.mGlyph.mChar = '*' | ColorMap[kBrightCyan];
    EnergyCellIlk.mMaterial = kSteel;
    EnergyCellIlk.mFlags = kMergeable | kIdentified | kBugProof;
    EnergyCellIlk.mProbability = 1;
    EnergyCellIlk.mWeight = 10;
    EnergyCellIlk.mSize = kTiny;
    EnergyCellIlk.mHardness = 10;
    EnergyCellIlk.mHP = 1;
    EnergyCellIlk.mCost = 1;
}

shObject *
createEnergyCell (int count)
{
    shObject *cell = new shObject ();

    if (count < 1) { count = NDX (8, 10); }

    cell->mIlk = &EnergyCellIlk;
    cell->mCount = count;
    cell->mHP = EnergyCellIlk.mHP;
    cell->mBugginess = 0;
    cell->identify ();

    return cell;
}

