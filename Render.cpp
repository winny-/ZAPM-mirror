#include <curses.h>

#include "Global.h"
#include "Util.h"
#include "Map.h"
#include "Interface.h"
#include "Creature.h"
#include "Monster.h"
#include "Hero.h"


static chtype ASCIITiles[kMaxTerrainType];
static chtype FeatureTiles[shFeature::kMaxFeatureType];


void
shInterface::initializeGlyphs ()
{
    memcpy (&mSqGlyphs[0], &ASCIITiles[0], sizeof (mSqGlyphs));

    mSqGlyphs[kStone] = ' ';
    mSqGlyphs[kVWall] = '#' | ColorMap[kBlue];
    mSqGlyphs[kHWall] = '#' | ColorMap[kBlue];
    mSqGlyphs[kVirtualWall] = '#' | ColorMap[kGreen];
    mSqGlyphs[kCaveWall] = '#' | ColorMap[kBrown];
    mSqGlyphs[kNWCorner] = '#' | ColorMap[kBlue];
    mSqGlyphs[kNECorner] = '#' | ColorMap[kBlue];
    mSqGlyphs[kSWCorner] = '#' | ColorMap[kBlue];
    mSqGlyphs[kSECorner] = '#' | ColorMap[kBlue];
    mSqGlyphs[kSewerWall] = '#' | ColorMap[kCyan];
    mSqGlyphs[kNTee] = '#' | ColorMap[kBlue];
    mSqGlyphs[kSTee] = '#' | ColorMap[kBlue];
    mSqGlyphs[kWTee] = '#' | ColorMap[kBlue];
    mSqGlyphs[kETee] = '#' | ColorMap[kBlue];

    mSqGlyphs[kFloor] = '.' | ColorMap[kBlue];
    mSqGlyphs[kCavernFloor] = '.' | ColorMap[kBrown];
    mSqGlyphs[kSewerFloor] = '.' | ColorMap[kBrown];
    mSqGlyphs[kVirtualFloor] = '.' | ColorMap[kGreen];
    mSqGlyphs[kSewage] = '~' | ColorMap[kGreen];
    mSqGlyphs[kVoid] = '^' | ColorMap[kBlue];

    FeatureTiles[shFeature::kDoorHiddenVert] = '#' | ColorMap[kBlue];
    FeatureTiles[shFeature::kDoorHiddenHoriz] = '#' | ColorMap[kBlue];
    FeatureTiles[shFeature::kDoorBerserkClosed] = '+' | ColorMap[kRed];
    FeatureTiles[shFeature::kDoorClosed] = '+' | ColorMap[kCyan];
    FeatureTiles[shFeature::kMachinery] = '&' | ColorMap[kCyan];
    FeatureTiles[shFeature::kMovingHWall] = '=' | ColorMap[kCyan];
    FeatureTiles[shFeature::kStairsUp] = '<' | ColorMap[kWhite];
    FeatureTiles[shFeature::kStairsDown] = '>' | ColorMap[kWhite];
    FeatureTiles[shFeature::kVat] = '{' | ColorMap[kBrightGreen];
    FeatureTiles[shFeature::kComputerTerminal] = '_' | ColorMap[kGreen];
    FeatureTiles[shFeature::kPit] = '^' | ColorMap[kYellow];
    FeatureTiles[shFeature::kAcidPit] = '^' | ColorMap[kBrightYellow];
    FeatureTiles[shFeature::kSewagePit] = '^' | ColorMap[kGreen];
    FeatureTiles[shFeature::kRadTrap] = '^' | ColorMap[kBrightGreen];
    FeatureTiles[shFeature::kTrapDoor] = '^' | ColorMap[kRed];
    FeatureTiles[shFeature::kHole] = '^' | ColorMap[kGray];
    FeatureTiles[shFeature::kWeb] = '"' | ColorMap[kCyan];
    FeatureTiles[shFeature::kPortal] = '^' | ColorMap[kMagenta];
    FeatureTiles[shFeature::kDoorOpen] = '\'' | ColorMap[kCyan];
    FeatureTiles[shFeature::kDoorBerserkOpen] = '\'' | ColorMap[kRed];


}


#define SQUARE_GLYPH(_sqptr) I->mSqGlyphs[(_sqptr)->mTerr]


//SUMMARY: draws the entire map 

void
shMapLevel::draw ()
{
    int x, y;

    wattrset (I->mMainWin, A_NORMAL);
    for (y = I->mY0; y < I->mYMax; y++) {
        for (x = I->mX0; x < I->mXMax; x++) {
            drawSq (x, y);
        }
    }
}


void
shMapLevel::drawSqTerrain (int x, int y, int forget /* = 0 */, 
                           int draw /* = 1 */)
{
    shFeature *f = getFeature (x, y);
    chtype c;

    if (f) {
        if (shFeature::kDoorClosed == f->mType && 
            shFeature::kBerserk & f->mDoor && 
            !f->mTrapUnknown) 
        {
            c = FeatureTiles [shFeature::kDoorBerserkClosed];
        } else if (shFeature::kDoorOpen == f->mType && 
                   shFeature::kBerserk & f->mDoor && 
                   !f->mTrapUnknown) 
        {
            c = FeatureTiles [shFeature::kDoorBerserkOpen];
        } else if (!f->mTrapUnknown || 
                   f->isDoor ())
        {
            c = FeatureTiles [f->mType];
        } else {
            goto doterrain;
        }
    } else {
doterrain:
        c = SQUARE_GLYPH (getSquare (x, y));
        if (isFloor (x, y) && !isLit (x, y, x, y)) {
            if (!forget) {
                setMemory (x, y, ' ');
                forget = 1;
            }
        }
    }
    if (0 == forget) {
        setMemory (x, y, c);
    }
    if (draw) {
        if (!f && isFloor (x, y) ) {
            c = SQUARE_GLYPH (getSquare (x, y));
            //c = '.' | ColorMap[kBlue] | A_DIM;
        }
        wmove (I->mMainWin, y - I->mX0, x - I->mX0);
        waddch (I->mMainWin, c);
    }
}

/* returns: 1 if something was drawn */
int
shMapLevel::drawSqCreature (int x, int y)
{
    shCreature *c = getCreature (x, y);

    wmove (I->mMainWin, y - I->mX0, x - I->mX0);
    if (c->mHidden > 0) {
        if (Hero.hasTelepathy () && c->hasMind () &&
            distance (&Hero, x, y) < 5 * Hero.mCLevel + 20)
        {
            c->mHidden *= -1;
        } else {
            switch (c->mMimic) {
            case shCreature::kObject:
                waddch (I->mMainWin, c->mMimickedObject->mGlyph.mChar);
                setMemory (x, y, c->mMimickedObject->mGlyph.mChar);
                return 1;
            default:
            case shCreature::kNothing: 
                return 0; /* invisible, or hiding under existing object */
            }
        }
    }
    waddch (I->mMainWin, c -> mGlyph.mChar);
    return 1;
}


void
shMapLevel::reveal ()
{
    int x; int y;
    
    for (y = 0; y < MAPMAXROWS; y++) {
        for (x = 0; x < MAPMAXCOLUMNS; x++) {
            if (' ' == getMemory (x, y)) {
                drawSqTerrain (x, y, 0, 1);
            } else {
                drawSq (x, y);
            }
        }
    }

}


int
shMapLevel::rememberedCreature (int x, int y)
{
    return ((chtype) 'I' | ColorMap[kBrightRed]) == getMemory (x, y);
}

void
shMapLevel::feelSq (int x, int y)
{
    int drawnchar = 0;
    wmove (I->mMainWin, y - I->mX0, x - I->mX0);

    if (isOccupied (x, y) && (Hero.mX != x || Hero.mY != y)) {
        waddch (I->mMainWin, 'I' | ColorMap[kBrightRed]);
        setMemory (x, y, 'I' | ColorMap[kBrightRed]);
        drawnchar = 1;
        return;
    }
    if (!isWatery (x, y) && countObjects (x, y) > 0) {
        /* display the lowest objecttype */
        shObjectVector *objs = getObjects (x, y);
        int i;
        int besttype = kMaxObjectType;
        shObject *bestobj = NULL;
        
        for (i = 0; i < objs->count (); i++) {
            if (objs->get (i) -> mIlk -> mType < besttype) {
                besttype = objs->get (i) -> mIlk -> mType;
                bestobj = objs->get (i);
            }
        }
        if (!drawnchar) {
            setMemory (x, y, bestobj->mIlk->mGlyph.mChar);
            waddch (I->mMainWin, bestobj->mIlk->mGlyph.mChar);
        }
    } else {
        drawSqTerrain (x, y, 0, !drawnchar);
    }
}




void
shMapLevel::drawSq (int x, int y, int forget /* = 0 */)
{
    int drawnchar = 0;
    wmove (I->mMainWin, y - I->mX0, x - I->mX0);

 /* if (0 == mVisibility[x][y]) { */

    if (!Hero.canSee (x, y)) {
        if (!Hero.isBlind () && isInLOS (x, y) && 
            getSpecialEffect (x, y)) 
        {
            if (drawSqSpecialEffect (x, y)) {
                return;
            }
        }
        if (   Hero.hasTelepathy () 
            && isOccupied (x, y) 
            && getCreature (x, y) -> hasMind ()
            && distance (&Hero, x, y) < 5 * (Hero.mCLevel + 10)) 
        {
            drawSqCreature (x, y);
        } else if (   Hero.hasNightVision ()
                   && isOccupied (x, y)
                   && Hero.canSee (getCreature (x, y)))
        {
            if (rememberedCreature (x, y)) {
                //FIXME: what terrain/object/etc did the hero remember here 
                //       prior to the 'I' monster that was here?
                setMemory (x, y, ' ');
            }
            drawSqCreature (x, y);
        } else if (   Hero.hasMotionDetection () 
                   && isOccupied (x, y) 
                   && getCreature (x, y) -> isMoving ()
                   && distance (&Hero, x, y) < 50) 
        {
            /* you see only a blip on the radar */
            wattrset (I->mMainWin, ColorMap[kRed]);
            waddch (I->mMainWin, '0'); 
            wattrset (I->mMainWin, A_NORMAL);
            drawnchar = 1;
        } else if (   Hero.hasNightVision ()
                   && isInLOS (x, y)
		      && ' ' != (getMemory (x, y) & A_CHARTEXT)
                   && getKnownFeature (x, y) 
                   && getKnownFeature (x, y) ->isDoor ())
        { /* kludge: show doors because, e.g. night vision might have
             revealed a creature on the other side that hero might not
             expect to see if she remembers the door was closed. */
            drawSqTerrain (x, y, 0);
        } else {
            if (Hero.mZ < 0) {
                if ('.' == (getMemory (x, y) & A_CHARTEXT)) {
                    waddch (I->mMainWin, ' ');
                } else {
                    waddch (I->mMainWin, getMemory (x, y));
                    //mvwchgat (I->mMainWin, y - I->mX0, x - I->mX0,
                    //          1, A_DIM, COLOR_WHITE, NULL);
                }

            } else {
                if (Flags.mShowLOS && 
                    '.' == (getMemory (x, y) & A_CHARTEXT)) 
                {
                    waddch (I->mMainWin, ' ');
                } else {
                    waddch (I->mMainWin, getMemory (x, y));
                }
            }
        }
        return;
    }
    if (getSpecialEffect (x, y)) {
        drawnchar = drawSqSpecialEffect (x, y);
    } 
    if (isOccupied (x, y) && !drawnchar) {    
        drawnchar = drawSqCreature (x, y);
    }
    if (!isWatery (x, y) && countObjects (x, y) > 0) {
        /* display the lowest objecttype */
        shObjectVector *objs = getObjects (x, y);
        int i;
        int besttype = kMaxObjectType;
        shObject *bestobj = NULL;
        
        for (i = 0; i < objs->count (); i++) {
            if (objs->get (i) -> mIlk -> mType < besttype) {
                besttype = objs->get (i) -> mIlk -> mType;
                bestobj = objs->get (i);
            }
        }
        if (0 == forget) {
            //FIXME: if creature was a mimic, should remember
            // the mimicked object instead of this obj!
            setMemory (x, y, bestobj->mIlk->mGlyph.mChar);
        }
        if (!drawnchar) {
            waddch (I->mMainWin, bestobj->mIlk->mGlyph.mChar);
            drawnchar = 1;
        }
    } else {
        drawSqTerrain (x, y, forget, !drawnchar);
    }
}


int
shMapLevel::drawSqSpecialEffect (int x, int y)
{
    shSpecialEffect e = getSpecialEffect (x, y);
    switch (e) {
    case kNone:
    case kInvisibleEffect:
        return 0;
    case kExplosionEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kGray]);
        break;
    case kRadiationEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kGreen]);
        break;
    case kHeatEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kBrightRed]);
        break;
    case kColdEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kBlue]);
        break;
    case kPoisonEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kRed]);
        break;
    case kLaserBeamHorizEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '-' | ColorMap[kBrightCyan]);
        break;
    case kLaserBeamVertEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '|' | ColorMap[kBrightCyan]);
        break;
    case kLaserBeamBDiagEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '\\' | ColorMap[kBrightCyan]);
        break;
    case kLaserBeamFDiagEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '/' | ColorMap[kBrightCyan]);
        break;
    case kLaserBeamEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kBrightCyan]);
        break;
    case kRailHorizEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '-' | ColorMap[kBrightRed]);
        break;
    case kRailVertEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '|' | ColorMap[kBrightRed]);
        break;
    case kRailBDiagEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '\\' | ColorMap[kBrightRed]);
        break;
    case kRailFDiagEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '/' | ColorMap[kBrightRed]);
        break;
    case kRailEffect:
        //wmove (I->mMainWin, y, x);
        //waddch (I->mMainWin, '*' | ColorMap[kBrightRed]);
        break;
    case kDisintegrationEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kBlue] | A_REVERSE);
        break;
    case kBinaryEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, (RNG(2) ? '1' : '0') | ColorMap[kBrightGreen] | A_REVERSE);
        break;     
    case kBugsEffect:
    {
        const char bugs[] = "~`@#$%^&*()-_=+:;'\"[]{}|\\/?>.<,";
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, bugs[RNG(strlen(bugs))] | ColorMap[RNG(kBrightCyan)] | A_REVERSE);
        break;     
    }
    case kVirusesEffect:
        wmove (I->mMainWin, y, x);
        waddch (I->mMainWin, '*' | ColorMap[kCyan]);
        break;     
    default:
      return 0;
    }
    return 1;
}


void
shInterface::drawSideWin ()
{
    char linebuf[17];
    linebuf[16] = '\0';

    if (!Hero.mProfession) { return; }

    werase (I->mSideWin);
    wmove (I->mSideWin, 0, 0);
    wattrset (I->mSideWin, ColorMap[kWhite]);
    waddnstr (I->mSideWin, Hero.mName, 16);

    wmove (I->mSideWin, 1, 0);
    wattrset (I->mSideWin, A_NORMAL);
    snprintf (linebuf, 16, "%s", Hero.mProfession->mTitles[Hero.mCLevel/3]);
    waddstr (I->mSideWin, linebuf);

    wmove (I->mSideWin, 2, 0);
    wattrset (I->mSideWin, A_NORMAL);
    snprintf (linebuf, 16, "XL %2d:%d", Hero.mCLevel, Hero.mXP);
    waddstr (I->mSideWin, linebuf);

    wmove (I->mSideWin, 4, 0);
    wattrset (I->mSideWin, A_NORMAL);
    snprintf (linebuf, 16, "Str %2d  Int %2d", Hero.getStr (), Hero.getInt ());
    waddstr (I->mSideWin, linebuf);
    
    wmove (I->mSideWin, 5, 0);
    snprintf (linebuf, 16, "Con %2d  Wis %2d", Hero.getCon (), Hero.getWis ());
    waddstr (I->mSideWin, linebuf);
    
    wmove (I->mSideWin, 6, 0);
    snprintf (linebuf, 16, "Agi %2d  Cha %2d", Hero.getAgi (), Hero.getCha ());
    waddstr (I->mSideWin, linebuf);
    
    wmove (I->mSideWin, 7, 0);
    snprintf (linebuf, 16, "Dex %2d", Hero.getDex ());
    waddstr (I->mSideWin, linebuf);

    wmove (I->mSideWin, 9, 0);
    if (Hero.mSpeed < 100) {
        snprintf (linebuf, 16, "Speed %4d", Hero.mSpeed - 100);
        waddstr (I->mSideWin, linebuf);
    } else if (Hero.mSpeed >= 100) {
        snprintf (linebuf, 16, "Speed %+4d", Hero.mSpeed - 100);
        waddstr (I->mSideWin, linebuf);
    }

    
    wmove (I->mSideWin, 10, 0);
    snprintf (linebuf, 16, "Armor  %3d", Hero.mAC);
    waddstr (I->mSideWin, linebuf);

    wmove (I->mSideWin, 11, 0);
    if (Hero.hasHealthMonitoring () && 
        Hero.mHP < Hero.hpWarningThreshold ()) 
    {
        wattrset (I->mSideWin, ColorMap[kBrightRed]);
    }
    snprintf (linebuf, 16, "HitPts %3d(%d)", Hero.mHP, Hero.mMaxHP);
    waddstr (I->mSideWin, linebuf);
    wattrset (I->mSideWin, A_NORMAL);

    {
        int en, enmax;
        en = Hero.countEnergy (&enmax);

        wmove (I->mSideWin, 12, 0);
        wattrset (I->mSideWin, ColorMap[kCyan]);
        snprintf (linebuf, 16, "Energy%4d", en);
        if (enmax)
            snprintf (linebuf+10, 6, "(%d)", enmax);
        waddstr (I->mSideWin, linebuf);
        wattrset (I->mSideWin, A_NORMAL);
    }

    wmove (I->mSideWin, 13, 0);
    snprintf (linebuf, 16, "$%d", Hero.countMoney ());
    wattrset (I->mSideWin, ColorMap[kGreen]);
    waddstr (I->mSideWin, linebuf);
    wattrset (I->mSideWin, A_NORMAL);

    if (Hero.mWeapon && Hero.mWeapon->isA (kWeapon)) {
        shWeaponIlk *ilk = (shWeaponIlk *) Hero.mWeapon->mIlk;
        shObjectIlk *ammo = 
            Hero.mWeapon->isA (kRayGun) ? NULL : ilk->mAmmoType;
        int i, n = 0;

        if (ammo) {
            for (i = 0; i < Hero.mInventory->count (); i++) {
                shObject *obj = Hero.mInventory->get (i);
                if (obj->isA (ammo)) {
                    n += obj->mCount;
                }
            }
            wmove (I->mSideWin, 13, 7);
            waddch (I->mSideWin, ammo->mGlyph.mChar);
            snprintf (linebuf, 15, "%d", n);
            waddstr (I->mSideWin, linebuf);
        }
    }

    { /* Conditions */
        int n = 14;
        const char *condition = NULL;
        wattrset (I->mSideWin, ColorMap[kYellow]);

        if (Hero.isBlind ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Blind");
        }
        if (Hero.getStoryFlag ("superglued tongue")) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Mute");
        }
        if (Hero.isStunned ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Stunned");
        }
        if (Hero.isConfused ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Confused");
        }
        if (Hero.isHosed ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Hosed");
        }
        if (Hero.isSickened ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Sickened");
        }
        if (Hero.isFrightened ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Frightened");
        }
        if (Hero.getStoryFlag ("radsymptom") > 1) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Radiation Sick");
        }
        if (Hero.getStoryFlag ("impregnation") > 0) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Pregnant");
        }
        if (Hero.isViolated ()) {
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, "Sore");
        }
        
        switch (Hero.getEncumbrance ()) {
        case kOverloaded: condition = "Overloaded"; break;
        case kOvertaxed: condition = "Overtaxed"; break;
        case kStrained: condition = "Strained"; break;
        case kBurdened: condition = "Burdened";
        }
        if (condition) {
            char condstr[16];

            strncpy (condstr, condition, sizeof(condstr));
            wmove (I->mSideWin, n++, 0);
            waddstr (I->mSideWin, condstr);
        }
    }
    wattrset (I->mSideWin, A_NORMAL);
        
    wmove (I->mSideWin, GodMode ? 17:19, 0);
    wattrset (I->mSideWin, A_BOLD);
    snprintf (linebuf, 16, "%5s %d", Level->mName, Level->mDLevel);
    waddstr (I->mSideWin, linebuf);

#ifdef SH_DEBUG
    if (GodMode) {
/*
        wmove (I->mSideWin, 17, 0);
        wattrset (I->mSideWin, A_NORMAL);
        snprintf (linebuf, 16, "wt %5d/%5d", Hero.mWeight, Hero.mCarryingCapacity);
        waddstr (I->mSideWin, linebuf);
*/
        wmove (I->mSideWin, 18, 0);
        wattrset (I->mSideWin, A_NORMAL);
        snprintf (linebuf, 16, "rad %d ", Hero.mRad);
        waddstr (I->mSideWin, linebuf);
        
        wmove (I->mSideWin, 19, 0);
        wattrset (I->mSideWin, A_NORMAL);
        snprintf (linebuf, 16, "%d ", Clock);
        waddstr (I->mSideWin, linebuf);
    }
#endif
    wnoutrefresh (mSideWin);
}


void
shInterface::drawLog ()
{
    if (mDiagWin) 
        wnoutrefresh (mDiagWin);
    touchwin (mLogWin);
    wnoutrefresh (mLogWin);
    doupdate ();
}


void
shInterface::drawScreen ()
{
    Level->draw ();
    drawSideWin ();
    wnoutrefresh (mMainWin);
    drawLog (); /* calls doupdate() for us */
}


void
shInterface::refreshScreen ()
{
    wnoutrefresh (mMainWin);
    drawLog ();
}


void
shInterface::cursorOnHero ()
{
    wmove (mMainWin, Hero.mY, Hero.mX);
}


void
shInterface::cursorOnXY (int x, int y)
{
    wmove (mMainWin, y, x);
}
