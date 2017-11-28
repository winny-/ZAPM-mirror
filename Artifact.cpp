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


void
initializeArtifacts ()
{
    ArtifactIlks.add ( 
        new shToolIlk ("The Continuum Transfunctioner", 
                       "Rubik's Cube", "Rubik's Cube",
                       kRed,
                       NULL, 1000000, kPlastic,
                       kUnique, 500, kTiny, 100, 100, 
                       25, NULL, 0) );

    ArtifactIlks.add ( 
        new shToolIlk ("Bizarro Orgasmatron", 
                       "orgasmatron", "orgasmatron",
                       kRed,
                       NULL, 1000000, kPlastic,
                       kUnique | kIndestructible, 500, kMedium, 100, 100, 
                       25, &useBizarroOrgasmatron, 0) );
}

