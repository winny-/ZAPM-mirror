#include "Global.h"
#include "Interface.h"
#include "Object.h"

shEvent::shEvent ()
{
    mCancelled = 0;
}

shEvent::shEvent (int msfromnow)
{
    mDay = 0; //TODO: real day
    mTime = Clock + msfromnow;
    mCancelled = 0;

#ifdef SH_DEBUG
    mComment[49] = 0;
#endif
}


shEvent::~shEvent ()
{
    
}


void
shEvent::setTimer (unsigned int msfromnow)
{
    mTime = Clock + msfromnow;
}


void
shEvent::increaseTime (unsigned int ms)
{
    mTime += ms;
}


void
shEvent::cancel ()
{
    // FIX: this is lame, should remove it from the event queue
    mCancelled = 1;
}


// These nifty priority queue routines from Cormen, Leiserson and Rivest.
// _Introduction to Algorithms_


void
shEventQueue::verify ()
{
    int i;
    
    for (i = 1; i < mCount; i++) {
        if (mItems[i]->mTime < mItems[(i-1)/2]->mTime) {
            I->p ("Event queue corruption has occured!!");
            abort ();
        }
    }
}



int
shEventQueue::insert (shEvent *event, shThing *thing)
{
    int i;
    shTime keytime = event->mTime;

    verify ();

    if (NULL != thing) {
        thing->addEvent (event);
        event->mTargets.add (thing);
    }

    if (++mCount == mCapacity) {
        grow (mCapacity * 2);
    }
    i = mCount - 1;
    while (i > 0 && keytime < mItems[(i-1)/2]->mTime) {
        mItems[i] = mItems[(i-1)/2];
        i = (i-1) / 2;
    }
    mItems[i] = event;
#if 0
    {
        char buf[50];
        int x;
        int n;
        n = sprintf (buf, "insert: ");
        for (x = 0; x < mCount; x++) {
            n += sprintf (buf + n, "%d ", mItems[x]->mTime);
        }
        I->debug (buf);
    }
#endif

    verify ();

    return i;
}


shEvent *
shEventQueue::extract ()
{
    shEvent *res;
    int i, l, r, smallest;

    verify ();

    if (0 == mCount) { // no events scheduled
        return NULL;
    }
    res = mItems[0];
    mItems[0] = mItems[--mCount];
    Clock = res->mTime;

    i = 0;
 heapify:
    l = 2 * i + 1;
    r = 2 * i + 2;
    if ((l < mCount) && (mItems[l]->mTime < mItems[i]->mTime)) {
        smallest = l;
    }
    else {
        smallest = i;
    }
    if ((r < mCount) && (mItems[r]->mTime < mItems[smallest]->mTime)) {
        smallest = r;
    }
    if (smallest != i) {
        shEvent *tmp = mItems[i];
        mItems[i] = mItems[smallest];
        mItems[smallest] = tmp;
        i = smallest;
        goto heapify;
    }
#if 0
    {
        char buf[50];
        int x;
        int n;
        n = sprintf (buf, "extract: %d <-- ", res->mTime);
        for (x = 0; x < mCount; x++) {
            n += sprintf (buf + n, "%d ", mItems[x]->mTime);
        }
        I->debug (buf);
    }
#endif
    verify ();

    if (res->mCancelled) {
        /* delete res; */
        return extract ();
    }
    else {
        for (i = 0; i < res->mTargets.count (); i++) {
            res->mTargets.get (i) -> removeEvent (res);
        }
        res->mTargets.reset ();
        return res;
    }
}


