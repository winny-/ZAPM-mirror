#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Creature.h"
#include "Interface.h"

/******************************************************

shopping code

******************************************************/


/* called when the hero enters a shop room */

void
shHero::enterShop ()
{
    shMonster *shopkeeper = mLevel->getShopKeeper (mX, mY);
    char buf[50];
    
    if (shopkeeper) {
        if (shopkeeper->isHostile ()) {
            return;
        }
        if (tryToTranslate (shopkeeper)) {
            switch (mLevel->getRoom (mX, mY) -> mType) {
            case shRoom::kGeneralStore:
                I->p ("\"Welcome to 'All-Mart'!  How may I help you?\""); 
                break;
            case shRoom::kHardwareStore:
                I->p ("\"Welcome to 'Pod Depot', "
                      "the cheapest hardware store around!\""); 
                break;
            case shRoom::kSoftwareStore:
                I->p ("\"Welcome to 'MegaSoft'!\""); break;
            case shRoom::kArmorStore:
                I->p ("\"Welcome to 'Swashbuckler Outfitters'!\""); break;
            case shRoom::kWeaponStore:
                /* this joke shamelessly stolen from "The Simpsons": */
                I->p ("\"Welcome to 'Bloodbath and Beyond'.\""); 
                break;
            case shRoom::kImplantStore:
                I->p ("\"Welcome to Steve's Emporium of Previously Enjoyed"
                      "Bionic implants!\""); break;
            default:
                I->p ("\"Welcome!\""); break;
            }
        } else {
            shopkeeper->the (buf, 50);
            I->p ("%s beeps and bloops enthusiastically.", buf);
            Hero.resetStoryFlag ("theft warning");
        }
    } else {
        I->p ("Hmm...  This store appears to be deserted.");
    }
}


void
shHero::leaveShop ()
{
    shMonster *shopkeeper = mLevel->getShopKeeper (mLastX, mLastY);
    char buf[50];
    shObjectVector v;
    int i;

    if (selectObjectsByFunction (&v, mInventory, &shObject::isUnpaid)) {
        /* the goods are no longer unpaid - they're stolen! */
        for (i = 0; i < v.count (); i++) {
            v.get (i) -> resetUnpaid ();
        }
    }
    if (!shopkeeper) {
        if (v.count ()) {
            /* successfully robbed the shop! */
            I->p ("You make off with the loot.");
        }
        return;
    }
    shopkeeper->the (buf, 50);
    if (shopkeeper->isHostile ()) {
        if (tryToTranslate (shopkeeper)) {
            I->p ("\"Not so fast!\"");
        } else {
            I->p ("%s is beeping frantically!", buf);
        }
    } else if (0 == v.count ()) {
        if (tryToTranslate (shopkeeper)) {
            I->p ("\"Come again soon!\"");
        } else {
            I->p ("%s twitters cheerfully.", buf);
        }
    } else {
        I->p ("You left with unpaid merchandise!");
        shopkeeper->newEnemy (this);
    }
    Hero.resetStoryFlag ("theft warning");
}


void
shHero::usedUpItem (shObject *obj, int cnt, const char *action)
{
    char buf[50];
    char buf2[50];
    shMonster *shopkeeper = mLevel->getShopKeeper (mX, mY);

    if (shopkeeper && 
        !shopkeeper->isHostile () &&
        obj->isUnpaid ()) 
    {
        Hero.resetStoryFlag ("theft warning");
        if (cnt > obj->mCount) {
            cnt = obj->mCount;
        }
        int price = obj->mIlk->mCost * cnt;

        shopkeeper->the (buf, 50);
        if (tryToTranslate (shopkeeper)) {
            I->p ("\"You %s it, you bought it!\"", action);
            obj->getShortDescription (buf2, 50);
            I->p ("You owe %s $%d for %s %s.", 
                  buf, price, cnt > 1 ? "those" : "that", buf2);
        } else {
            I->p ("%s testily beeps something about %d buckazoids.", 
                  buf, price);
        }
        shopkeeper->mShopKeeper.mBill += price;
    }
}


void
shHero::pickedUpItem (shObject *obj)
{
    char buf[50];
    shMonster *shopkeeper = mLevel->getShopKeeper (mX, mY);

    if (shopkeeper && 
        obj->isUnpaid () && 
        !shopkeeper->isHostile () &&
        shopkeeper->canSee (this)) 
    {
        Hero.resetStoryFlag ("theft warning");
        int price = obj->mIlk->mCost * obj->mCount;
        if (tryToTranslate (shopkeeper)) {
#if 0
/* I think it's too easy to identify items for free! */
            obj->setIlkKnown (); /* the clerk knows what he's selling, but 
                                    not if it's buggy or optimized */
#endif    
            switch (RNG (5)) {
            case 0:
                obj->these (buf, 50);
                I->p ("\"%s %s yours for only %d buckazoids!\"", buf, 
                      obj->mCount > 1 ? "are" : "is", price); 
                break;
            case 1:
                obj->these (buf, 50);
                I->p ("\"Take %s home for %d buckazoids!\"", buf, price); 
                break;
            case 2:
                obj->these (buf, 50);
                I->p ("\"Only $%d for %s!\"", price, buf); 
                break;
            case 3:
                obj->an (buf, 50);
                I->p ("\"%s, on sale for %d buckazoids!\"", buf, price); 
                break;
            case 4:
                obj->an (buf, 50);
                I->p ("\"%s for $%d!  Such a deal!\"", buf, price); 
                break;
            }
        } else {
            char shbuf[50];
            shopkeeper->the (shbuf, 50);
            I->p ("%s beeps something about %d buckazoids.", shbuf, price);
        }
    }
}


void
shHero::maybeSellItem (shObject *obj)
{
    char buf[50];
    char buf2[50];
    shMonster *shopkeeper = mLevel->getShopKeeper (mX, mY);

    if (shopkeeper && 
        !shopkeeper->isHostile () &&
        !obj->isUnpaid () && 
        !obj->isA (kMoney) &&
        shopkeeper->canSee (this)) 
    {
        int price = obj->mIlk->mCost * obj->mCount / 2;
        int quote = price;

        Hero.resetStoryFlag ("theft warning");

        if (shopkeeper->countMoney () < quote) {
            quote = shopkeeper->countMoney ();
        }

        shopkeeper->the (buf, 50);
        obj->an (buf2, 50);

        if (0 == quote) {
            if (tryToTranslate (shopkeeper)) {
                I->p ("%s is unable to buy %s from you.", buf, buf2);
            } else {
                I->p ("%s whirs disappointedly.", buf, buf2);
            }
            return;
        } else if (quote < price) {
            if (tryToTranslate (shopkeeper)) {
                I->p ("\"I can only offer you $%d for %s.\"", quote, buf2);
            } else {
                I->p ("%s chirps and beeps something about %d buckazoids.",
                      buf, quote);
            }
        } else {
            if (tryToTranslate (shopkeeper)) {
                I->p ("%s offers to buy %s for %d buckazoids.", buf, buf2, 
                      quote);
            } else {
                I->p ("%s chirps and beeps something about %d buckazoids.",
                      buf, quote);
            }
        }
        if (I->yn ("Sell %s?", obj->mCount > 1 ? "them" : "it")) {
            obj->setUnpaid ();
            shopkeeper->loseMoney (quote);
            gainMoney (quote);
        }
        I->pageLog ();
    }
}


void
shHero::payShopkeeper ()
{
    char buf[80];
    shMonster *shopkeeper = mLevel->getShopKeeper (mX, mY);
    shMonster *guard = mLevel->getGuard (mX, mY);
    int i;
    shObject *obj;
    shObjectVector v;
    int price;

    if (!shopkeeper && !guard) {
        I->p ("There's nobody around to pay.");
        return;
    }

    if (guard) {
        guard->the (buf, 50);
        if (guard->isHostile ()) {
            I->p ("%s intends to collect payment from your dead body!", buf);
            return;
        }
        price = guard->mGuard.mToll;
        if (0 == price) {
            I->p ("You've already paid the toll.");
            return;
        } else if (price > 0) {
            if (I->yn ("Pay %s %d buckazoids?", buf, price)) {
                if (price > countMoney ()) {
                    I->p ("But you don't have that much money.");
                    return;
                } else {
                    loseMoney (price);
                    guard->gainMoney (price);
                    guard->mGuard.mToll = 0;
                    if (tryToTranslate (guard)) {
                        I->p ("\"You may pass.");
                    } else {
                        I->p ("%s beeps calmly.", buf);
                    }
                }
            }
        }
        return;
    }

    if (shopkeeper->isHostile ()) {
        /* the record-keeping of the shopping bill is somewhat up in the air
           once the shopkeeper is angry, so the quick and dirty solution is
           not to allow pacification
         */
        shopkeeper->the (buf, 50);
        I->p ("%s intends to collect payment from your dead body!", buf);
        return;
    }
    
    price = shopkeeper->mShopKeeper.mBill;
    if (price) {
        I->p ("You owe %d buckazoids for the use of merchandise.", price);
        if (price > countMoney ()) {
            I->p ("But you don't have enough money to cover that bill.");
        } else {
            loseMoney (price);
            shopkeeper->gainMoney (price);
            shopkeeper->mShopKeeper.mBill = 0;
            I->p ("You pay that amount.");
        }
        I->pause ();
    }

    if (0 == selectObjectsByFunction (&v, mInventory, &shObject::isUnpaid)) {
        I->p ("You don't have any merchandise to pay for.");
        return;
    }

    shMenu menu ("Pay for which items?",
                 shMenu::kMultiPick | shMenu::kCategorizeObjects);
    for (i = 0; i < v.count (); i++) {
        obj = v.get (i);
        obj->inv (buf, 80);
        menu.addItem (obj->mLetter, buf, obj, obj->mCount);
    }
    while (menu.getResult ((void **) &obj)) {
        price = obj->mCount * obj->mIlk->mCost;
        if (price > countMoney ()) {
            obj->the (buf, 80);
            I->p ("You can't afford to pay for %s.", buf);
        } else {
            loseMoney (price);
            shopkeeper->gainMoney (price);
            if (tryToTranslate (shopkeeper)) {
                if (RNG (2)) {
                    /* sometimes, the clerk knows what he's selling, but 
                       never if it's buggy or optimized. */
                    obj->setIlkKnown (); 
                }
                obj->the (buf, 80);
                I->p ("\"That'll be %d buckazoids for %s, thank you.\"", 
                      price, buf);
            } else {
                obj->the (buf, 80);
                I->p ("You buy %s for %d buckazoids.", buf, price);
            }
            obj->resetUnpaid ();
        }
    }
    Hero.resetStoryFlag ("theft warning");

}


/* shopkeeper strategy: 
     stand in front of the shop doorway unless hero is there
                   
   returns ms elapsed, -2 if the monster dies
*/
int
shMonster::doShopKeep ()
{    
    I->debug ("  shopkeeper strategy");
    int elapsed;
    char buf[50];
    int res = -1;
    int retry = 3;
    
    the (buf, 50);

    while (-1 == res) {
        if (!retry--) {
            return 200;
        }

        switch (mTactic) {

        case kNewEnemy:
            mStrategy = kAngryShopKeep;
            mTactic = kReady;
            return doAngryShopKeep ();
        case kMoveTo:
            res = doMoveTo ();
            continue;
        case kReady:    
            if (!Level->isInShop (mX, mY)) {
                /* somehow, we're not in our shop! */
                
                mDestX = mShopKeeper.mHomeX;
                mDestY = mShopKeeper.mHomeY;
                
                if (setupPath ()) {
                    mTactic = kMoveTo;
                    continue;
                } else {
                    return 2000;
                }
            }

            if (canSee (&Hero) && 
                Level->isInShop (Hero.mX, Hero.mY)) 
            {
                if (Level->isInDoorWay (Hero.mX, Hero.mY)) {
                    shObjectVector v;
                    if (selectObjectsByFunction (&v, Hero.mInventory, 
                                                 &shObject::isUnpaid)) 
                    {
                        /* Hero about to leave w/ unpaid merchandise */
                        if (!Hero.getStoryFlag ("theft warning")) {
                            if (Hero.tryToTranslate (this)) {
                                I->p ("\"Please don't "
                                      "leave without paying!\"");
                            } else {
                                I->p ("%s chitters urgently.", buf);
                            }
                            Hero.setStoryFlag ("theft warning", 1);
                        } 
                        mDestX = mShopKeeper.mHomeX;
                        mDestY = mShopKeeper.mHomeY;
                        res = doQuickMoveTo ();
                        continue;
                    } else if (mX == mShopKeeper.mHomeX && 
                               mY == mShopKeeper.mHomeY)
                    {
                        /* Hero is in the threshold of the shop doorway, and 
                           we're in the way! */
                        mDestX = mX;
                        mDestY = mY;
                        if (!mLevel -> 
                            findAdjacentUnoccupiedSquare (&mDestX, &mDestY)) 
                        {
                            elapsed = doQuickMoveTo ();
                            if (-1 == elapsed) elapsed = 800;
                            return elapsed;
                        }
                        return RNG (300, 1000); /* nowhere to go, just wait. */
                    } 
                } else if (0 == RNG (6)) {
                    /* move to an empty square near the home square */
                    mDestX = mShopKeeper.mHomeX;
                    mDestY = mShopKeeper.mHomeY;
                    if (!mLevel -> 
                        findAdjacentUnoccupiedSquare (&mDestX, &mDestY)) 
                    {
                        elapsed = doQuickMoveTo ();
                        if (-1 == elapsed) elapsed = 800;
                        return elapsed;
                    }
                    return RNG (300, 1000); /* nowhere to go, let's wait... */
                } else if (0 == RNG (50)) {
                    char *quips[3] = {
                        "I'd give them away, but my wifebot won't let me!",
                        "All merchandise sold as-is.",
                        "Shoplifters will be vaporized!",
                    };
                    if (Hero.tryToTranslate (this)) {
                        I->p ("\"%s\"", quips[RNG(3)]);
                    } /* */
                    return RNG (500, 1000);
                }
            } else {
                return RNG (800, 1600);
            }
        case kFight:
        case kShoot:
            mTactic = kReady;
            I->debug ("Unexped shopkeeper tactic!");
        }
    }

    return RNG (300, 1000); /* keep on lurking */
}




//returns ms elapsed, -2 if the monster dies
int
shMonster::doAngryShopKeep ()
{    
    if (canSee (&Hero) && 0 == RNG (10)) {
        if (Hero.tryToTranslate (this)) {
            I->p ("\"%s\"", 
                  RNG (2) ? "Stop, thief!" : "You shoplifting scum!");
        } else {
            char buf[50];
            the (buf, 50);
            I->p ("%s beeps angrilly at you.", buf);
        }
    }
    return doWander ();
}
