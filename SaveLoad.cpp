#include "Global.h"
#include "Object.h"
#include "Interface.h"
#include "Creature.h"
#include "Hero.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static void
safeRead (int fd, void *ptr, size_t len)
{
    while (len) {
	int result = read (fd, ptr, len);
	if (result <= 0) {
	    abort ();
	} else {
	    len -= result;
	}
    }
}


static void
safeWrite (int fd, void *ptr, size_t len)
{
    while (len) {
	int result = write (fd, ptr, len);
	if (result <= 0) {
	    abort ();
	} else {
	    len -= result;
	}
    }
}




#define SWRITE(_fd, _ptr, _len) \
    safeWrite ((_fd), (void *) (_ptr), (_len))
#define SREAD(_fd, _ptr, _len) \
    safeRead ((_fd), (void *) (_ptr), (_len))

#define SWRITEBLOCK(_fd, _p1, _p2) \
    SWRITE ((_fd), (_p1), ((char *) _p2) - ((char *) _p1));
#define SREADBLOCK(_fd, _p1, _p2) \
    SREAD ((_fd), (_p1), ((char *) _p2) - ((char *) _p1)); 


#if 0
static void SWRITE (int fd, void *ptr, int len)
{
    int res = write (
}
#endif

typedef shVector <void **> TargetList;

struct  UpdatePair 
{
    int mId;
    TargetList mTargets;

    UpdatePair (int id) { mId = id; }
};

static shVector <void *> PtrList;   /* maps pointers to integer identifiers */

shVector <UpdatePair *> UpdateList; /* keeps track of pointers for linking */


static void
saveMagic (int fd, unsigned int key)
{
    unsigned int magic[2] = { 0xc0debabe, key };
    SWRITE (fd, &magic, sizeof(magic));
}

static void
loadMagic (int fd, unsigned int key)
{
    unsigned int magic[2];
    SREAD (fd, &magic, sizeof(magic));
    if (0xc0debabe != magic[0] ||
        key != magic[1]) 
    {
        abort ();
    }
}

static void
magicVersionString (char *buf)
{
    snprintf (buf, 12, "ZapM        ");
    snprintf (&buf[4], 12, "%s", ZAPM_VERSION);
}

static void
saveHeader (int fd)
{
    char magic[12];
    magicVersionString (magic);
    SWRITE (fd, magic, 12);
}

static int
loadHeader (int fd)
{
    char magic[12];
    char buf[12];
    char *m;
    char *b;

    /* compare only the first two numbers in the version string */

    magicVersionString (magic);
    m = strchr (magic, '.');
    if (m) m = strchr (m + 1, '.');
    if (m) *m = '\0';

    SREAD (fd, buf, 12);
    b = strchr (buf, '.');
    if (b) b = strchr (b + 1, '.');
    if (b) *b = '\0';

    if (strncmp (magic, buf, 12)) {
        I->p ("Couldn't load save file due to version incompatibility.");
        return -1;
    }
    return 0;
}

static void
savePointer (int fd, void *ptr)
{
    saveMagic (fd, 1);

    if (NULL == ptr) { 
        int id = -1;
        SWRITE (fd, &id, sizeof (int));
    } else {
        int id = PtrList.find (ptr);
        
        if (-1 == id) {
            id = PtrList.add (ptr);
        }
        SWRITE (fd, &id, sizeof (int));
        I->debug ("savePointer %3d %p", id, ptr);
    }
}


static void
loadPointer (int fd, void **ptr, const char *msg)
{
    int id;

    loadMagic (fd, 1);

    SREAD (fd, &id, sizeof (int));

    *ptr = NULL;
    if (-1 == id) {
        return;
    }
    if (id < PtrList.count ()) {
        *ptr = PtrList.get (id); /* might set to NULL */
    } 
    if (NULL == *ptr) {
        UpdatePair *p;
        int i;

        I->debug ("loadPointer %3d 0x???????? <- %p %s", id, ptr, msg);
        for (i = 0; i < UpdateList.count (); i++) {
            p = UpdateList.get (i);
            if (p->mId == id) {
                p->mTargets.add (ptr);
                return;
            }
        }
        p = new UpdatePair (id);
        p->mTargets.add (ptr);
        UpdateList.add (p);

    } else {
        I->debug ("loadPointer %3d %p <- %p %s", id, *ptr, ptr, msg);
    }
}


/* loads a new object pointer */

static void
loadStruct (int fd, void *ptr)
{
    int id;
    int i;
    int j;

    loadMagic (fd, 1);

    SREAD (fd, &id, sizeof (int));

    if (id >= PtrList.count ()) {

        PtrList.ensure (id);
        
        for (i = PtrList.count (); i < id; i++) {
            PtrList.set (i, NULL);
        }
    }
    PtrList.set (id, ptr);

    I->debug ("loadStruct  %3d %p", id, ptr);

/*
    int check = PtrList.add (ptr);
    assert (check == id);
*/
    for (i = 0; i < UpdateList.count (); i++) {
        UpdatePair *p = UpdateList.get (i);
        if (p->mId == id) {
            for (j = 0; j < p->mTargets.count (); j++) {
                * p->mTargets.get (j) = ptr;
                I->debug ("                %p <- %p", 
                          ptr, p->mTargets.get (j));
            }
            UpdateList.remove (p);
            delete p;
            return;
        }
    }
}


static void
saveTarget (int fd, void *ptr)
{
    savePointer (fd, ptr);
}


static void
saveOptions (int fd)
{
    SWRITE (fd, &Flags, sizeof (Flags));
}

static void
loadOptions (int fd)
{
    SREAD (fd, &Flags, sizeof (Flags));
}

static void
saveIlks (int fd)
{
    SWRITE (fd, CanisterData, sizeof (CanisterData));
    SWRITE (fd, ImplantData, sizeof (ImplantData));
    SWRITE (fd, FloppyData, sizeof (FloppyData));
    SWRITE (fd, RayGunData, sizeof (RayGunData));
    SWRITE (fd, JumpsuitData, sizeof (JumpsuitData));
    SWRITE (fd, BeltData, sizeof (BeltData));
    
    SWRITE (fd, MedicalProcedureData, sizeof (MedicalProcedureData));
}

static void
loadIlks (int fd)
{
    SREAD (fd, CanisterData, sizeof (CanisterData));
    SREAD (fd, ImplantData, sizeof (ImplantData));
    SREAD (fd, FloppyData, sizeof (FloppyData));
    SREAD (fd, RayGunData, sizeof (RayGunData));
    SREAD (fd, JumpsuitData, sizeof (JumpsuitData));
    SREAD (fd, BeltData, sizeof (BeltData));

    SREAD (fd, MedicalProcedureData, sizeof (MedicalProcedureData));
}

static void
saveInt (int fd, int x)
{
    saveMagic (fd, 99);

    SWRITE (fd, &x, sizeof (int));
}

static int
loadInt (int fd)
{
    int x;

    loadMagic (fd, 99);

    SREAD (fd, &x, sizeof (int));
    return x;
}

static void
saveString (int fd, const char *str)
{
    saveMagic (fd, 22);

    if (NULL == str) {
        saveInt (fd, 0);
    } else {
        saveInt (fd, strlen (str));
        SWRITE (fd, str, strlen (str));
    }
}

static void
loadString (int fd, const char **str)
{
    loadMagic (fd, 22);


    int len = loadInt (fd);
    if (0 == len) {
        *str = NULL;
    } else {
        *str = (const char *) malloc (len + 1);
        SREAD (fd, *str, len);
        ((char *)(*str))[len] = 0;
    }
}


static void
saveDiscoveries (int fd)
{
    int i, n;
    n = ObjectIlks.count ();
    for (i = 0; i < n; i++) {
        shObjectIlk *ilk = ObjectIlks.get (i);
        saveInt (fd, ilk->mFlags & kIdentified);
        saveString (fd, ilk->mUserName);
    }
}


static void
loadDiscoveries (int fd)
{
    int i, n;
    n = ObjectIlks.count ();
    for (i = 0; i < n; i++) {
        shObjectIlk *ilk = ObjectIlks.get (i);
        ilk->mFlags |= loadInt (fd);
        loadString (fd, &ilk->mUserName);
    }
}

#if 0
void
shEventQueue::saveState (int fd)
{
    int i, n;
    n = count ();
    saveInt (fd, n);
    for (i = 0; i < n; i++) {
        get (i) -> saveState (fd);
    }
}

static shEvent *
loadEvent (int fd)
{
   shEvent::Type type;

   shEvent *event;
   type = (shEvent::Type) loadInt (fd);
   switch (type) {
   case shEvent::kGameOver:
       event = new shGameOverEvent (); break;
   case shEvent::kHeroEquip:
       event = new shHeroEquipEvent (); break;
   case shEvent::kHeroReady:
       event = new shHeroReadyEvent (); break;
   case shEvent::kHeroUpkeep:
       event = new shHeroUpkeepEvent (); break;
   case shEvent::kMonsterReady:
       event = new shMonsterReadyEvent (); break;
   case shEvent::kMonsterSpawn:
       event = new shMonsterSpawnEvent (); break;
   default:
       abort ();
   }
   event->loadState (fd);
   return event;
}

void
shEventQueue::loadState (int fd)
{
    int i, n;
 
    n = loadInt (fd);
    for (i = 0; i < n; i++) {
        Q.add (loadEvent (fd));
    }
}
#endif

static void
saveMaze (int fd)
{
    int i;
    
    saveInt (fd, Maze.count ());
    for (i = 1; i < Maze.count (); i++) {
        Maze.get (i) -> saveState (fd);
        saveMagic (fd, 253);
    }
}


static void
loadMaze (int fd)
{
    int i, n;

    n = loadInt (fd);
    Maze.add (NULL); /* dummy zeroth level */
    for (i = 1; i < n; i++) {
        shMapLevel *level = new shMapLevel ();
        Maze.add (level);
        level->loadState (fd);
        loadMagic (fd, 253);
    }
}


void
shCreature::saveState (int fd)
{
    int i, n;

    saveMagic (fd, 'CRE>');
    saveTarget (fd, this);

    saveInt (fd, mX);
    saveInt (fd, mY);
    saveInt (fd, mZ);
    savePointer (fd, mLevel);
    saveInt (fd, mIlk->mId);

    SWRITE (fd, &mType, sizeof (shCreatureType));
    if (mProfession) {
        saveInt (fd, mProfession->mId);
    } else {
        saveInt (fd, -1);
    }

    SWRITEBLOCK (fd, &mName, &mWeapon);

    savePointer (fd, mWeapon);
    savePointer (fd, mHelmet);
    savePointer (fd, mBodyArmor);
    savePointer (fd, mJumpsuit);
    savePointer (fd, mBoots);
    savePointer (fd, mGoggles);
    savePointer (fd, mBelt);
    for (i = 0; i < shImplantIlk::kMaxSite; i++) {
        savePointer (fd, mImplants[i]);
    }
    switch (mMimic) {
    case kObject:  saveInt (fd, mMimickedObject->mId); break;
    case kFeature: saveInt (fd, mMimickedFeature); break;
    case kMonster: saveInt (fd, mMimickedMonster->mId); break;
    case kNothing: 
    default: 
        saveInt (fd, 0); break;
    }
    

    n = mTimeOuts.count ();
    SWRITE (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        SWRITE (fd, mTimeOuts.get (i), sizeof (shCreature::TimeOut));
    }

    n = mSkills.count ();
    SWRITE (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        shSkill *skill = mSkills.get (i);
        saveInt (fd, skill->mCode);
        saveInt (fd, skill->mPower);
        saveInt (fd, skill->mRanks);
        saveInt (fd, skill->mBonus);
        saveInt (fd, skill->mExercise);
        saveInt (fd, skill->mAccess);
    }

    n = mInventory->count ();
    SWRITE (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        mInventory->get (i) -> saveState (fd);
    }

    savePointer (fd, mLastLevel);
    saveMagic (fd, '<CRE');   
}


void
shCreature::loadState (int fd)
{
    int i, n;

    loadMagic (fd, 'CRE>');
    loadStruct (fd, this);

    mX = loadInt (fd);
    mY = loadInt (fd);
    mZ = loadInt (fd);
    loadPointer (fd, (void **) &mLevel, "c.mLevel");
    i = loadInt (fd);
    mIlk = MonsterIlks.get (i);

    SREAD (fd, &mType, sizeof (shCreatureType));
    n = loadInt (fd);
    if (-1 == n) { 
        mProfession = NULL;
    } else {
        mProfession = Professions.get (n);
    }

    SREADBLOCK (fd, &mName, &mWeapon);

    loadPointer (fd, (void **) &mWeapon, "c.mWeapon");
    loadPointer (fd, (void **) &mHelmet, "c.mHelmet");
    loadPointer (fd, (void **) &mBodyArmor, "c.mBodyArmor");
    loadPointer (fd, (void **) &mJumpsuit, "c.mJumpsuit");
    loadPointer (fd, (void **) &mBoots, "c.mBoots");
    loadPointer (fd, (void **) &mGoggles, "c.mGoggles");
    loadPointer (fd, (void **) &mBelt, "c.mBelt");
    for (i = 0; i < shImplantIlk::kMaxSite; i++) {
        loadPointer (fd, (void **) &mImplants[i], "c.mImplants");
    }
    switch (mMimic) {
    case kObject: mMimickedObject = ObjectIlks.get (loadInt (fd)); break;
    case kFeature: mMimickedFeature = (shFeature::Type) loadInt (fd); break;
    case kMonster: mMimickedMonster = MonsterIlks.get (loadInt (fd)); break;
    case kNothing: 
    default: 
        loadInt (fd); break;
    }
    
    SREAD (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        TimeOut *t = new TimeOut ();
        SREAD (fd, t, sizeof (TimeOut));
        mTimeOuts.add (t);
    }

    SREAD (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        shSkill *skill = new shSkill (kUninitializedSkill, kNoMutantPower);

        skill->mCode = (shSkillCode) loadInt (fd);
        skill->mPower = (shMutantPower) loadInt (fd);
        skill->mRanks = loadInt (fd);
        skill->mBonus = loadInt (fd);
        skill->mExercise = loadInt (fd);
        skill->mAccess = loadInt (fd);
        mSkills.add (skill);
    }

    SREAD (fd, &n, sizeof (int));
    for (i = 0; i < n; i++) {
        shObject *obj = new shObject ();
        obj->loadState (fd);
        mInventory->add (obj);
    }

    loadPointer (fd, (void **) &mLastLevel, "c.mLastLevel");

    loadMagic (fd, '<CRE');
}


void
shMonster::saveState (int fd)
{
    shCreature::saveState (fd);

    SWRITEBLOCK (fd, &mTame, &mPlaceHolder);
}


void
shMonster::loadState (int fd)
{
    shCreature::loadState (fd);
    
    SREADBLOCK (fd, &mTame, &mPlaceHolder);
}


void
shHero::saveState (int fd)
{
    int i, n;
    saveMagic (fd, 0x1337c0de);
    shCreature::saveState (fd);

    saveInt (fd, mXP);
    saveInt (fd, mScore);
    saveInt (fd, mBusy);
    saveInt (fd, mSkillPoints);
    
    n = mStoryFlags.count ();
    saveInt (fd, n);
    for (i = 0; i < n; i++) {
        shStoryFlag *sf = mStoryFlags.get (i);
        saveString (fd, sf->mName);
        saveInt (fd, sf->mValue);
    }
    
    n = mPets.count ();
    saveInt (fd, n);
    for (i = 0; i < n; i++) {
        savePointer (fd, mPets.get (i));
    }
}

void
shHero::loadState (int fd)
{
    int i, n;
    loadMagic (fd, 0x1337c0de);
    shCreature::loadState (fd);

    mXP = loadInt (fd);
    mScore = loadInt (fd);
    mBusy = loadInt (fd);
    mSkillPoints = loadInt (fd);

    n = loadInt (fd);
    for (i = 0; i < n; i++) {
        shStoryFlag *sf = new shStoryFlag ();
        loadString (fd, &sf->mName);
        sf->mValue = loadInt (fd);
        mStoryFlags.add (sf);
    }
    
    n = loadInt (fd);
    mPets.setCount (n);
    for (i = 0; i < n; i++) {
        loadPointer (fd, (void **) mPets.getPtr (i), "H.mPets");
    }

}


void
shMapLevel::saveState (int fd)
{
    int i, x, y;

    saveMagic (fd, 'MAP>');
    saveTarget (fd, this);

    SWRITEBLOCK (fd, &mDLevel, &mObjects);
    
    i = 0;
    for (x = 0; x < mColumns; x++) {
        for (y = 0; y < mRows; y++) {
            shObjectVector *v = mObjects[x][y];
            if (v) { 
                i += v->count ();
            }
        }
    }
    saveInt (fd, i);

    for (x = 0; x < mColumns; x++) {
        for (y = 0; y < mRows; y++) {
            shObjectVector *v = mObjects[x][y];
            if (v) {
                for (i = 0; i < v->count (); i++) {
                    shObject *obj = v->get (i);
                    obj->mX = x;
                    obj->mY = y;
                    obj->saveState (fd);
                }
            }
        }
    }

    i = mCrList.count ();
    saveInt (fd, i);
    for (i = 0; i < mCrList.count (); i++) {
        shCreature *c = mCrList.get (i);
        if (c->isHero ()) {
            saveInt (fd, 1);
        } else {
            saveInt (fd, 0);
        }
        c-> saveState (fd);
    }

    i = mFeatures.count ();
    saveInt (fd, i);
    for (i = 0; i < mFeatures.count (); i++) {
        SWRITE (fd, mFeatures.get (i), sizeof (shFeature));
    }
    saveMagic (fd, '<MAP');
}



void
shMapLevel::loadState (int fd)
{
    int i, n;
    shObject *obj;

    loadMagic (fd, 'MAP>');
    loadStruct (fd, this);

    SREADBLOCK (fd, &mDLevel, &mObjects);

    n = loadInt (fd);
    for (i = 0; i < n; i ++) {
        obj = new shObject ();
        obj->loadState (fd);
        putObject (obj, obj->mX, obj->mY);
    } 

    i = loadInt (fd);
    while (i--) {
        shCreature *c;
        int ishero = loadInt (fd);
        if (ishero) {
            c = &Hero;
        } else {
            c = new shMonster ();
        }
        c->loadState (fd);
        mCreatures[c->mX][c->mY] = c;
        mCrList.add (c);
        c->mLevel = this;
    }

    i = loadInt (fd);
    while (i--) {
        shFeature *f = new shFeature ();
        SREAD (fd, f, sizeof (shFeature));
        mFeatures.add (f);
        if (shFeature::kStairsUp == f->mType ||
            shFeature::kStairsDown == f->mType)
        {
            mExits.add (f);
        }
    }
    loadMagic (fd, '<MAP');
}


void
shObject::saveState (int fd)
{
    saveMagic (fd, 'OBJ>');
    saveTarget (fd, this);
    saveInt (fd, mIlk->mId);
    SWRITEBLOCK (fd, &mCount, &mOwner);
    if (kInventory == mLocation) {
        savePointer (fd, mOwner);
    }
    saveString (fd, mUserName);
    if (isA (&WreckIlk)) {
        saveInt (fd, mCorpseIlk->mId);
    } else {
        saveInt (fd, mImplantSite);
    }
    saveMagic (fd, '<OBJ');
}

void
shObject::loadState (int fd)
{
    loadMagic (fd, 'OBJ>');
    loadStruct (fd, this);
    mIlk = ObjectIlks.get (loadInt (fd));
    SREADBLOCK (fd, &mCount, &mOwner);
    if (kInventory == mLocation) {
        loadPointer (fd, (void **) &mOwner, "o.mOwner");
    }
    loadString (fd, &mUserName);
    if (isA (&WreckIlk)) {
        mCorpseIlk = MonsterIlks.get (loadInt (fd));
    } else {
        mImplantSite = (shImplantIlk::Site) loadInt (fd);
    }
    loadMagic (fd, '<OBJ');
}



void
shEvent::saveState (int fd)
{
    int i;
    saveInt (fd, mDay);
    saveInt (fd, mTime);
    saveInt (fd, mCancelled);
    saveInt (fd, mTargets.count ());
    for (i = 0; i < mTargets.count (); i++) {
        savePointer (fd, mTargets.get (i));
    }
#ifdef SH_DEBUG
    SWRITE (fd, mComment, sizeof (mComment));
#endif    
}




void
shEvent::loadState (int fd)
{
    int i, n;
    mDay = loadInt (fd);
    mTime = loadInt (fd);
    mCancelled = loadInt (fd);
    n = loadInt (fd);
    mTargets.setCount (n);
    for (i = 0; i < n; i++) {
        loadPointer (fd, (void **) mTargets.getPtr (i), "E.mTargets");
    }
#ifdef SH_DEBUG
    SREAD (fd, mComment, sizeof (mComment));
#endif    
}

#if 0
void
shGameOverEvent::saveState (int fd)
{
    saveInt (fd, kGameOver);
    shEvent::saveState (fd);
}


void
shHeroEquipEvent::saveState (int fd)
{
    saveInt (fd, kHeroEquip);
    savePointer (fd, mHero);
    savePointer (fd, mArmor);
    saveInt (fd, mDonOrDoff);
    shEvent::saveState (fd);
}


void
shHeroEquipEvent::loadState (int fd)
{
    loadPointer (fd, (void **) &mHero, "E.mHero");
    loadPointer (fd, (void **) &mArmor, "E.mArmor");
    mDonOrDoff = (shHeroEquipEvent::DonOrDoff) loadInt (fd);
    shEvent::loadState (fd);
}


void
shHeroReadyEvent::saveState (int fd)
{
    saveInt (fd, kHeroReady);
    savePointer (fd, (void **) mHero);
    shEvent::saveState (fd);
}


void
shHeroReadyEvent::loadState (int fd)
{
    loadPointer (fd, (void **) &mHero, "E.mHero");
    shEvent::loadState (fd);
}


void
shHeroUpkeepEvent::saveState (int fd)
{
    saveInt (fd, kHeroUpkeep);
    savePointer (fd, mHero);
    shEvent::saveState (fd);
}


void
shHeroUpkeepEvent::loadState (int fd)
{
    loadPointer (fd, (void **) &mHero, "E.mHero");
    shEvent::loadState (fd);
}



void
shMonsterReadyEvent::saveState (int fd)
{
    saveInt (fd, kMonsterReady);
    savePointer (fd, mMonster);
    shEvent::saveState (fd);
}


void
shMonsterReadyEvent::loadState (int fd)
{
    loadPointer (fd, (void **) &mMonster, "E.mMonster");
    shEvent::loadState (fd);
}


void
shMonsterSpawnEvent::saveState (int fd)
{
    saveInt (fd, kMonsterSpawn);
    shEvent::saveState (fd);
}

#endif


int
nameOK (const char *name)
{
    int fd;
    char filename[ZAPM_PATH_LENGTH];
    const char *p;

    if (strlen (name) > HERO_NAME_LENGTH || 
        strlen (name) < 1 ||
        isspace (name[0]))
    {
        return 0;
    }
    for (p = name; *p; p++) {
        if (!isprint (*p) || '/' == *p || '\\' == *p) 
            return 0;; 
    }
        

    snprintf (filename, sizeof(filename)-1, "%s/%s.tmp", DataDir, name);
    fd = open (filename, O_CREAT | O_WRONLY | O_EXCL | O_BINARY, 
#ifdef _WIN32
               S_IWRITE | S_IREAD);
#else
               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif
    if (-1 == fd) {
		I->debug ("nameOK() couldn't create file %s:%d", filename, errno);
        return 0;
    } 
    close (fd);
    unlink (filename);
    return 1;
}



/* returns 0 on success, -1 on failure*/
int
saveGame ()
{
    int fd;
    char savename[ZAPM_PATH_LENGTH];
    int success = -1;
    PtrList.reset ();
    UpdateList.reset ();

    snprintf (savename, sizeof(savename)-1, "%s/%s.sav", DataDir, Hero.mName);
    
retry:
    fd = open (savename, O_CREAT | O_WRONLY | O_EXCL | O_BINARY, 
#ifdef _WIN32
               S_IWRITE | S_IREAD);
#else
               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif

    if (-1 == fd) {
        if (EEXIST == errno) {
            I->p ("Strangely, a save file already exists.");
#ifndef SH_DEBUG
            if (GodMode)
#else 
            if (1)
#endif
            {
                if (!I->yn ("Delete it?")) {
                    return -1;
                }
            } else {
                I->p ("deleting it...");
            }
            if (-1 == unlink (savename)) {
                I->p ("Couldn't unlink save file %s: %s", 
                      savename, strerror (errno));
                return -1;
            }

            goto retry;
        } else {
            I->p ("Couldn't open the save file %s", savename);
            I->p ("%s", strerror (errno));
            return -1;
        }
    }

    I->p("Saving...");
    saveHeader (fd);

    saveInt (fd, Clock);
    saveInt (fd, MonsterClock);
    saveIlks (fd);
    saveDiscoveries (fd);
    saveOptions (fd);
    saveMaze (fd);

    success = 0; /* if we got this far, it worked. */

    close (fd);
    return success;
}


/* returns 0 on success, -1 on failure*/
int
loadGame (const char* name)
{
    int fd;
    char savename[ZAPM_PATH_LENGTH];
    int success = -1;

    if (!name) 
        name = getenv ("USER");

    snprintf (savename, sizeof(savename)-1, "%s/%s.sav", DataDir, name);
    
    fd = open (savename, O_RDONLY | O_BINARY, 0);

    if (-1 == fd) {
        return -1;
    }

    I->p ("Loading save game...");

    if (loadHeader (fd)) {
        close (fd);
        if (!I->yn ("Erase it and start a new game?")) {
            delete I;
            exitZapm (0);
        }
        unlink (savename);
        return -1;
    }

    Clock = loadInt (fd);
    MonsterClock = loadInt (fd);
    loadIlks (fd);
    initializeObjects ();
    loadDiscoveries (fd);
    loadOptions (fd);
    loadMaze (fd);

    if (UpdateList.count ()) {
        int i;
        int giveup = 1;

        if (GodMode) {
            if (!I->yn ("Some pointers uninitialized, abort?")) {
                giveup = 0;
            }
        } else {
            I->p ("Crap! This save file is corrupt!");
#ifdef SH_DEBUG
            if (I->yn ("Erase the save file?"))
#endif
                unlink (savename);
        }
        for (i = 0; i < UpdateList.count (); i++) {
            UpdatePair *p = UpdateList.get (i);
            I->debug ("Still need object %d", p->mId);
        }
        if (giveup) abort ();
    }

    Level = Hero.mLevel;

    success = 0; /* if we got this far, it worked. */

    close (fd);

    I->p ("Save game loaded successfully.");

    if (GodMode && !I->yn ("erase save file?")) {
    } else {
        unlink (savename);
    }
    return success;
}
