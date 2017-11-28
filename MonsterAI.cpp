#include "Global.h"
#include "Util.h"
#include "Monster.h"
#include "Hero.h"


void
shMonster::newEnemy (shCreature *c)
{
    if (c->isHero ()) {
        if (isPet ()) {

        } else {
            makeAngry ();
        }
    } else if (c->isPet ()) {
        mEnemyX = c->mX;
        mEnemyY = c->mY;
    }
    if (isA ("alien egg")) {
        ++mAlienEgg.mHatchChance;
    }
}


void
shMonster::makeAngry ()
{
    if (!isHostile ()) {
        if (mHP == mMaxHP && isA ("monolith")) {
            return;
        }
        I->p ("%s gets angry!", the ());
        mDisposition = kHostile;
        mTactic = kNewEnemy;
    }
}


void
shMonster::newDest (int x, int y)
{
    mDestX = x;
    mDestY = y;
}


/* try to move the creature one square in the given direction
   RETURNS: elapsed time, or 
            -2 if the creature dies
*/

int
shCreature::doMove (shDirection dir)
{
    int x = mX;
    int y = mY;
    int speed;

    speed = isDiagonal (dir) ? DIAGTURN : FULLTURN;

    mDir = dir;

    if (0 == speed) {
        return 200;
    }

    if (!mLevel->moveForward (dir, &x, &y)) {
        I->p ("Probable bug: a monster tried to move off the map!");
        /* do nothing */
        return 100;
    }

    if (mHidden)
        revealSelf ();
    
    if (mLevel->isObstacle (x, y)) {
        if (Hero.canSee (this)) {
            if (Hero.canSee (x, y)) {
                I->p ("%s bumps into %s.", the (),
                      mLevel->getSquare (x, y) -> the ());
            } else {
                I->p ("%s bumps into something.", the ());
            }
        }
        /* you can't tell when an unseen monster bumps into an obstacle even if
           you can see the obstacle, so don't print a message */
        return speed / 2;
    }
    else if (mLevel->isOccupied (x, y)) {
        if (&Hero == mLevel->getCreature (x,y)) {
            I->p ("%s bumps into you!", the ());
        } else if (Hero.canSee (this)) {
            if (Hero.canSee (x, y)) {
                I->p ("%s bumps into %s.", the (), 
                      mLevel->getCreature (x, y) -> the ());
            } else {
                I->p ("%s bump into something.", the ());
            }
        } else if (Hero.canSee (x, y)) {
            I->p ("Something bumps into %s.", the ());
        }
        return speed / 2;
    }
    else {
        I->dirty ();
        if (mLevel->moveCreature (this, x, y)) {
            /* we moved and died. */
            return -2;
        }
    }
    return speed;
}


int
shMonster::setupPath ()
{
    if (shortestPath (mX, mY, mDestX, mDestY, mPlannedMoves, 100)) {
        mPlannedMoveIndex = 0;
        return 1;
    } else {
        mPlannedMoveIndex = -1;
        return 0;
    }
}


#if 0
int
shMonster::patchPath ()
{
    int len;
    int i, x, y;
    shDirection patch[15];
    
    for (i = 0, x = mX, y = mY;
         i < 5 && x != mDestX && y != mDestY;
         i++)
    {
        mLevel->moveForward (mPlannedMoves[mPlannedMoveIndex+i], &x, &y);
    }
             
    len = shortestPath (mX, mY, x, y, &patch, 15);
    if (len) {
        memmove (&mPlannedMoves[mPlannedMoveIndex+len],
                 &mPlannedMoves[mPlannedMoveIndex+i],
                 (100 - i) * sizeof (shDirection));
        memcpy (&mPlannedMoves[mPlannedMoveIndex], &patch[0], 
                len * sizeof (shDirection));
        return 1;
    }
    return 0;
}
#endif


/* Attempts to move one square towards destination, with simple obstacle
   avoidance.  This is a quick and dirty routine for use in close combat
   situations when the destination is within sight.

   returns ms elapsed, or -1 if nothing happened, or -2 if monster died
*/

int
shMonster::doQuickMoveTo (shDirection dir /* = kNoDirection */)
{

    int i;
    int dx = 0;
    int dy = 0;
    shDirection dirlist[9];

    if (kNoDirection == dir) {
        if (mX == mDestX && mY == mDestY) {
            /* we're already there */
            mTactic = kReady;
            return -1;
        }
        dir = vectorDirection (mX, mY, mDestX, mDestY);
        I->debug ("quickmoveto %d, %d", mDestX, mDestY);
    } 
//    I->debug ("quickmoveto %s");

    
    dirlist[0] = dir;  /* first try to move directly towards destination */
    dirlist[1] = mDir; /* then try just repeating the last move we made */
    dirlist[2] = (shDirection) ((dir + 1) % 8); /* then try moving in nearby */
    dirlist[3] = (shDirection) ((dir + 7) % 8); /* directions */
    dirlist[4] = (shDirection) ((dir + 6) % 8);
    dirlist[5] = (shDirection) ((dir + 2) % 8);
    dirlist[6] = (shDirection) ((dir + 3) % 8);
    dirlist[7] = (shDirection) ((dir + 5) % 8);
    dirlist[8] = (shDirection) ((dir + 4) % 8);

    for (i = 0; i < 9; i++) {
        dir = dirlist[i];
        dx = 0; 
        dy = 0;
        if (kNoDirection == dir) {
            continue;
        }
        mLevel->moveForward (dir, &dx, &dy);
        if (mLevel->isObstacle (mX + dx, mY + dy)) {
            int elapsed = clearObstacle (mX + dx, mY + dy);
            if (elapsed > 0) { 
                return elapsed;
            }
            continue;
        }
        if (mLevel->isOccupied (mX + dx, mY + dy)) {
            if ((4 == i && RNG (4)) ||
                (6 == i && RNG (4)))
            { /* most of the time, it's better to wait for the other creature 
                 to get out of the way */
                return 100;
            }
            continue;
        }
        return doMove (dir);
    }            

    /* we seem to be stuck here, give up */
    mTactic = kReady;
    return HALFTURN;
}


/* attempts to clear an obstacle in square x, y
   returns time elapsed on success, -1 on failure
*/

int 
shMonster::clearObstacle (int x, int y)
{
    shFeature *f;

    I->debug ("  clear obstacle");

    f = mLevel->getFeature (x, y);
    if (f && shFeature::kDoorClosed == f->mType) {
        if (!openDoor (x, y)) {
            return -1;
        } 
        if (Hero.canSee (x, y)) {
            Hero.interrupt ();
        }
        return FULLTURN;
    } 
    return -1;          
}


/* move one square towards the destination using computed pathfinding data
   returns ms elapsed, or -1 if nothing happened, or -2 if monster died
*/
int
shMonster::doMoveTo ()
{
    shDirection dir;
    int dx = 0;
    int dy = 0;

    I->debug ("  move to destination %d, %d", mDestX, mDestY);

    if (mX == mDestX && mY == mDestY) {
        /* we're already there */
        mTactic = kReady;
        return -1;
    }

    if (mPlannedMoveIndex > 15 && !setupPath()) {
        mTactic = kReady;
        return -1;
    }

    dir = mPlannedMoves[mPlannedMoveIndex];
    mLevel->moveForward (dir, &dx, &dy);
    
    if (mLevel->isObstacle (mX + dx, mY + dy)) {
        int elapsed = clearObstacle (mX + dx, mY + dy);
        if (-1 == elapsed) {
            if (!setupPath ()) {
                mTactic = kReady;
            }
        }
        return elapsed;
    }
    if (mLevel->isOccupied (mX + dx, mY + dy)) {
        if (RNG (4)) {
            /* best bet: just wait for the creature to move out of the way */
            return 500;
        } else  {
            mTactic = kReady;
            return doQuickMoveTo (dir);
        } 
        return -1;
    }

    ++mPlannedMoveIndex;
    return doMove (dir);
}

/*******
  Nethack-style quick and simple AI

  consider each adjacent square 

 *******/

int
shMonster::findSquares (int flags, shCoord *coord, int *info)
{
    int n = 0;
    int x, y;
    
    int gridbug = isA ("grid bug");

    for (y = mY - 1; y <= mY + 1; y++) {
        for (x = mX - 1; x <= mX + 1; x++) {
            *info = 0;
            if (!mLevel->isInBounds (x, y)) 
                continue;

            if (gridbug && abs (x-mX) + abs (y-mY) > 1){
                continue;
            }

            shCreature *c = mLevel->getCreature (x, y);
            shFeature *f = mLevel->getFeature (x, y);
            if (f) {
                if (f->isDoor () && 
                    !(f->isOpenDoor () || f->isAutomaticDoor ()))
                {
                    if (flags & kDoor) {
                        *info |= kDoor;
                    } else {
                        continue;
                    }
                } else if (f->isObstacle ()) {
                    continue;
                }
                
                if (f->isTrap () && 
                    (isPet () ? !f->mTrapUnknown : !f->mTrapMonUnknown))
                {
                    if (isFlying () && (shFeature::kPit == f->mType ||
                                        shFeature::kAcidPit == f->mType ||
                                        shFeature::kTrapDoor == f->mType ||
                                        shFeature::kHole == f->mType))
                    {
                        /* this is an acceptable square, keep going */
                    } else if (canSwim () && 
                               shFeature::kSewagePit == f->mType)
                    {
                        
                    } else if (f->isBerserkDoor () && mHP > 8) {
                        /* risk it */
                    } else if (flags & kTrap) {
                        *info |= kTrap;
                    } else {
                        continue;
                    }
                }
                
                if (canHideUnder (f) && flags & kHidingSpot) {
                    *info |= kHidingSpot;
                }
            }
            
            if (!mLevel->appearsToBeFloor (x, y)) {
                if (flags & kWall) {
                    *info |= kWall;
                } else {
                    continue;
                }
            } else if (kSewage == mLevel->getSquare (x, y) -> mTerr) {
                if (canHideUnderWater () && flags & kHidingSpot) {
                    *info |= kHidingSpot;
                }
            }
            
            if (c == this) {
                
            } else if (c && c->isHero ()) {
                if (flags & kHero) {
                    *info |= kHero;
                } else {
                    continue;
                }
            } else if (c && !c->isHero ()) {
                if (flags & kMonster) {
                    *info |= kMonster;
                } else {
                    continue;
                }
            }
            
            if ((x == Hero.mX || y == Hero.mY ||
                 x - y == Hero.mX - Hero.mY ||
                 x + y == Hero.mX + Hero.mY) 
                && mLevel->isInLOS (x, y))
            {
                if (flags & kLinedUp) {
                    *info |= kLinedUp;
                } else {
                    continue;
                }
            }
            
            if (flags & kFreeItem || flags & kHidingSpot) {
                shObjectVector *v = mLevel->getObjects (x, y);
                shObject *obj;
                int i;
                
                if (v) {
                    for (i = 0; i < v->count (); i++) {
                        obj = v->get (i);
                        if (canHideUnder (obj)) {
                            *info |= kHidingSpot;
                        }
                        if (flags & kFreeMoney && obj->isA (kMoney)) {
                            *info |= kFreeMoney;
                        } else if (flags & kFreeWeapon && 
                                   (obj->isA (kWeapon) || obj->isA (kRayGun))) 
                        {
                            *info |= kFreeWeapon;
                        } else if (flags &kFreeArmor && obj->isA (kArmor)) {
                            *info |= kFreeArmor;
                        } else if (flags &kFreeEnergy && 
                                   obj->isA (kEnergyCell)) 
                        {
                            *info |= kFreeEnergy;
                        }                           
                    }
                }
            }
            
            coord->mX = x;
            coord->mY = y;
            ++coord;
            ++info;
            ++n;
        }
    }
    return n;
}


void
shMonster::doRangedAttack (shAttack *atk, shDirection dir)
{
    const char *buf = the ();

    Hero.interrupt ();

    if (mHidden) {
        revealSelf ();
    }

    switch (atk->mEffect) {
    case shAttack::kBeam:
        switch (atk->mType) {
        case shAttack::kBreatheFire: I->p ("%s breathes flames!", buf); break;
        case shAttack::kBreatheBugs: I->p ("%s breathes bugs!", buf); break;
        case shAttack::kBreatheViruses: 
            I->p ("%s breathes viruses!", buf); break;
        case shAttack::kBreatheTime: I->p ("%s breathes time!", buf); break;
        case shAttack::kBreatheTraffic: 
            I->p ("%s breathes megabytes!", buf); break;
        case shAttack::kLaser:
            I->p ("%s shoots a laser!", buf); break;
        }
        Level->areaEffect (atk, NULL, mX, mY, dir, this, 0);
        break;
    default:
        I->debug ("UNIMPLEMENTED RANGED ATTACK");
    }

}


/* attempt a melee attack against the target
   returns:   1 if target is eliminated (dies, teleports, etc)
              0 if target was attacked
             -1 if attack was a miss
             -2 if attacker dies
             -3 if no attack was made
   modifies: elapsed is set to elapsed AP
*/
int
shMonster::doAttack (shCreature *target, int *elapsed)
{
    const char *t_monster = the ();
    const char *t_weapon;
    
    shAttack *atk = NULL;
                
    if (0 == mIlk->mAttacks.count ()) {
        *elapsed = HALFTURN;
        return -3;
    }
    if (kAlien == mType && 
        target->isHero () &&
        Hero.getStoryFlag ("impregnation") &&
        mHP == mMaxHP)
    { /* Aliens won't attack creatures that have already 
         been impregnated (until they've taken damage) */
        *elapsed = HALFTURN;
        return -3;
    }

    if (mHidden) {
        revealSelf ();
    }
    
    if (NULL == mWeapon) {
        /* randomly pick a physical attack: */
        atk = mIlk->mAttacks.get (RNG (mIlk->mAttacks.count ()));
    } else if (mWeapon->isMeleeWeapon ()) {
        atk = & ((shWeaponIlk *) mWeapon->mIlk) -> mAttack;
    } else {
        if (target->isHero () && Hero.canSee (this)) {
            t_weapon = mWeapon->her (this);
            I->p ("%s swings %s at you!", t_monster, t_weapon);
        }
        atk = &ImprovisedObjectAttack;
    }
    
    if (NULL == atk) {
        *elapsed = HALFTURN;
        return -3;
    } else {
        *elapsed = atk->mAttackTime;
        if (shAttack::kExplode == atk->mType) {
            if (target->isHero () || Hero.canSee (this)) {
                I->p ("%s explodes!", t_monster);
            }
            die (kSlain, NULL);
            return -2;
        } else if (shAttack::kSpawnPrograms == atk->mType) {
            die (kSlain, NULL);
            return -2;
        }
        return meleeAttack (mWeapon, atk, target->mX, target->mY);
    }
}


/* try to return attack against a pet who attacked from mEnemyX,Y */
int
shMonster::doRetaliate ()
{
    shCreature *c = mLevel->getCreature (mEnemyX, mEnemyY);
    int elapsed;
    int result;

    I->debug ("retaliating against %d %d", mEnemyX, mEnemyY);
    if (c && c->isPet ()) {
        result = doAttack (c, &elapsed);
        if (-2 == result) {
            return -2;
        } else if (-3 != result) {
            mEnemyX = -1;
        }
        return elapsed;
    } else {
        mEnemyX = -1;
    }
    return 0;
}


int
shMonster::doWander ()
{
    int i, n;
    shCoord coord[9];
    int info[9];
    int best = -1;
    int score, bestscore = -9999;
    int flags = 0;
    int res = -1;
    int dist;
    int health = mHP * 3 / mMaxHP;
    int gridbug = isA ("grid bug");

    int val_linedup;
    int val_adjacent;
    int val_near;
    int val_medium;
    int val_far;
    int val_owntrack;   // where this monster has been
    int val_htrack;     // where the hero has been
    int val_same;
    int val_money;
    int val_weapon;
    int val_armor;
    int val_energy;
    int val_crowd = isMultiplier () ? -2 : 0;
    int val_hidingspot; // obj or feature big enough to hide under */

    int elapsed;
    int hasrangedweapon = 0;

    if (mDestX < 0) {
        mLevel->findUnoccupiedSquare (&mDestX, &mDestY);
    }
    
    if (!isHostile ()) {
        flags = kLinedUp | kDoor;
        val_adjacent = 0;
        val_linedup = 0;
        val_near = 0;
        val_medium = 0;
        val_far = 0;
        val_owntrack = -2;
        val_htrack = -3;
        val_same = -5;
        val_money = 0;
        val_weapon = 0;
        val_armor = 0;
        val_energy = 0;
        val_hidingspot = 0;
    } else if (getMutantLevel () && RNG (3) && 
               (res = useMutantPower ())) 
    {
         return res;
    } else if (canSee (&Hero) || canSmell (&Hero)) {
        mDestX = Hero.mX;
        mDestY = Hero.mY;

        if (getInt () > 7 && !RNG(3)) {
            // tell other monsters where the hero is
            mLevel->alertMonsters (mX, mY, 50, Hero.mX, Hero.mY);
            I->debug ("  alerting monsters near %d %d", mX, mY);
        }

        res = readyWeapon ();
        if (-1 != res) {
            return res;
        }

        val_money = 0;
        val_weapon = (numHands () && getInt () > 7) ? 35 : 0;
        val_armor = 0;
        val_energy = (numHands ()) ? 30 : 0;
        val_hidingspot = canSee (&Hero) ? 0 : 40;

        if (mIlk->mRangedAttacks.count () && 
            canSee (&Hero) &&
            distance (this, &Hero) < 60) 
        {
            shAttack *atk = mIlk->mRangedAttacks.get (
                RNG (mIlk->mRangedAttacks.count ()));
            if (!RNG (atk->mProb)) {
                shDirection dir = linedUpDirection (this, &Hero);
                if (kNoDirection != dir) {
                    doRangedAttack (atk, dir);
                    return atk->mAttackTime;
                }
            }
            hasrangedweapon = 1;
            val_adjacent = 0;
        }
        
        if (mWeapon && !mWeapon->isMeleeWeapon ()) {
            if (!hasAmmo (mWeapon)) {
                return readyWeapon ();
            }

            int maxrange;
            if (mWeapon->isThrownWeapon ()) 
                maxrange = 60; //FIXME: calculate range
            else if (mWeapon->isA (kRayGun)) 
                maxrange = 45;
            else 
                maxrange = 120;

            shDirection dir = linedUpDirection (this, &Hero);
            if (kNoDirection != dir && 
                RNG (10) && 
                canSee (&Hero) &&
                distance (this, &Hero) < maxrange) 
            {
                if (mHidden) {
                    revealSelf ();
                }
                if (mWeapon->isThrownWeapon ()) {
                    shObject *obj = removeOneObjectFromInventory (mWeapon);
                    elapsed = throwObject (obj, dir);
                } else {
                    elapsed = shootWeapon (mWeapon, dir);
                }
                if (-2 == elapsed) {
                    die (kSuicide);
                }
                return elapsed;
            }

            if (mHidden) {
                return HALFTURN;
            }

            flags = kLinedUp | kDoor;
            val_linedup = 15;    /* try to line up a shot */
            val_adjacent = -10;
            
            if (isFleeing () || health < 1) { /* hurt, retreat */
                val_near = 2;
                val_medium = 1;
                val_far = 0;
                val_linedup = -5;
                val_htrack = -3;
            } else { /* close in for a shot */
                val_near = 0;
                val_medium = 1;
                val_far = -1;
                val_htrack = 3;
            } 

            val_owntrack = -3;
            val_same = -2;
        } else {
            if (areAdjacent (this, &Hero) && !isFleeing ()) {
                if (gridbug && abs (Hero.mX-mX) + abs (Hero.mY-mY) > 1) {
                    /* can't attack diagonally */
                } else if (-2 == doAttack (&Hero, &elapsed)) {
                    return -2;
                } else {
                    return elapsed;
                }
            }
        
            if (mHidden) {
                return HALFTURN;
            }
        
            if (isMultiplier () && !RNG (mHP > 1 ? 6 : 12) &&
                mLevel->countAdjacentCreatures (mX, mY) < 5 &&
                mLevel->mCrList.count () < 150) 
            {
                int x = mX;
                int y = mY;
                if (0 == mLevel->findAdjacentUnoccupiedSquare (&x, &y) &&
                    mLevel->countAdjacentCreatures (x, y) < 5) 
                {
                    shMonster *m = new shMonster (mIlk);
                    if (0 == mLevel->putCreature (m, x, y)) {
                        if (mHP > 1) {
                            --mHP;
                            --mMaxHP;
                        }
                        //m->mHP = m->mMaxHP = mHP;
                    } else {
                        /* delete m; */
                    }
                    return LONGTURN;
                }
            }

            flags = kLinedUp | kDoor;
            if (hasrangedweapon) {
                val_linedup = 2;
            } else if (mCLevel < 2) {
                val_linedup = 0;
            } else if (getInt () < 7) { 
                val_linedup = 1;
            } else if (getInt () < 10) {
                val_linedup = -3;
            } else {
                val_linedup = -10;
            }

            if (isFleeing ()) {
                val_near = 1;
                val_medium = 1;
                val_far = 1;
                val_linedup = -5;
                val_htrack = -3;
                val_adjacent = -30;
            } else {
                val_adjacent = 30;
                val_near = -1;
                val_medium = -1;
                val_far = -1;
                val_htrack = 3;
            }
            val_owntrack = -10;
            val_same = -10;
        } 
    } else { /* don't know where hero is */
        if (mHidden) {
            return HALFTURN;
        }
    
        if (canMimicObjects ()) {
            mimicSomething ();
            return FULLTURN;
        }

        if (mX == mDestX && mY == mDestY) {
            /* already here, what now? */
            if (!mLevel->moveForward (mDir, &mDestX, &mDestY)) {
                mDestX = mX + RNG (13) - 6;
                mDestY = mY + RNG (9) - 4;
                mLevel->findNearbyUnoccupiedSquare (&mDestX, &mDestY);
            }
        }

        flags = kDoor | kLinedUp;
        val_linedup = 0;
        val_adjacent = 0;
        val_near = -3;
        val_medium = -2;
        val_far = -1;
        val_owntrack = -5;
        val_htrack = 10;
        val_same = -20;
        val_money = 0;
        val_weapon = numHands () ? 5 : 0;
        val_energy = (numHands ()) ? 20 : 0;
        val_armor = 0;
        val_hidingspot = 40;
        if (!RNG (40)) { /* wander somewhere new */
            mDestX = mX + RNG (13) - 6;
            mDestY = mY + RNG (9) - 4;
            mLevel->findNearbyUnoccupiedSquare (&mDestX, &mDestY);
        } 
    }

    if (mHidden) { /* short circuit the rest of this for now */
        return HALFTURN;
    }

    if (val_money) flags |= kFreeMoney;
    if (val_weapon) flags |= kFreeWeapon;
    if (val_armor) flags |= kFreeArmor;
    if (val_energy) flags |= kFreeEnergy;
    if (val_hidingspot && canHideUnderObjects ()) flags |= kHidingSpot;

    char buffers[3][3][8] = { { "       ", "       ", "       " }, 
                              { "       ", "   X   ", "       " }, 
                              { "       ", "       ", "       " } };
                              
    n = findSquares (flags, coord, info);
    for (i = 0; i < n; i++) {
        char ownt = 0, herot = 0;
        score = 100;
        if (info[i] & kLinedUp) score += val_linedup;
        if (info[i] & kFreeMoney) score += val_money;
        if (info[i] & kFreeWeapon) score += val_weapon;
        if (info[i] & kFreeArmor) score += val_armor;
        if (info[i] & kFreeEnergy) score += val_energy;
        if (info[i] & kHidingSpot) score += val_hidingspot;
        if (!(info[i] & kFreeItem)) {
            int ti;
            for (ti = 0 ; ti < TRACKLEN; ti++) {
                if (mTrack[ti].mX == coord[i].mX && 
                    mTrack[ti].mY == coord[i].mY) {
                    score += val_owntrack;
                    ownt++;
                }
            }
            for (ti = 0 ; ti < TRACKLEN; ti++) {
                if (Hero.mTrack[ti].mX == coord[i].mX && 
                    Hero.mTrack[ti].mY == coord[i].mY) {
                    score += val_htrack;
                    herot++;
                }
            }
            if (coord[i].mX == mX && coord[i].mY == mY) {
                if (info[i] & kHidingSpot) {
                    /* this prevents endless pacing between adjacent
                       hiding spots */
                    score += 5;
                } else {
                    score += val_same;
                }
            }
        }
        if (areAdjacent (coord[i].mX, coord[i].mY, mDestX, mDestY))
            score += val_adjacent;
        dist = rlDistance (coord[i].mX, coord[i].mY, mDestX, mDestY);
        if (dist < 25) {
            score += val_near * dist;
        } else if (dist < 50) {
            score += val_near * 25 + val_medium * (dist - 25);
        } else {
            score += val_near * 25 + val_medium * 25 + val_far * (dist - 50);
        }

        if (val_crowd) {
            score += val_crowd *
                mLevel->countAdjacentCreatures (coord[i].mX, coord[i].mY);
            if (coord[i].mX == mX && coord[i].mY == mY) 
                score += val_crowd; /* count as adjacent to self */
        }
/*
        I->debug (" %5d [%2d %2d] %s%s%s", score, coord[i].mX, coord[i].mY,
                  info[i] & kLinedUp ? "X" : "",
                  info[i] & kFreeMoney ? "$" : "",
                  info[i] & kFreeWeapon ? ")" : "");
*/
        
        {
            sprintf (buffers[coord[i].mY - mY + 1][(coord[i].mX - mX + 1)],
                     " % 4d%c", score, 
                     info[i] & kLinedUp ? 'l' : 
                     info[i] & kFreeMoney ? '$' :
                     info[i] & kFreeWeapon ? ')' :
                     info[i] & kDoor ? '+' : 
                     ownt ? '_' : 
                     herot ? 't' : ' ');
        }


        if (score > bestscore || 
            (score == bestscore && RNG (2))) 
        {
            bestscore = score;
            best = i;
        }
    }

    I->debug ("  dest: %d %d   Near/Medium/Far %d %d %d", mDestX, mDestY,
              val_near, val_medium, val_far);
    for (i = 0; i < 3; i++) 
        I->debug ("  %7s%7s%7s", buffers[i][0], buffers[i][1], buffers[i][2]);


    if (-1 == best) {
        /* nothing to do but stay where we are for now */
        return HALFTURN;
    }
    if (coord[best].mX == mX && coord[best].mY == mY) {
        /* apparently, we like where we are, try to do some things here: */
        shFeature *f = mLevel->getFeature (mX, mY);
        shObjectVector *v = mLevel->getObjects (mX, mY);
        shObject *obj;

        if (!mHidden && mLevel->isWatery (mX, mY) && canHideUnderWater () && !canSee (&Hero)) {
            mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
            return FULLTURN;
        }

        if (!mHidden && f && canHideUnder (f) && !canSee (&Hero)) {
            mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
            return FULLTURN;
        }

        if (!mHidden && v && canHideUnderObjects () && !canSee (&Hero)) {
            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);
                if (canHideUnder (obj)) {
                    mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
                    return FULLTURN;
                }
            }
        }

        /* try to pick up some junk off the floor */
        if (!mHidden && v && numHands ()) {
            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);

                if (!mHidden && canHideUnder (obj)) {
                    mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
                    return FULLTURN;
                }

                if ((info[best] & kFreeMoney && obj->isA (kMoney)) ||
                    (info[best] & kFreeWeapon && obj->isA (kWeapon)) ||
                    (info[best] & kFreeWeapon && obj->isA (kRayGun)) ||
                    (info[best] & kFreeArmor && obj->isA (kArmor)) ||
                    (info[best] & kFreeEnergy && obj->isA (kEnergyCell)))
                {
                    if (Hero.canSee (this)) {
                        I->p ("%s picks up %s.", the (), AN (obj));
                    }
                    addObjectToInventory (obj);
                    v->remove (obj);
                    if (0 == v->count ()) {
                        delete v;
                        mLevel->setObjects (mX, mY, NULL);
                    }
                    return FULLTURN;
                }
            } 
        }
        return QUICKTURN;
    } else {
        return doQuickMoveTo (vectorDirection (mX, mY, 
                                               coord[best].mX, coord[best].mY));
    }
}


int
shMonster::doHatch ()
{
    I->debug ("hatch strategy");
    int chance = 100000;

    if (mAlienEgg.mHatchChance) {
        /* we've been disturbed by something */
        --mAlienEgg.mHatchChance;
        chance = 2;
    } else if (distance (this, &Hero) <= 15 && 
               !Hero.getStoryFlag ("impregnation"))
    {
        chance = canSee (&Hero) ? 6 : 22;
    }
    if (!RNG (chance)) {
        shMonster *facehugger = new shMonster (findAMonsterIlk ("facehugger"));
        int x = mX;
        int y = mY;
        assert (facehugger);
        if (Hero.canSee (this)) {
            I->p ("The alien egg hatches!");
        }
        mLevel->removeCreature (this);
        mState = kDead;
        mLevel->putCreature (facehugger, x, y);
        return -2;
    }
    return FULLTURN;
}


/* sitstill strategy: do nothing until Hero is adjacent, then attack
   returns ms elapsed, -2 if the monster dies
*/
int
shMonster::doSitStill ()
{
    int elapsed;

    if (!isHostile ()) {
        return FULLTURN;
    }
    if (getMutantLevel ()) {
        int res = useMutantPower ();
        if (-1 == res) return -2;
        if (res) return res;
    }

/* at first, I had sessile creatures attack any monster that was adjacent,
   but that just resulted in a lot of annoying "you hear combat" messages
   and fewer monsters for the hero to fight
*/
    if (Hero.isAdjacent (mX, mY)) {
        if (-2 == doAttack (&Hero, &elapsed)) {
            return -2;
        } else {
            return elapsed;
        }
    } else {
        return RNG (HALFTURN, LONGTURN);
    }
}


/* returns -2 if died
  
*/

int
shMonster::mimicSomething ()
{
    if (canMimicMoney ()) {
        mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
        mMimic = kObject;
        mMimickedObject = &MoneyIlk;
    } else if (canMimicObjects ()) {
        shObjectIlk *ilk = NULL;
        int tries = 10;
        while (!ilk && tries--) {
            shObject *obj = generateObject (50);
            if (obj->mIlk->mSize == getSize ()) {
                ilk = obj->mIlk;
            }
            delete obj;
        }

        if (!ilk) {
            return 0;
        }

        mHidden = getSkillModifier (::kHide) + RNG (1, 20) + 10;
        mMimic = kObject;
        mMimickedObject = ilk;
    }
    return 0;
}


/* hide strategy: do nothing until Hero is adjacent, then surprise attack
   returns: elapsed time
            -2 if monster dies
 */
int
shMonster::doHide ()
{
    int elapsed;
    if (Hero.isAdjacent (mX, mY)) {
        mHidden = 0;
        if (-2 == doAttack (&Hero, &elapsed)) {
            return -2;
        } else {
            return elapsed;
        }
    }
    if (mHidden) {
        /* stay hidden */
        return RNG (500, 1500);
    } else if (Hero.canSee (this)) {
        /* Hero sees us, no point in hiding now... */
        return doWander ();
    } else {
        mHidden = getSkillModifier (::kHide) + RNG (1, 20);
        if (isA ("creeping credits")) {
            mHidden += 10; /* too lazy to add skill modifier */
            mMimic = kObject;
            mMimickedObject = &MoneyIlk;
        }
        return RNG (2000);
    }
}


/* lurk strategy: do nothing until Hero stumbles on us
   returns ms elapsed, -2 if the monster dies
*/
int
shMonster::doLurk ()
{    
    I->debug ("  lurk strategy");
    int elapsed;

    if (canSee (&Hero) || (canSmell (&Hero) && !RNG(22))) {
        mStrategy = kWander;
        mTactic = kReady;
        elapsed = readyWeapon ();
        if (-1 == elapsed) {
            return doWander ();
        } else {
            return elapsed;
        }
    }
    return RNG (300, 1000); /* keep on lurking */
}



#define GD_UNCHALLENGED 0
#define GD_CHALLENGED   1
#define GD_JANITOR      2


int
shMonster::doGuard ()
{
    if (isHostile ()) {
        mStrategy = kWander;
        mTactic = kNewEnemy;
        return doWander ();
    }

    if (canSee (&Hero)) {
        const char *the_guard = the ();
        readyWeapon ();
        if (mGuard.mToll > 0) {
            if (Hero.looksLikeJanitor ()) {
                switch (mGuard.mChallengeIssued) {
                case 2:
                    goto done;
                case 1:
                    mGuard.mChallengeIssued = 2;
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"Oh, it's you.  Get in there and clean up"
                              "in aisle 6!\"");
                    } else {
                        I->p ("%s emits some gruff beeps.", the_guard);
                    }
                    goto done;
                case 0:
                default:
                    mGuard.mChallengeIssued = 2;
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"You may pass, janitor.\"");
                    } else {
                        I->p ("%s emits some gruff beeps.", the_guard);
                    }
                    goto done;
                }                    
            }

            if (Hero.mX >= mGuard.mSX && Hero.mX <= mGuard.mEX &&
                Hero.mY >= mGuard.mSY && Hero.mY <= mGuard.mEY)
            {
                switch (mGuard.mChallengeIssued) {
                case 0: /* The Hero somehow slipped past (e.g. transportation)
                           without the guard issuing a challenge. */
                    mGuard.mToll *= 2;
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"Halt!  You must pay a fine of %d buckazoids "
                              "for trespassing!\"", mGuard.mToll);
                    } else {
                        I->p ("%s beeps an ultimatum concerning %d "
                              "buckazoids.", the_guard, mGuard.mToll);
                    }
                    break;
                case 2: /* The Hero was once dressed as a janitor but now 
                           appears in street clothes. */
                    mGuard.mToll /= 2;
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"Halt!  You must pay a fine of %d buckazoids "
                              " for removing your uniform while on duty!\"", 
                              mGuard.mToll);
                    } else {
                        I->p ("%s beeps an ultimatum concerning %d "
                              "buckazoids.", the_guard, mGuard.mToll);
                    }
                    break;
                case 1: default: /* The Hero ignored the challenge,
                                    and must be vaporized. */
                    goto getangry;
                }

                if (I->yn ("Will you pay?")) {
                    if (Hero.countMoney () < mGuard.mToll) {
                        I->p ("You don't have enough money.");
                    } else {
                        Hero.loseMoney (mGuard.mToll);
                        gainMoney (mGuard.mToll);
                        mGuard.mToll = 0;
                        if (Hero.tryToTranslate (this)) {
                            I->p ("\"You may pass.\"");
                        } else {
                            I->p ("%s beeps calmly.", the_guard);
                        }
                        return FULLTURN;
                    }
                }
            getangry:
                makeAngry ();
                mStrategy = kWander;
                return doWander ();
            }

            switch (mGuard.mChallengeIssued) {
            case 1:
                goto done;
            case 2:
                if (Hero.tryToTranslate (this)) {
                    I->p ("\"If you're not on duty, you must pay %d "
                          "buckazoids to pass this way.\"", mGuard.mToll);
                } else {
                    I->p ("%s emits some intimidating beeps about %d "
                          "buckazoids.", the_guard, mGuard.mToll);
                }
                break;
            case 0:
            default:
                if (Hero.tryToTranslate (this)) {
                    I->p ("\"Halt!  You must pay a toll of %d buckazoids "
                          "to pass this way.\"", mGuard.mToll);
                } else {
                    I->p ("%s emits some intimidating beeps about %d "
                          "buckazoids.", the_guard, mGuard.mToll);
                }
            }
            mGuard.mChallengeIssued = 1;
            
            if (I->yn ("Will you pay?")) {
                if (Hero.countMoney () < mGuard.mToll) {
                    I->p ("You don't have enough money.");
                } else {
                    Hero.loseMoney (mGuard.mToll);
                    gainMoney (mGuard.mToll);
                    mGuard.mToll = 0;
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"You may pass.\"");
                    } else {
                        I->p ("%s beeps calmly.", the_guard);
                    }
                }
            }
        }
    } else {
        if (mGuard.mToll > 0 && 1 == mGuard.mChallengeIssued) 
            mGuard.mChallengeIssued = 0;
    }
done:
    return FULLTURN;
    
}


void
shMonster::takeTurn ()
{
    const char *t_monster;
    int elapsed;
    int couldsee = Hero.canSee (this);

    /* decide what to do on the monster's turn */

    t_monster =  the ();

    if (mLevel != Hero.mLevel) {
        /* we'll be reawakened when the Hero returns to our level. */
        mState = kWaiting;
        return;
    }
    I->debug ("* %p %s @ %d, %d has %d AP:", this, getDescription (),
              mX, mY, mAP);

    if (hasAutoRegeneration ()) {
        if (MAXTIME == mLastRegen) 
            mLastRegen = Clock;
        while (Clock - mLastRegen > 1000) {
            if (mHP < mMaxHP) ++mHP;
            mLastRegen += 1000;
        }
    }

    if (checkTimeOuts ()) {
        return;
    }

    if (isAsleep ()) {
        elapsed = FULLTURN;
    } else if (isParalyzed ()) {
        elapsed = NDX (2, 6) * 100;
    } else if (isTrapped ()) {
        if (shMonster::kSitStill != mStrategy &&
            shMonster::kHatch != mStrategy) 
        {
            --mTrapped;
        }
        /* FIXME: should still be able to attack adjacent hero sometimes */
        if (Hero.canSee (this)) {
            shFeature *f = mLevel->getFeature (mX, mY);
            if (f)
                f->mTrapUnknown = 0;
            if (!mTrapped) {
                if (f) {
                    switch (f->mType) {
                    case shFeature::kPit:
                        I->p ("%s climbs out of the pit.", t_monster); break;
                    case shFeature::kAcidPit:
                        I->p ("%s climbs out of the acid filled pit.",
                              t_monster); 
                        break;
                    case shFeature::kSewagePit:
                        I->p ("%s swims to shallow sewage.", t_monster); break;
                    case shFeature::kWeb:
                        I->p ("%s frees %s from the web.", 
                              t_monster, herself ());
                        break;
                    default:
                        I->p ("%s frees %s from %s.",  
                              t_monster, herself (), THE (f));
                    } 
                }
            }
        }
        elapsed = FULLTURN;
    } else if (isStunned () || 
               (isConfused () && RNG (2))) 
    {
        if (shMonster::kSitStill == mStrategy || 
            shMonster::kHatch == mStrategy) 
        {
            /* do nothing */
            elapsed = FULLTURN;
        } else {
            /* move at random */
            shDirection dir = (shDirection) RNG (8);
            elapsed = doMove (dir);
        }
    } else if (-1 != mEnemyX) {
        elapsed = doRetaliate ();
    } else {
        switch (mStrategy) {
        case shMonster::kPet:
            elapsed = doPet (); break;
        case shMonster::kHide:
            elapsed = doHide (); break;
        case shMonster::kLurk:
            elapsed = doLurk (); break;
        case shMonster::kSitStill:
            elapsed = doSitStill (); break;
        case shMonster::kHatch:
            elapsed = doHatch (); break;
        case shMonster::kShopKeep:
            elapsed = doShopKeep (); break;
        case shMonster::kAngryShopKeep:
            elapsed = doAngryShopKeep (); break;
        case shMonster::kGuard:
            elapsed = doGuard (); break;
        case shMonster::kDoctor:
            elapsed = doDoctor (); break;
        case shMonster::kWander:
        default:
            elapsed = doWander (); break;
        }
    }
    //I->debug ("%d elapsed", elapsed);
    if (-2 == elapsed) {

    } else {
        if (!couldsee && !isPet () && Hero.canSee (this)) {
            Hero.interrupt ();
        }
        if (isStunned ()) {
            elapsed = HALFTURN;
        }
        mAP -= elapsed;
    }
}


int
shMonster::likesMoney ()
{
    return (isA ("creeping credits")) ;
}


int
shMonster::likesWeapons ()
{
    return (mIlk->mNumHands > 0) ? 1 : 0;
}


int
shMonster::needsWeapon ()
{
    if (mWeapon && !hasAmmo (mWeapon)) {
        return 1;
    }
    return (mIlk->mNumHands > 0 && NULL == mWeapon) ? 1 : 0;
}


static const char* weaponpreferences[] = {
    "disintegration ray gun",
    "railgun",
    "freeze ray gun",
    "heat ray gun",
    "poison ray gun",
    "gatling cannon",
    "laser cannon",
    "gamma ray gun",
//    "gauss ray gun",
    "boltgun",
    "pulse rifle",
    "sniper rifle",
    "concussion grenade",
    "frag grenade",
    "stun grenade",
    "shotgun",
    "laser rifle",
    "assault pistol",
    "blaster",
    "phaser",
    "pistol",
    "laser pistol",
    "pea shooter",
    "light saber",
    "katana",
    "chainsaw",
    "pair of nunchucks",
    "cattle prod",
    "mop",
    "anal probe",
    "club",
    "knife",
    NULL
};


/* selects and ready a weapon
   returns: ms elapsed, -1 if nothing done
*/
int
shMonster::readyWeapon ()
{
    int i;
    shObject *best = NULL;
    int bestrank = 1000;
    int j;

/*
    if (!needsWeapon ()) {
        return -1;
    }
*/
    if (mIlk->mNumHands < 1) {
        return -1;
    }

    for (i = 0; i < mInventory->count (); i++) {
        shObject *obj = mInventory->get (i);
        if (!(obj->isA (kWeapon) || obj->isA (kRayGun)) || 
            !hasAmmo (obj)) 
        {
            continue;
        }

        if (obj->isA ("stasis ray gun" ) && !Hero.isParalyzed () && !Hero.isAsleep ()) {
            best = obj;
            break;
        }
        for (j = 0; weaponpreferences[j] && j < bestrank; j++) {
            if (obj->isA (weaponpreferences[j])) {
                bestrank = j;
                best = obj;
            }
        }
        if (!best && 
            !obj->isA (kRayGun))  /* don't accidentaly use helpful ray guns! */
        {
            /* somehow we fell through here because a weapon isn't in the
               the preferences list
             */
            best = obj; 
        }
    }

    if (best) {
        if (best == mWeapon) {
            return -1;
        }
        wield (best);
        return FULLTURN;
    } else {
        wield (NULL);
        return -1;
    }
}


void
shMapLevel::makeNoise (int x, int y, int radius)
{
    shCreature *c;
    int i;
    int d;
    
    for (i = 0; i < mCrList.count (); i++) {
        c = mCrList.get (i);
        d = distance (x, y, c->mX, c->mY);
        if (d < radius ||
            (d < 2 * radius && RNG(2)))
        {
            /* do something to wakeup the mosnter*/    
        }
    }
}


void
shMapLevel::alertMonsters (int x, int y, int radius, 
                           int destx, int desty)
{
    shCreature *c;
    int i,d;
    
    for (i = 0; i < mCrList.count (); i++) {
        c = mCrList.get (i);
        d = distance (x, y, c->mX, c->mY);
        if (d < radius ||
            (d < 2 * radius && RNG(2)))
        {
            if (!c->isHero () && c->isHostile () && !c->isPet () && 
                !c->isFleeing () && !c->isAsleep ()) 
            {
                shMonster *m = (shMonster *) c;
                if (m->getInt () < 5)
                    continue;

                if (!m->canSee (&Hero)) {
                    if (shMonster::kLurk == m->mStrategy)
                        m->mStrategy = shMonster::kWander;
                    m->mDestX = destx + RNG (9) - 4;
                    m->mDestY = desty + RNG (7) - 3;
                }
            }
        }
    }
}
