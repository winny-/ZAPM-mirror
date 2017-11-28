#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"

#include "MapBuilder.h"

/******

                                                                 
 ##########                                                      
 #........#        #######       ##############                  
 #.....<..#        #????(#########=++]!)#.....#                  
 #........#        #?????#.......#=?+()!#.....#                  
 ###'######        #..X..#.......#...X..#+#####                  
   #.#             ###'######+#######'###.+...#                  
   #.#  ########   #....................+.#...#                  
   #.####......#   #....................#.#######                
   #....+......#   #........#######.....#.#{.{.{#  ######        
   #.####......#####........#.....#.....#.#.....#  #....#        
   #.#  #......+............#.....#.....#.###'######....#        
   #.#  #......#####........#.....#.....................#        
   #.#  ########   #........###'###.....####'#######....#        
   #.#             #....................#.....#    #.>..#        
####+#####         #....................'.....#    #....#        
#........#         ###########...########.....#    ######        
#.....>..#         #.........#...#.(((((#######                  
#........#         #.........+...'X(((((#                        
##########         ######################                       

*********/

void
shMapLevel::buildTown ()
{
    int i;

    mMapType = kTown;

    layRoom (19, 6, 40, 19);       /* main room */
    layRoom (28, 9, 34, 13);       /* center room */
    layRoom (1, 1, 10, 5);         /* upper left */
    layRoom (8, 7, 15, 13);        /* guard room */
    layRoom (0, 15, 9, 19);        /* bottom left */
    layRoom (19, 2, 25, 6);        /* software store */
    layRoom (25, 3, 33, 6);        /* middle top */
    layRoom (33, 2, 40, 6);        /* */
    layRoom (19, 16, 29, 19);      /* bottom left empty shop */
    layRoom (33, 16, 40, 19);
    layRoom (40, 2, 46, 5);
    layRoom (42, 5, 46, 8);
    layRoom (42, 8, 48, 11);
    layRoom (51, 9, 56, 16);
    layRoom (40, 13, 46, 17);

    layCorridor (4, 5, 4, 15);
    layCorridor (5, 9, 8, 9);
    layCorridor (15, 11, 19, 11);

    layCorridor (40, 12, 51, 12);
    layCorridor (41, 6, 41, 11);


    addDoor (4, 5, 1, 0, 0, -1);
    addDoor (4, 15, 1, 0, 0, -1);
    addDoor (8, 9, 0, 0, 0, 0);
    addDoor (15, 11, 0, 0, 0, 0);
    addDoor (19, 11, 0, 0, 0, 0);

    addDoor (22, 6, 1, 0, 0, 0);
    addDoor (29, 6, 1, 0, 0, 0);
    addDoor (37, 6, 1, 0, 0, 0);

    addDoor (40, 7, 0, 0, 0, 0);
    addDoor (41, 5, 1, 0, 0, 0);
    addDoor (42, 6, 0, 0, 0, 0);
    addDoor (40, 15, 0, 0, 0, 0);

    addDoor (45, 11, 1, 0, 0, 0);
    addDoor (44, 13, 1, 0, 0, 0);

    addDoor (29, 18, 0, 0, 0, 0);
    addDoor (33, 18, 0, 0, 0, 0);
    addDoor (31, 13, 1, 0, 0, 0);
    

    mundaneRoom (1, 1, 10, 5);
    mundaneRoom (8, 7, 15, 13);
    mundaneRoom (0, 15, 9, 19);
    makeShop (19, 2, 25, 6, shRoom::kSoftwareStore);
    addVat (43, 9);
    addVat (45, 9);
    addVat (47, 9);
    makeShop (33, 2, 40, 6, shRoom::kGeneralStore);
    makeShop (33, 16, 40, 19, shRoom::kHardwareStore);
    makeHospital (19, 16, 29, 19);      /* bottom left empty shop */


    shMonster *guard = new shMonster (findAMonsterIlk ("guardbot"));
    putCreature (guard, 13, 11);
    guard->mGuard.mSX = 15;
    guard->mGuard.mSY = 0;
    guard->mGuard.mEX = 59;
    guard->mGuard.mEY = 19;
    guard->mGuard.mToll = 300;
    guard->mGuard.mChallengeIssued = 0;

    for (i = 0; i < 14; i++) {
        int x, y;
        shMonsterIlk *botilk;

        do {
            botilk = pickAMonsterIlk (RNG (mDLevel + 1));
        } while (!botilk || !IS_ROBOT (botilk->mType));
        shMonster *bot = new shMonster (botilk);
        findUnoccupiedSquare (&x, &y);
        putCreature (bot, x, y);
        //if (RNG (3)) { 
        //    bot->mStrategy = shMonster::kPeaceful;
        //}
    }
}
