/*******************************

      Quaffing from Vats

********************************/

#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Hero.h"
#include "Interface.h"



void
shMapLevel::addVat (int x, int y)
{
    shFeature *vat = new shFeature ();
    vat->mType = shFeature::kVat;
    vat->mX = x;
    vat->mY = y;
    vat->mVat.mHealthy = 0;
    vat->mVat.mRadioactive = 1;
    mFeatures.add (vat);
}





shAttack VatCorrosionAttack =
    shAttack (NULL, shAttack::kSlime, shAttack::kOther, 0,
              kCorrosive, 1, 6);

shAttack VatShockAttack =
    shAttack (NULL, shAttack::kZap, shAttack::kOther, 0, kElectrical, 1, 6);

void
shHero::quaffFromVat (shFeature *vat)
{
    assert (vat->mX == Hero.mX && vat->mY == Hero.mY);
    int dryup = 3;
    int result;

    if (vat->mVat.mRadioactive && !RNG (10)) {
        result = 10;
    } else if (RNG (2) < vat->mVat.mHealthy) {
        /* healthy vats are quite likely to give a good effect, and they're
           the only way to get the gain ability effect */
        result = RNG (0, 4);
    } else {
        result = RNG (1, 9);
    }

    switch (result) {
    case 0:
        quaffGainAbility (NULL);
        break;
    case 1:
        if (vat->mVat.mRadioactive) {
            getMutantPower ();
            dryup = 2;
            break;
        } /* else fall through... */
    case 2:
        mRad -= RNG (1, 200);
        mRad = maxi (0, mRad);
        if (!mRad)
            I->p ("You feel purified!");
        else 
            I->p ("You feel less contaminated.");
        break;
    case 3:
        mHP += NDX (4, 6);
        mHP = mini (mHP, mMaxHP);
        I->p ("You feel better!");
        break;
    case 4:
    {
        int amt = RNG (1, 6);
        Hero.mChaDrain -= amt;
        Hero.mAbil.mCha += amt;
        I->p ("You feel invigorated!");
        break;
    }
    case 5:
        I->p ("Mmmm... hot fun!");
        break;
    case 6:
        I->p ("You are jolted by an electric shock!");
        if (sufferDamage (&VatShockAttack)) {
            die (kKilled, "an improperly grounded sludge vat");
        }
        dryup = 2;
        break;
    case 7:
        I->p ("Mmmm... bouncy bubbly beverage!");
        if (Hero.getStoryFlag ("impregnation")) {
            I->p ("You feel the alien embryo inside you die.");
            Hero.setStoryFlag ("impregnation", 0);
        }
        break;
    case 8:
        I->p ("Oops!  You fall in!  You are covered in slime!");
        damageEquipment (&VatCorrosionAttack, kCorrosive);
        I->p ("You climb out of the vat.");
        break;
    case 9:
    {
        shMonster *monster;
        int x = mX;
        int y = mY;
        
        monster = new shMonster (findAMonsterIlk ("vat slime"));
        if (0 != Level->findNearbyUnoccupiedSquare (&x, &y) ||
            0 != Level->putCreature (monster, x, y)) {
            I->p ("The sludge gurgles!");
        } else {
            I->p ("It's alive!");
        }
        break;
    }
    case 10:
        mRad += NDX (2, 100);
        I->p ("Ick!  That had a toxic taste!");
        break;
    }

    if (!RNG (dryup)) {
        I->p ("The vat dries up!");
        Level->removeFeature (vat);
    }
}
