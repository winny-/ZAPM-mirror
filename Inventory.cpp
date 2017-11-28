/* Hero inventory functions */

#include "Global.h"
#include "Util.h"
#include "Hero.h"
#include "Creature.h"
#include "Object.h"
#include "Interface.h"


void
shHero::reorganizeInventory ()
{
    int i;
    int j;
    shObject *obj;
    shObject *obj2;
#ifdef MERGEMESSAGE
    char buf[80];
#endif

    for (i = 0; i < mInventory->count (); i++) {
        obj = mInventory->get (i);
        if (hasBugSensing ()) {
            obj->setBugginessKnown ();
        }
        if (!isBlind ()) {
            obj->setAppearanceKnown ();
        }
        for (j = 0; j < i; j++) {
            obj2 = mInventory->get (j);
            if (obj2->canMerge (obj)) {
                mInventory->remove (obj);               
                obj2->merge (obj);
#ifdef MERGEMESSAGE
                I->p ("Merging: %c - %s", obj2->mLetter, obj2->inv ());
#endif
                --i;
            }
        }
            
    }
}


int
shHero::addObjectToInventory (shObject *obj, int quiet /* = 0 */)
{
    int i;
    char spots[52];

    if (hasBugSensing ()) {
        obj->setBugginessKnown ();
    }

    const char *what = AN (obj);
        
    for (i = 0; i < 52; spots[i++] = 0);
    for (i = 0; i < mInventory->count (); i++) {
        shObject *iobj;
        char let;
        int spot;

        iobj = mInventory->get (i);
        if (iobj->canMerge (obj)) {
            mWeight += obj->getMass ();
            iobj->merge (obj);
            if (0 == quiet) {
                I->p ("%c - %s", iobj->mLetter, what);
            }
            computeIntrinsics ();
            return 1;
        }       
        let = iobj->mLetter;
        spot = 
            (let >= 'a' && let <= 'z') ? let - 'a' :
            (let >= 'A' && let <= 'Z') ? let - 'A' + 26 :
            -1;
            
        assert (-1 != spot);
        assert (0 == spots[spot]);
        spots[spot] = 1;
    }

    for (i = 0; i < 52; i++) {
        if (0 == spots[i]) {
            mInventory->add (obj);
            mWeight += obj->getMass ();
            obj->mLetter = i < 26 ? i + 'a' : i + 'A' - 26;
            obj->mLocation = shObject::kInventory;
            obj->mOwner = this;
            if (0 == quiet) {
                I->p ("%c - %s", obj->mLetter, what);
            }
            mInventory->sort (&compareObjects);
            computeIntrinsics ();
            return 1;
        }
    }

    I->p ("You don't have any room in your pack for %s.", what);
    return 0;
}


int
selectObjects (shObjectVector *dest, shObjectVector *src,
               shObjectIlk *ilk)
{
    int i;
    int n = 0;

    for (i = 0; i < src->count (); i++) {
        if (src->get (i) -> isA (ilk)) {
            dest->add (src->get (i));
            n++;
        }
    }
    return n;
}

int
selectObjects (shObjectVector *dest, shObjectVector *src,
               shObjectType type)
{
    int i;
    int n = 0;

    for (i = 0; i < src->count (); i++) {
        if ((kMaxObjectType == type) || (src->get (i) -> isA (type))) {
            dest->add (src->get (i));
            n++;
        }
    }
    return n;
}


int
selectObjectsByFlag (shObjectVector *dest, shObjectVector *src, int flag)
{
    int i;
    int n = 0;

    for (i = 0; i < src->count (); i++) {
        if (flag & src->get (i) -> mIlk -> mFlags) {
            dest->add (src->get (i));
            n++;
        }
    }
    return n;
}


int
selectObjectsByFunction (shObjectVector *dest, shObjectVector *src, 
                         int (shObject::*idfunc) (), int neg)
{
    int i;
    int n = 0;

    for (i = 0; i < src->count (); i++) {
        if ((src->get (i)->*idfunc) () ? !neg : neg)  {
            dest->add (src->get (i));
            n++;
        }
    }
    return n;
}


int
unselectObjectsByFunction (shObjectVector *dest, shObjectVector *src, 
                           int (shObject::*idfunc) ())
{
    return selectObjectsByFunction (dest, src, idfunc, 1);
}


int
compare_letters (const void *a, const void *b)
{
    if (*(char*) a < *(char*)b)
        return -1;
    else if (*(char*)a > *(char*)b)
        return 1;
    else 
        return 0;
}


// EFFECTS: prompts the user to select an item
//          if type is kMaxObjectType, no type restriction

shObject *
shHero::quickPickItem (shObjectVector *v, const char *action, int flags,
                       int *count /* = NULL */ )
{
    int i, j;
    char letterlist[256];
    char fastlist[256];
    char first;
    char pick;
    int cnt;

    if (0 == v->count () && !(flags & shMenu::kAnythingAllowed)) {
        I->p ("You don't have anything to %s.", action);
        return NULL;
    } 
    for (i = 0; i < v->count (); i++) {
        letterlist[i] = v->get (i) -> mLetter;
    }
    letterlist[i] = 0;
    qsort (letterlist, i, sizeof(char), compare_letters);
    first = 0;
    cnt = 0;
    i = 0;
    j = -1;

    while (i < v->count ()) {
        if (letterlist[i] == first + cnt) { /* series */
            ++cnt;
            if (cnt <= 3) {
                fastlist[++j] = letterlist[i];
            } else if (4 == cnt) {
                fastlist[j-1] = '-';
                fastlist[j] = letterlist[i];
            } else {
                fastlist[j] = letterlist[i];
            }
            ++i;
        } else {
            first = fastlist[++j] = letterlist[i++];
            cnt = 1;
        }
    }
    fastlist[++j] = 0;
        

 tryagain:
    I->p ("What do you want to %s? [%s%s%s]", action, 
          flags & shMenu::kNothingAllowed ? "- " : "",
          fastlist,
          flags & shMenu::kAnythingAllowed
                 ? 0 == v->count () ? "*" : " or ?*" 
                 : 0 == v->count () ? "" : " or ?"); 
    cnt = 0;

    while (1) {
        pick = I->getChar (I->logWin());
    
        if ('*' == pick) {
            if (flags & shMenu::kAnythingAllowed) {
                int i;
                char actionbuf[40]; 
                snprintf (actionbuf, 40, "%s what?", action);
                                
                shMenu menu (actionbuf, flags);
                shObject *obj;
                for (i = 0; i < mInventory->count (); i++) {
                    obj = mInventory->get (i);
                    menu.addItem (obj->mLetter, obj->inv (), obj, obj->mCount);
                }
                menu.getResult ((const void **) &obj, count);
                if (!obj) {
                    I->pageLog ();
                    I->nevermind ();
                }
                return obj;
            }
        }

        if ('?' == pick) {
            if (v->count ()) {
                int i;
                char actionbuf[40]; 
                snprintf (actionbuf, 40, "%s what?", action);
                shMenu menu (actionbuf, flags);
                shObject *obj;
                for (i = 0; i < v->count (); i++) {
                    obj = v->get (i);
                    menu.addItem (obj->mLetter, obj->inv (), obj, obj->mCount);
                }
                menu.getResult ((const void **) &obj, count);
                if (!obj) {
                    I->pageLog ();
                    I->nevermind ();
                }
                return obj;
            }
        }

        if ((pick >= '0') && (pick <= '9')) {
            if (flags & shMenu::kCountAllowed) {
                cnt = 10 * cnt + pick - '0';
                continue;
            }
            else {
                I->pageLog ();
                I->p ("No count allowed with this command.");
                goto tryagain;
            }
        }

        if ('-' == pick) {
            if (flags & shMenu::kNothingAllowed) {
                return TheNothingObject;
            }
            I->nevermind ();
            return NULL;
        }
        
        if (!(flags & shMenu::kAnythingAllowed)) {
            for (i = 0; i < v->count (); i++) {
                shObject *obj = v->get (i);             
                if (pick == obj->mLetter) {
                    if (NULL != count) {
                        *count = (cnt > 0  && cnt < obj->mCount) 
                            ? cnt : obj->mCount;
                        
                    }
                    return obj;
                }
            }
        }

        for (i = 0; i < mInventory->count (); i++) {
            shObject *obj = mInventory->get (i);
            if (pick ==  obj->mLetter) {
                if (flags & shMenu::kAnythingAllowed) {
                    if (NULL != count) {
                        *count = cnt > 0 ? cnt : obj->mCount;
                    }
                    return obj;
                }
                else {
                    I->pageLog ();
                    I->p ("You can't %s that!", action);
                    goto tryagain;
                }
            }
        }
        I->pageLog ();
        I->nevermind ();
//      I->p ("You don't have such an object.");
        return NULL;
    }
}
