#include <ctype.h>
#include "Global.h"
#include "Util.h"
#include "Object.h"
#include "Interface.h"
#include "Hero.h"

shVector <shObjectIlk *> ArtifactIlks;


static int
useBizarroOrgasmatron (shObject *obj)
{
    I->p ("Hot dog!  You finally got your paws on the Bizarro Orgasmatron!");
    I->p ("You use its awesome power to beam yourself off this two-bit space hulk.");
    I->p ("");

    Hero.earnScore (10000);
    Hero.die (kWonGame);
    return FULLTURN;
}


static int
useBizarreOrgasmatron (shObject *obj)
{
    I->p ("Hot dog!  You finally got your paws on the Bizarre Orgasmatron!");
    I->p ("You use its awesome power to beam yourself off this two-bit space hulk.");
    I->p ("");
    I->pause ();

    if (Hero.countEnergy () > 15) {
        Hero.loseEnergy (15);
        if (Hero.transport (-1, -1, 100)) {
            Hero.die (kKilled, "the power of the Bizarre Orgasmatron");
            return FULLTURN;
        }
    } 
    I->p ("Hrmm... that's bizarre...");
    obj->setIlkKnown ();
    return FULLTURN;
}


static int
useBizaaroOrgasmatron (shObject *obj)
{
    I->p ("You activate the Bizaaro Orgasmatron...");
    I->pause ();
    I->p ("An invisible choir sings, and you are bathed in radiance...");
    I->pause ();
    I->p ("The voice of eit_cyrus booms out:"); 
    I->setColor (kBrightGreen);
    I->p ("\"CoGNRATULATOINSS, n00B!!!\"");
    I->pause ();
    I->p ("\"In retrun  for thy sevrice, I grants thee teh gift of a SPELLCHEECKER!!!\"");
    I->p ("\"HAHAHAHAHOHOHOAHAHAHA!!! !!!!!1\");");
    I->setColor (kGray);
    
    obj->setIlkKnown ();
    return FULLTURN;
}


static int
useBizzaroOrgasmatron (shObject *obj)
{
    I->p ("This makes you feel great!");
    obj->setIlkKnown ();
    return FULLTURN;
}


static int
useBazaaroOrgasmatron (shObject *obj)
{
    I->p ("Nothing happens.");    
    obj->setIlkKnown ();
    return FULLTURN;
}


static int
useBazarroOrgasmatron (shObject *obj)
{
    char *capsname = strdup (Hero.mName);
    char *p;

    for (p = capsname; *p; p++)
        *p = toupper (*p);
    I->p ("THANK YOU %s!", capsname);
    I->p ("BUT OUR PRINCESS IS IN ANOTHER CASTLE!");
    free (capsname);
    obj->setIlkKnown ();
    return FULLTURN;
}


void
initializeArtifacts ()
{
    ArtifactIlks.add ( 
        new shToolIlk ("Continuum Transfunctioner", 
                       "Rubik's Cube", "Rubik's Cube",
                       kRed,
                       NULL, 1000000, kPlastic,
                       kUnique, 500, kTiny, 100, 100, 
                       25, NULL, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bizarro Orgasmatron", 
                       "Orgasmatron", "Bizarro Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBizarroOrgasmatron, 0) );


    ArtifactIlks.add ( 
        new shToolIlk ("Bizarre Orgasmatron", 
                       "Orgasmatron", "Bizarre Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBizarreOrgasmatron, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bizaaro Orgasmatron", 
                       "Orgasmatron", "Bizaaro Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBizaaroOrgasmatron, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bizzaro Orgasmatron", 
                       "Orgasmatron", "Bizzaro Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBizzaroOrgasmatron, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bazaaro Orgasmatron", 
                       "Orgasmatron", "Bazaaro Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBazaaroOrgasmatron, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bazarro Orgasmatron", 
                       "Orgasmatron", "Bazarro Orgasmatron",
                       kBrightMagenta,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible | kIdentified, 500, kMedium, 100, 100, 
                       25, &useBazarroOrgasmatron, 0) );


    ArtifactIlks.add (
        new shImplantIlk ("Eye of the BOFH", 
                          "detached retina", "detached retina",
                          kRed, kFleshy, 
                          kUnique | kIndestructible, 
                          shImplantIlk::kAnyEye,
                          kNightVision, 0, 1000, 0) );

}

