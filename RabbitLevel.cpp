#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"

#include "MapBuilder.h"

/******

The Killer Rabbit Level

*********/                         

void
shMapLevel::buildRabbitLevel ()
{
    mMapType = kRabbit;

    layRoom (15, 5, 45, 15);
    layRoom (45, 9, 47, 11);
    addDoor (45, 10, 0, 0, 1, 0, 1, 1, 1);


    shMonster *rabbit = new shMonster (findAMonsterIlk ("killer rabbit"));
    putCreature (rabbit, 40, 10);

    mFlags |= shMapLevel::kNoTransport;
    mFlags |= shMapLevel::kNoDig;
}
