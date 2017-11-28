#include <math.h>
#include <stdio.h>
#include "Global.h"
#include "Util.h"
#include "Monster.h"
#include "Creature.h"
#include "Object.h"
#include "Interface.h"
#include "Hero.h"

shMonsterIlk *Earthling;


#define MAXMONSTERLEVEL 25
static shVector <shMonsterIlk *> MonsterIlksGroupedByLevel[MAXMONSTERLEVEL];



void
initializeMonsters ()
{
    shMonsterIlk *cur;

#include "MonsterData.h"

};

//constructor:
shMonsterIlk::shMonsterIlk (const char *name,
                            shCreatureType ty,
                            shMonsterIlk *parent,
                            shThingSize size,
                            int hitdice,
                            int baselevel,
                            int str, int con, int agi, int dex,
                            int in, int wis, int cha,
                            int speed,
                            int gender,
                            int numhands,
                            int ac, /*natural armor bonus */
                            int numappearingdice,
                            int numappearingdiesides,
                            shMonster::Strategy strategy,
                            int peacefulchance,
                            int probability,
                            char sym,
                            shColor color)
    : mAttacks () , mRangedAttacks ()
{
    int i;

    mId = MonsterIlks.add (this);
    mName = name;
    mType = ty;
    mParent = parent;
    mSize = size;
    mHitDice = hitdice;
    mBaseLevel = baselevel;
    mMutantLevel = 0;
    mStr = str;
    mCon = con;
    mAgi = agi;
    mDex = dex;
    mInt = in;
    mWis = wis;
    mCha = cha;
    mSpeed = speed;
    mGender = gender;
    mNumHands = numhands;
    mNaturalArmorBonus = ac;
    mInateIntrinsics = 0;
    for (i = 0; i < kMaxEnergyType; i++) {
        mInateResistances[i] = 0;
    }
    mFeats = 0;
    mNumAppearingDice = numappearingdice;
    mNumAppearingDieSides = numappearingdiesides;
    mDefaultStrategy = strategy;
    mDefaultDisposition = shMonster::kHostile;
    mPeacefulChance = peacefulchance;
    mProbability = probability;
    initGlyph (&mGlyph, sym, color, 0);

    for (i = 0; i < kMaxMutantPower; i++) {
        mMutantPowers[i] = 0;
    }

    MonsterIlksGroupedByLevel[mBaseLevel].add (this);
}         


void
shMonsterIlk::addMutantPower (shMutantPower power)
{
    if (kTelepathyPower == power) {
        mInateIntrinsics |= kTelepathy;
    } else {
        mMutantPowers[power] = 1;
        ++mMutantLevel;
    }
}

void
shMonsterIlk::addAttack (shAttack::Type type, int etype, int numdice, 
                         int dicesides, int attacktime, 
                         int prob /* = 1 */,
                         int energy2 /* = kNoEnergy */, 
                         int numdice2 /* = 0 */, 
                         int dicesides2 /* = 6 */ )
{
    int i;
    int flags = kMelee;
    shAttack *atk;

    if (mAttacks.count ()) {
        flags |= kSecondaryAttack;
    }
    if (shAttack::kTouch == type) {
        flags |= kTouchAttack;
    }           

    atk = new shAttack (NULL, type, shAttack::kSingle, flags,
                        (shEnergyType) etype, numdice, dicesides,
                        attacktime, prob, 0, 
                        (shEnergyType) energy2, numdice2);
    atk->mDamage[1].mDieSides = dicesides2;

    for (i = 0; i < prob; i++) {
        mAttacks.add (atk);
    }
}


void
shMonsterIlk::addRangedAttack (shAttack::Type type, 
                               shAttack::Effect effect,
                               int etype, int numdice, 
                               int dicesides, int attacktime,
                               int range, /* = 10 */
                               int prob /* = 1 */,
                               int energy2 /* = kNoEnergy */, 
                               int numdice2 /* = 0 */, 
                               int dicesides2 /* = 6 */ )
{
    int flags = kAimed;
    shAttack *atk;

    atk = new shAttack (NULL, type, effect, flags,
                        (shEnergyType) etype, numdice, dicesides,
                        attacktime, prob, range, 
                        (shEnergyType) energy2, numdice2);
    atk->mDamage[1].mDieSides = dicesides2;

    mRangedAttacks.add (atk);
}


void
shMonsterIlk::addEquipment (const char *description)
{
    mStartingEquipment.add (description);
}

void
shMonsterIlk::addIntrinsic (shIntrinsics intrinsic)
{
    mInateIntrinsics |= intrinsic;
}

void
shMonsterIlk::addResistance (shEnergyType energy, int amount)
{
    mInateResistances[energy] = amount;
}

void
shMonsterIlk::addFeat (shFeat feat)
{
    mFeats |= feat;
}


//constructor:
shMonster::shMonster (shMonsterIlk *ilk, int extralevels /* = 0 */ )
    :shCreature ()
{
    int diesize;
    int i;
    int do_or = 0;
    int do_and = 0;
    int gotweapon = 0;

    mType = ilk->mType;
    strncpy (mName, ilk->mName, 30);
    mCLevel = ilk->mBaseLevel;
    mNaturalArmorBonus = ilk->mNaturalArmorBonus;
    mReflexSaveBonus = 0;

    mDir = kNoDirection;
    mIlk = ilk;
    mTame = 0;
    mStrategy = ilk->mDefaultStrategy;
    mDisposition = ilk->mDefaultDisposition;
    mTactic = kReady;
    mDestX = -1;
    mEnemyX = -1;
    mPlannedMoveIndex = -1;
    mSpellTimer = 0;
    
    memcpy (mInateResistances, ilk->mInateResistances, 
            sizeof (mInateResistances));
    mInateIntrinsics = ilk->mInateIntrinsics;
    mFeats = ilk->mFeats;

#define IMMUNE 122

    switch (mType) {
    case kMutant:
        diesize = 8;
        mBAB = mCLevel;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kMagnetic] = IMMUNE;
        break;  
    case kHumanoid:
        diesize = 8; 
        mBAB = mCLevel / 2;
        mInateResistances[kMagnetic] = IMMUNE;
        break;
    case kAnimal:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateIntrinsics |= kScent;
        break;
    case kInsect:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateIntrinsics |= kScent;
        break;
    case kOutsider:
        diesize = 8;
        mBAB = mCLevel;
        mInateResistances[kMagnetic] = IMMUNE;
        break;
    case kBot:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kPoisonous] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kStunning] = IMMUNE;
        mInateResistances[kHealing] = IMMUNE;
        mInateResistances[kRestoring] = IMMUNE;
        mInateResistances[kChoking] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        break;
    case kDroid:
        diesize = 8;
        mBAB = mCLevel / 2;
        mInateResistances[kPoisonous] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kStunning] = IMMUNE;
        mInateResistances[kHealing] = IMMUNE;
        mInateResistances[kRestoring] = IMMUNE;
        mInateResistances[kChoking] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        break;
    case kProgram:
        diesize = 8;
        mBAB = mCLevel / 2;
        mInateResistances[kPoisonous] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kStunning] = IMMUNE;
        mInateResistances[kHealing] = IMMUNE;
        mInateResistances[kRestoring] = IMMUNE;
        mInateResistances[kChoking] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        break;
    case kConstruct:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kPoisonous] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kStunning] = IMMUNE;
        mInateResistances[kChoking] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        break;
    case kOoze:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kPoisonous] = 3;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kChoking] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        break;
    case kAberration:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateResistances[kRadiological] = IMMUNE;
        mInateResistances[kViolating] = IMMUNE;
        break;
    case kCyborg:
        diesize = 8;
        mBAB = mCLevel;
        mInateResistances[kPoisonous] = 2;
        mInateResistances[kRadiological] = 10;
        mInateIntrinsics |= kBreathless;
        break;
    case kEgg:
        mInateResistances[kViolating] = IMMUNE;
        mInateIntrinsics |= kBreathless;
        /* fall through */
    case kBeast:
        diesize = 8;
        mBAB = mCLevel;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateIntrinsics |= kScent;
        break;
    case kVermin:
        diesize = 8;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateIntrinsics |= kScent;
        mInateIntrinsics |= kCanSwim;
        break;
    case kAlien:
        diesize = 8;
        mBAB = mCLevel;
        mAlienEgg.mHatchChance = 0;
        mInateResistances[kMagnetic] = IMMUNE;
        mInateResistances[kRadiological] = 10;
        mInateResistances[kCorrosive] = IMMUNE;
        mInateIntrinsics |= kScent;
        gainRank (kUnarmedCombat);
        break;
    default:
        I->debug ("Alert! Unknown monster type"); 
        diesize = 6;
        mBAB = mCLevel * 3 / 4;
        mInateResistances[kMagnetic] = IMMUNE;
    }

    if (3 == mIlk->mGender) {
        mGender = RNG (1, 2);
    } else {
        mGender = mIlk->mGender;
    }

    /* roll ability scores */
    rollAbilityScores (mIlk->mStr, mIlk->mCon, mIlk->mAgi, mIlk->mDex,
                       mIlk->mInt, mIlk->mWis, mIlk->mCha);

    /* roll hit points */
    rollHitPoints (mIlk->mHitDice, diesize);

    /* setup speed */
    mSpeed = mIlk->mSpeed;
    computeAC ();

    mGlyph = mIlk->mGlyph;
    mState = kActing;
    
    for (i = 0; i < mIlk->mStartingEquipment.count (); i++) {
        const char *str = mIlk->mStartingEquipment.get (i);

        if ('|' == str[0]) { /* the or symbol indicates don't try to create 
                                the object unless the previous obj failed */
            if (0 == do_or) {
                do_or = 1;
                do_and = 0;
                continue;
            } else {
                ++str;
            }
        } else if (',' == str[0]) { /* the comma symbol indicates create the 
                                       object only if the prev obj was made */
            if (1 == do_and) {
                ++str;
            } else {
                continue;
            }
        }

        shObject *obj = createObject (str, 0);

        if (NULL == obj) {
            do_or = 1;
            do_and = 0;
            I->debug ("unable to equip %s", str);
            continue;
        }
        do_or = 0;
        do_and = 1;
        addObjectToInventory (obj);
        if (!mWeapon && obj->isA (kWeapon)) {
            gainRank (((shWeaponIlk *) obj->mIlk) -> mSkill, 1 + mBAB / 4);
            ++gotweapon;
            /* don't wield until hero is in sight so he can see message */
            //wield (obj, 1); 
        } else if (obj->isA (kArmor)) {
            don (obj, 1);
        }
    }

    if (!gotweapon) {
        gainRank (kUnarmedCombat, 1 + mBAB / 4);
    }

    /* maybe monster gets some more treasure */
    if (noTreasure ()) {
        /* no treasure */
    } else {
        if (RNG (50) <= 5 + mCLevel) {
            addObjectToInventory (createMoney (NDX (mCLevel + 1, 10)));
        }
        if (RNG (80) <= 5 + mCLevel) {
            shObject *obj = generateObject (-1);
            
            while (obj->getMass () > 5000) {
                delete obj;
                obj = generateObject (-1);
            }
            
            addObjectToInventory (obj);
        }
    }

    computeIntrinsics ();

    if (RNG (100) < mIlk->mPeacefulChance) {
        mDisposition = kIndifferent;
    }


    I->debug ("spawned %s with %d HP speed %d", mName, mHP, mSpeed);
}


shMonsterIlk *
pickAMonsterIlk (int level)
{
    int baselevel = level;
    int n = 0;
    shVector <shMonsterIlk *> V;
    shMonsterIlk *ilk;
    int i;
    int c;

//    rarity = RNG (3) ? 1 : RNG (3) ? 2 : 3;
    
    while (0 == (n = MonsterIlksGroupedByLevel[baselevel].count ())) {
        baselevel--;
    }


    for (i = 0; i < n; i++) {
        ilk = MonsterIlksGroupedByLevel[baselevel].get (i);
        c = ilk->mProbability;
        while (c--) {
            V.add (ilk);
        }
    }
    if (0 == V.count ()) {
        return NULL;
    }

    return V.get (RNG (V.count ()));
}


shMonsterIlk *
findAMonsterIlk (const char *name)
{
    int i;
    shMonsterIlk *ilk;

    for (i = 0; i < MonsterIlks.count (); i++) {
        ilk = MonsterIlks.get (i);
        if (0 == strcmp (name, ilk->mName)) {
            return ilk;
        }
    }
    return NULL;
}


shMonster *
generateMonster (int level)
{
    /* generates random monster of the given level */

    int baselevel = level;
    int n;

    while (0 == (n = MonsterIlksGroupedByLevel[baselevel].count ())) {
        baselevel--;
    }
    
    return new shMonster (MonsterIlksGroupedByLevel[baselevel].get (RNG (n)),
                          baselevel - level);
}


const char *
shMonster::the ()
{
    char *buf = GetBuf ();

    if (!Hero.canSee (this)) {
        strncpy (buf, hasMind () ? "someone" : "something", SHBUFLEN);
    } else {
        snprintf (buf, SHBUFLEN, "the %s", mIlk->mName);
    }
    return buf;
}

const char *
shMonster::an ()
{
    char *buf = GetBuf ();

    if (!Hero.canSee (this)) {
        strncpy (buf, hasMind () ? "someone" : "something", SHBUFLEN);
    } else if (isUnique ()) {
        return the ();
    } else {
        snprintf (buf, SHBUFLEN, 
                  isvowel (mIlk->mName[0]) ? "an %s" : "a %s", 
                  mIlk->mName);
    }
    return buf;
}

const char *
shMonster::your ()
{
    char *buf = GetBuf ();

    if (Hero.isBlind ()) {
        strncpy (buf, hasMind () ? "someone" : "something", SHBUFLEN);
    } else {
        snprintf (buf, SHBUFLEN, "your %s", mIlk->mName);
    }
    return buf;
}


const char *
shMonster::getDescription ()
{
    return mIlk->mName;
}


int
shMonster::numHands ()
{
    return mIlk->mNumHands;
}

int
shMonster::die (shCauseOfDeath how, const char *killer)
{
    return shCreature::die (how, killer);
}


/*     SPOILER GENERATION     */

FILE *spoilerfile;

int speedToAP (int speed); /* Game.cpp */

void
shMonsterIlk::spoilers ()
{
    int i, n = 100;
    int AC = 0;
    int speed = 0;
    int HP = 0;
    int ap = 0;
    
    int acfoo = 4 * mBaseLevel / 3  + 10;

    int attacks = 0;
    int hits = 0;
    int damagefoo = 0;
    int damage15 = 0;
    int damage20 = 0;
    int damage25 = 0;

    int rhits = 0;
    int rattacks = 0;
    int rdamagefoo = 0;
    int zhits = 0;

    int difficulty = 0;

    shMonster *m;

    for (i = 0; i < n; i++) {
        m = new shMonster (this);
        int clk = 0;
        
        AC += m->getAC ();
        speed += m->mSpeed;
        HP += m->mMaxHP;

        m->readyWeapon ();

        if (m->mWeapon && m->mWeapon->isA (kRayGun)) {
            zhits += 1;
            continue;
        }

        while (clk < 60000) {
            shAttack *atk = NULL;

            int attack = 0;
            int damage = 0;

            if (!mAttacks.count ()) {
                break;
            } 

            clk += 100;
            ap += speedToAP (m->mSpeed);
            if (ap < 0) { 
                continue;
            }

            attack += RNG (1, 20);
            if (20 == attack) attack = 100;
                
            if (m->mWeapon) {
                atk = & ((shWeaponIlk *) m->mWeapon->mIlk) -> mAttack;
                if (m->mWeapon->isMeleeWeapon ()) {
                    damage = ABILITY_MODIFIER (m->getStr ());
                } else {
                    damage = 0;
                    --attack; /* assume range penalty */
                }
                attack += ((shWeaponIlk *) m->mWeapon->mIlk) -> mToHitBonus;
                attack += m->getWeaponSkillModifier (m->mWeapon->mIlk);
                attack += m->mWeapon->mEnhancement;
                damage += m->mWeapon->mEnhancement;
                if (!m->mWeapon->isMeleeWeapon ()) {
                    rattacks++;
                } else {
                    attacks++;
                }
            } else {
                atk = mAttacks.get (RNG (mAttacks.count ()));
                if (atk->isSecondaryAttack ()) {
                    damage = ABILITY_MODIFIER (m->getStr ());
                    if (damage > 0) {
                        damage /= 2;
                    }
                } else {
                    damage = ABILITY_MODIFIER (m->getStr ());
                }
                ++attacks;
            }
            

            ap -= atk->mAttackTime;



            attack += m->mBAB;
            attack += m->mToHitModifier;
            damage += NDX (atk->mDamage[0].mNumDice,
                           atk->mDamage[0].mDieSides);
            if (acfoo > 22) { /* spot hero +2 agi bonus */
                damage -= (acfoo - 21) / 2;
            }
            if (damage < 1) damage = 1;
            if (attack > acfoo) {
                if (m->mWeapon && !m->mWeapon->isMeleeWeapon ()) {
                    rdamagefoo += damage;
                    ++rhits;
                } else {
                    damagefoo += damage;
                    ++hits;
                }

            }
            if (attack >= 15) {
                damage15 += damage;
            }
            if (attack >= 20) {
                damage20 += damage;
            }
            if (attack >= 25) {
                damage25 += damage;
            }
        }
            
        delete m;
    }



    difficulty = damagefoo;

    fprintf (spoilerfile,
             "%20s %2d %3d %4d %4d %3d/%3d %4d %3d/%3d %4d %2d\n",
             mName, 
             AC / n, HP / n, speed / n, 
             acfoo,
             rhits / (n),
             rattacks / (n), 
             rdamagefoo / (n),
             hits / (n),
             attacks / (n), 
             damagefoo / (n),
             zhits);

/*
    I->p ("                                     %4d/15 %4d/20 %4d/25", 
          damage15 / (6 * n),
          damage20 / (6 * n),
          damage25 / (6 * n));
*/
}





void
monsterSpoilers ()
{
    shMonsterIlk *ilk;
    int i;

    spoilerfile = fopen ("monsters.txt", "w");

    I->p ("Generating spoilers, please be patient...");

    fprintf (spoilerfile, "%20s %2s %3s %4s %2s %3s/%3s %4s %3s/%3s %4s\n",
             "Monster", "AC", "HP", "Spd", "vsAC", "hit", "rgd", "dmg", "hit", "atk", "dmg");

    for (i = 0; i < MonsterIlks.count (); i++) {
        ilk = MonsterIlks.get (i);
        ilk->spoilers ();
    }
    fclose (spoilerfile);
    I->p ("Spoilers saved in monsters.txt");
}
