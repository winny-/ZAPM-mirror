// this file is included by Global.h, should not be included directly

#ifndef EVENT_H
#define EVENT_H

#include <stdarg.h>
#include <stdio.h>
#include <new>
#include "Util.h"

/* an shEntity is a game-state object that must be saved and loaded
   all shEntities must have a constructor that takes no arguments
*/

struct shEntity
{
    int mEId; /* set to -1 by default */

    shEntity () { mEId = -1; }
    virtual ~shEntity ();
    virtual void saveState (int fd);
    virtual void loadState (int fd);
};

struct shEvent : shEntity 
{
    enum Type {
        kUninitialized,
        kGameOver,
        kHeroEquip,
        kHeroReady,
        kHeroUpkeep,
        kMonsterReady,
        kMonsterSpawn
    };



    int mDay;
    shTime mTime;    
    int mCancelled;

    shVector <shThing *> mTargets; /* Things to which this event refers */

    virtual void fire () = 0;
    int getType () { return 0; }

    void setTimer (unsigned int msfromnow);
    void increaseTime (unsigned int ms);

/*  Cancel the event - it will not be fired. */
    void cancel ();

    virtual ~shEvent ();
    virtual void saveState (int fd);
    virtual void loadState (int fd);

 protected:
    shEvent ();
    shEvent (int msfromnow);

 public:
#ifdef SH_DEBUG
    char mComment[50];
#endif
    void
    setComment (char *format, ...) 
    {
#ifdef SH_DEBUG
        va_list ap;
        va_start (ap, format);
        vsnprintf (mComment, 49, format, ap);
        va_end (ap);
#endif    
    }

};

#define NEW_EVENT(_newtype) new (malloc (512)) _newtype


#define CONVERT_EVENT(_evt, _newtype) new (_evt) _newtype


class shEventQueue : shVector <shEvent *> {
 public:
//    shEventQueue (int capacity = 10);
    
    int insert (shEvent *event, shThing *thing);

    shEvent *remove (shEvent *event);
    
    shEvent *extract ();
    void verify ();
    void saveState (int fd);
    void loadState (int fd);
};




struct shGameOverEvent : public shEvent
{
    void fire () { };
    void saveState (int fd);
};

extern shGameOverEvent *gameOver;

#endif

