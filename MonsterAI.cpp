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
        char buf[60];
        the (buf, 60);
        I->p ("%s gets angry!", buf);
        mDisposition = kHostile;
        mTactic = kNewEnemy;
    }
}



/* try to move the creature one square in the given direction
   RETURNS: elapsed time, or 
            -2 if the creature dies
*/

int
shCreature::doMove (shDirection dir)
{
    const int buflen = 40;
    char buf[buflen];
    char buf2[buflen];
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

    if (mLevel->isObstacle (x, y)) {
        if (Hero.canSee (this)) {
            if (Hero.canSee (x, y)) {
                I->p ("You see %s bump into %s", the (buf, buflen),
                      mLevel->getSquare (x, y) -> the ());
            } else {
                I->p ("You see %s bump into something.", 
                      the (buf, buflen));
            }
        }
        /* you can't tell when an unseen monster bumps into an obstacle even if
           you can see the obstacle, so don't print a message */
        return speed / 2;
    }
    else if (mLevel->isOccupied (x, y)) {
        if (&Hero == mLevel->getCreature (x,y)) {
            I->p ("%s bumps into you!", the (buf, buflen));
        } else if (Hero.canSee (this)) {
            if (Hero.canSee (x, y)) {
                I->p ("You see %s bump into %s", the (buf, buflen), 
                      mLevel->getCreature (x, y) -> the (buf2, buflen));
            } else {
                I->p ("You see %s bump into something.", the (buf, buflen));
            }
        } else if (Hero.canSee (x, y)) {
            I->p ("You see something bump into %s", the (buf, buflen));
        }
        return speed / 2;
    }
    else {
        int i;
        for (i = TRACKLEN - 1; i > 0; --i) {
            mTrack[i] = mTrack[i-1];
        }
        mTrack[0].mX = mX;
        mTrack[0].mY = mY;

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
    }

    I->debug ("quickmoveto %d, %d", mDestX, mDestY);
    
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

    *info = 0;
    for (y = mY - 1; y <= mY + 1; y++) {
        for (x = mX - 1; x <= mX + 1; x++) {
            if (mLevel->isInBounds (x, y)) {
                shFeature *f = mLevel->getFeature (x, y);
                if (f && f->isDoor () && 
                         !(f->isOpenDoor () || f->isAutomaticDoor ())) 
                {
                    if (flags & kDoor) {
                        *info |= kDoor;
                    } else {
                        continue;
                    }
                }

                if (f && f->isTrap () && 
                    (isPet () ? !f->mTrapUnknown : !f->mTrapMonUnknown)) 
                {
                    if (isFlying () && (shFeature::kPit == f->mType ||
                                        shFeature::kAcidPit == f->mType ||
                                        shFeature::kTrapDoor == f->mType ||
                                        shFeature::kHole == f->mType))
                    {
                        /* this is an acceptable square, keep going */
                    } else if (flags & kTrap) {
                        *info |= kTrap;
                    } else {
                        continue;
                    }
                }

                if (!mLevel->appearsToBeFloor (x, y)) {
                    if (flags & kWall) {
                        *info |= kWall;
                    } else {
                        continue;
                    }
                }

                shCreature *c = mLevel->getCreature (x, y);
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

                if (x == Hero.mX || y == Hero.mY ||
                    x - y == Hero.mX - Hero.mY ||
                    x + y == Hero.mX + Hero.mY && 
                    mLevel->isInLOS (x, y))
                {
                    if (flags & kLinedUp) {
                        *info |= kLinedUp;
                    } else {
                        continue;
                    }
                }

                if (flags & kFreeItem) {
                    shObjectVector *v = mLevel->getObjects (x, y);
                    shObject *obj;
                    int i;

                    if (v) {
                    for (i = 0; i < v->count (); i++) {
                        obj = v->get (i);
                        if (flags & kFreeMoney && obj->isA (kMoney)) {
                            *info |= kFreeMoney;
                        } else if (flags & kFreeWeapon && 
                                   (obj->isA (kWeapon) || obj->isA (kRayGun))) 
                        {
                            *info |= kFreeWeapon;
                        } else if (flags &kFreeArmor && obj->isA (kArmor)) {
                            *info |= kFreeArmor;
                        }                           
                    }
                    }
                }
                
                coord->mX = x;
                coord->mY = y;
                ++coord;
                ++info;
                ++n;
                *info = 0;
            }
        }
    }
    return n;
}


void
shMonster::doRangedAttack (shAttack *atk, shDirection dir)
{
    char buf[80];
    the (buf, 80);
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
        }
        Level->areaEffect (atk, mX, mY, dir, this);
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
    const int thebufsize = 40;
    char t_monster[thebufsize];
    char t_weapon[thebufsize];
    
    the (t_monster, thebufsize);
    
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
    
    if (NULL == mWeapon) {
        /* randomly pick a physical attack: */
        atk = mIlk->mAttacks.get (RNG (mIlk->mAttacks.count ()));
    } else if (mWeapon->isMeleeWeapon ()) {
        atk = & ((shWeaponIlk *) mWeapon->mIlk) -> mAttack;
    } else {
        if (target->isHero () && Hero.canSee (this)) {
            mWeapon->her (t_weapon, thebufsize, this);
            I->p ("%s swings %s at you!", t_monster, t_weapon);
        }
        atk = &ImprovisedObjectAttack;
    }
    
    if (NULL == atk) {
        *elapsed = HALFTURN;
        return -3;
    } else {
        *elapsed = atk->mAttackTime;
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

    int val_linedup;
    int val_adjacent;
    int val_near;
    int val_medium;
    int val_far;
    int val_track;
    int val_same;
    int val_money;
    int val_weapon;
    int val_armor;
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
        val_track = -2;
        val_same = -5;
        val_money = 0;
        val_weapon = 0;
        val_armor = 0;
    } else if (canSee (&Hero) || canSmell (&Hero)) {
        mDestX = Hero.mX;
        mDestY = Hero.mY;

        if (getMutantLevel () && RNG (2)) {
            res = useMutantPower ();
            if (res) return res;
        }
        res = readyWeapon ();
        if (-1 != res) {
            return res;
        }
        val_money = 0;
        val_weapon = getInt () > 7 ? 25 : 0;
        val_armor = 0;

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
                }
                return atk->mAttackTime;
            }
            hasrangedweapon = 1;
        }

        if (mWeapon && !mWeapon->isMeleeWeapon ()) {
            if (!hasAmmo (mWeapon)) {
                return readyWeapon ();
            }

            int maxrange;
            if (mWeapon->isThrownWeapon ()) maxrange = 60;
            else if (mWeapon->isA (kRayGun)) maxrange = 60;
            else maxrange = 120;

            shDirection dir = linedUpDirection (this, &Hero);
            if (kNoDirection != dir && 
                RNG (10) && 
                canSee (&Hero) &&
                distance (this, &Hero) < maxrange) 
            {
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
            flags = kLinedUp;
            val_linedup = 15;    /* try to line up a shot */
            val_adjacent = -10;
            
            if (health < 1) { /* hurt, retreat */
                val_near = 2;
                val_medium = 1;
                val_far = 0;
                val_linedup = -5;
            } else { /* close in for a shot */
                val_near = 0;
                val_medium = -1;
                val_far = 1;
            } 

            val_track = -3;
            val_same = -2;
        } else {
            if (areAdjacent (this, &Hero)) {
                if (-2 == doAttack (&Hero, &elapsed)) {
                    return -2;
                } else {
                    return elapsed;
                }
            }
                
            if (isMultiplier () && !RNG (8) &&
                mLevel->countAdjacentCreatures (mX, mY) < 5 &&
                mLevel->mCrList.count () < 150) 
            {
                int x = mX;
                int y = mY;
                if (0 == mLevel->findAdjacentUnoccupiedSquare (&x, &y) &&
                    mLevel->countAdjacentCreatures (x, y) < 4) 
                {
                    shMonster *m = new shMonster (mIlk);
                    if (0 == mLevel->putCreature (m, x, y)) {

                    } else {
                        /* delete m; */
                    }
                    return FULLTURN;
                }
            }

            flags = kLinedUp;
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
            val_adjacent = 30;
            val_near = -1;
            val_medium = -1;
            val_far = -1;
            val_track = -10;
            val_same = -10;
        } 
    } else {
        flags = kDoor | kLinedUp;
        val_linedup = 0;
        val_adjacent = 0;
        val_near = -1;
        val_medium = -1;
        val_far = -1;
        val_track = -10;
        val_same = -20;
        val_money = 0;
        val_weapon = numHands () ? 5 : 0;
        val_armor = 0;
        if (!RNG (10)) { /* wander somewhere new */
            mDestX = mX + RNG (13) - 6;
            mDestY = mY + RNG (9) - 4;
            mLevel->findNearbyUnoccupiedSquare (&mDestX, &mDestY);
        } 
    }

    if (val_money) flags |= kFreeMoney;
    if (val_weapon) flags |= kFreeWeapon;
    if (val_armor) flags |= kFreeArmor;

    n = findSquares (flags, coord, info);
    for (i = 0; i < n; i++) {
        score = 0;
        if (info[i] & kLinedUp) score += val_linedup;
        if (info[i] & kFreeMoney) score += val_money;
        if (info[i] & kFreeWeapon) score += val_weapon;
        if (info[i] & kFreeArmor) score += val_armor;
        if (!(info[i] & kFreeItem)) {
            int ti;
            for (ti = 0 ; ti < TRACKLEN; ti++) {
                if (mTrack[ti].mX == coord[i].mX && 
                    mTrack[ti].mY == coord[i].mY) {
                    score += val_track;
                }
            }
            if (coord[i].mX == mX && coord[i].mY == mY) {
                score += val_same; 
            }
        }
        if (areAdjacent (coord[i].mX, coord[i].mY, mDestX, mDestY))
            score += val_adjacent;
        dist = rlDistance (coord[i].mX, coord[i].mY, mDestX, mDestY);
        if (dist < 25) {
            score += val_near * dist;
        } else if (dist < 50) {
            score += val_medium * dist;
        } else {
            score += val_far * dist;
        }
/*
        I->debug (" %5d [%2d %2d] %s%s%s", score, coord[i].mX, coord[i].mY,
                  info[i] & kLinedUp ? "X" : "",
                  info[i] & kFreeMoney ? "$" : "",
                  info[i] & kFreeWeapon ? ")" : "");
*/
        if (score > bestscore || 
            (score == bestscore && RNG (2))) 
        {
            bestscore = score;
            best = i;
        }
    }

    if (-1 == best) {
        /* nothing to do but stay where we are for now */
        return 500;
    }
    if (coord[best].mX == mX && coord[best].mY == mY) {
        /* try to pick up some junk off the floor */
        shObjectVector *v;
        shObject *obj;

        if (numHands () && 
            (v = mLevel->getObjects (mX, mY)))
        {
            for (i = 0; i < v->count (); i++) {
                obj = v->get (i);

                if ((info[best] & kFreeMoney && obj->isA (kMoney)) ||
                    (info[best] & kFreeWeapon && obj->isA (kWeapon)) ||
                    (info[best] & kFreeWeapon && obj->isA (kRayGun)) ||
                    (info[best] & kFreeArmor && obj->isA (kArmor)))
                {
                    if (Hero.canSee (this)) {
                        char buf1[40];
                        char buf2[40];
                        
                        the (buf1, 40);
                        obj->an (buf2, 40);
                        I->p ("%s picks up %s.", buf1, buf2);
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
        chance = 5;
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


/* sessile strategy: do nothing until Hero is adjacent, then attack
   returns ms elapsed, -2 if the monster dies
*/
int
shMonster::doSessile ()
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
    if (Hero.isAdjacent (mX, mY)) {
        if (-2 == doAttack (&Hero, &elapsed)) {
            return -2;
        } else {
            return elapsed;
        }
    } else {
        return RNG (HALFTURN, LONGTURN);
    }

/* at first, I had sessile creatures attack any monster that was adjacent,
   but that just resulted in a lot of annoying "you hear combat" messages
   and fewer monsters for the hero to fight
*/
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

    if (canSee (&Hero) || canSmell (&Hero)) {
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





int
shMonster::doGuard ()
{
    char buf[50];

    
    if (isHostile ()) {
        mStrategy = kWander;
        mTactic = kNewEnemy;
        return doWander ();
    }

    if (canSee (&Hero)) {
        readyWeapon ();
        the (buf, 50);
        if (mGuard.mToll > 0) {
            if (Hero.mX >= mGuard.mSX && Hero.mX <= mGuard.mEX &&
                    Hero.mY >= mGuard.mSY && Hero.mY <= mGuard.mEY) 
            {
                makeAngry ();
                if (Hero.tryToTranslate (this)) {
                    I->p ("\"Intruder alert!\"");
                } else {
                    I->p ("%s beeps menacingly.", buf);
                }
                mStrategy = kWander;
                return doWander ();
            }

            
            if (mGuard.mChallengeIssued) {
                goto done;
            }
            mGuard.mChallengeIssued = 1;
            
            if (Hero.tryToTranslate (this)) {
                I->p ("\"Halt!  You must pay a toll of %d buckazoids "
                      "to pass this way.\"", mGuard.mToll);
            } else {
                I->p ("%s emits some intimidating beeps about %d "
                      "buckazoids.", buf, mGuard.mToll);
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
                        I->p ("%s beeps calmly.", buf);
                    }
                }
            }
        }
    } else {
        mGuard.mChallengeIssued = 0;
    }
done:
    return FULLTURN;
    
}


void
shMonster::takeTurn ()
{
    const int thebufsize = 40;
    char t_monster[thebufsize];
    int elapsed;
    int couldsee = Hero.canSee (this);

    /* decide what to do on the monster's turn */

    the (t_monster, thebufsize);

    if (mLevel != Hero.mLevel) {
        /* we'll be reawakened when the Hero returns to our level. */
        mState = kWaiting;
        return;
    }
    I->debug ("* %p %s @ %d, %d has %d AP:", this, t_monster,
              mX, mY, mAP);

    if (checkTimeOuts ()) {
        return;
    }

    if (isAsleep ()) {
        elapsed = FULLTURN;
    } else if (isTrapped ()) {
        if (Hero.canSee (this)) {
            shFeature *f = mLevel->getFeature (mX, mY);
            if (f) f->mTrapUnknown = 0;
        }
        if (0 == --mTrapped && Hero.canSee (this)) {
            I->p ("%s frees %s from the trap", 
                  t_monster, herself ());
        }
        elapsed = FULLTURN;
    } else if (isStunned () || 
               (isConfused () && RNG (2))) 
    {
        /* move at random */
        shDirection dir = (shDirection) RNG (8);
        elapsed = doMove (dir);
    } else if (isParalyzed ()) {
        elapsed = NDX (2, 6) * 100;
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
        case shMonster::kSessile:
            elapsed = doSessile (); break;
        case shMonster::kHatch:
            elapsed = doHatch (); break;
        case shMonster::kPassive:
            elapsed = FULLTURN; break;
        case shMonster::kShopKeep:
            elapsed = doShopKeep (); break;
        case shMonster::kAngryShopKeep:
            elapsed = doAngryShopKeep (); break;
        case shMonster::kGuard:
            elapsed = doGuard (); break;
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


static char* weaponpreferences[] = {
    "disintegration ray gun",
    "railgun",
    "freeze ray gun",
    "heat ray gun",
    "poison ray gun",
    "gatling cannon",
    "laser cannon",
    "gamma ray gun",
    "gauss ray gun",
    "boltgun",
    "pulse rifle",
    "sniper rifle",
    "concussion grenade",
    "frag grenade",
    "stun grenade",
    "shotgun",
    "phaser",
    "blaster",
    "laser rifle",
    "pistol",
    "laser pistol",
    "pea shooter",
    "light saber",
    "katana",
    "tazer",
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

        if (obj->isA ("stasis ray gun" ) && !Hero.isParalyzed ()) {
            best = obj;
            break;
        }
        for (j = 0; weaponpreferences[j] && j < bestrank; j++) {
            if (obj->isA (weaponpreferences[j])) {
                bestrank = j;
                best = obj;
            }
        }
        if (!best) {
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
        return 100;
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
