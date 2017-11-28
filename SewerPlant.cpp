#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"
#include "Hero.h"
#include "MapBuilder.h"

/******

The Waste Treatment Plant Level

guaranteed items: radiation suit, keycard


#############################################
# 
#
#           1         2         3     .   4         5         6
  0123456789012345678901234567890123456789012345678901234567890123

0   ####   ####        ####   ####        ########################
1  #....# #....# ######~~~~# #~~~~######  #======#...............#
2  #.<..# #....# #....+~~~~# #~~~~+....#  #~~~~~~#.               
3  #....# #....# #..###~~~~# #~~~~###..####~~~~~~#.               
4   #..#   #..#  #..#  ####   ####  #.....+~~~~~~+              
5   #..#   #..#  #..#               #..####~~~~~~#.            
6   #..#   #..#  #..#  ####   ####  #..#  #~~~~~~#.           ~~~
7   #..#####..#  #..###....# #~~~~###..#  #======#............~~~#
8   #.........#  #....+....# #~~~~+....#  ########............~~~#
9   ########..#  #..###....# #~~~~###..#         #............~~~#
0          #..#  #..#  ####   ####  #..###########...........#~~~#
1    ####  #..#  #..#               #............+...........#~~~#
2   #~~~~# #..####..#################..#############+#####+###~~~#
3   #~~~~# #...........................#         #.....#.....#~~~#
4   #~~~~# #############################         #..[..#.{.{.#~~~#
5    #~~#                                        #.....#.....#~~~#
6 ####~~######################################################~~~#
7 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
8 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
9 ################################################################
 
  0123456789012345678901234567890123456789012345678901234567890123

*********/                         


void
shMapLevel::buildSewerPlant ()
{
    int x, y;
    int i;

    mMapType = kSewerPlant;

    fillRect (2, 0, 5, 0, kSewerWall);
    fillRect (9, 0, 12, 0, kSewerWall);
    fillRect (21, 0, 24, 0, kSewerWall);
    fillRect (28, 0, 31, 0, kSewerWall);
    fillRect (1, 1, 6, 3, kSewerWall);
    fillRect (8, 1, 13, 3, kSewerWall);
    fillRect (2, 12, 7, 14, kSewerWall);
    fillRect (3, 11, 6, 15, kSewerWall);
    fillRect (2, 4, 5, 9, kSewerWall);
    fillRect (9, 1, 12, 12, kSewerWall);
    fillRect (6, 7, 8, 9, kSewerWall);
    fillRect (9, 12, 37, 14, kSewerWall);
    fillRect (15, 1, 18, 12, kSewerWall);
    fillRect (18, 1, 25, 3, kSewerWall);
    fillRect (21, 4, 24, 4, kSewerWall);
    fillRect (19, 7, 25, 9, kSewerWall);
    fillRect (21, 6, 24, 10, kSewerWall);
    fillRect (27, 1, 33, 3, kSewerWall);
    fillRect (34, 1, 37, 12, kSewerWall);
    fillRect (28, 4, 32, 4, kSewerWall);
    fillRect (27, 7, 33, 9, kSewerWall);
    fillRect (28, 6, 31, 10, kSewerWall);
    fillRect (37, 3, 39, 5, kSewerWall);
    fillRect (37, 10, 46, 12, kSewerWall);
    fillRect (40, 0, 47, 8, kSewerWall);
    fillRect (47, 0, 63, 15, kSewerWall);
    fillRect (0, 16, 63, 19, kSewerWall);

    fillRect (2, 1, 5, 3, kSewerFloor);
    fillRect (3, 4, 4, 7, kSewerFloor);
    fillRect (3, 8, 9, 8, kSewerFloor);
    fillRect (3, 12, 6, 13, kSewerFloor);
    fillRect (4, 15, 5, 16, kSewerFloor);
    fillRect (3, 12, 6, 14, kSewerFloor);
    fillRect (4, 15, 5, 16, kSewerFloor);
    fillRect (9, 1, 12, 3, kSewerFloor);
    fillRect (10, 4, 11, 12, kSewerFloor);
    fillRect (16, 2, 20, 2, kSewerFloor);
    fillRect (10, 13, 34, 13, kSewerFloor);
    fillRect (16, 3, 17, 12, kSewerFloor);
    fillRect (21, 1, 24, 3, kSewerFloor);
    fillRect (18, 8, 20, 8, kSewerFloor);
    fillRect (21, 7, 24, 9, kSewerFloor);
    fillRect (28, 1, 31, 3, kSewerFloor);
    fillRect (32, 2, 34, 2, kSewerFloor);
    fillRect (28, 7, 31, 9, kSewerFloor);
    fillRect (32, 8, 34, 8, kSewerFloor);
    fillRect (35, 2, 36, 13, kSewerFloor);
    fillRect (37, 4, 40, 4, kSewerFloor);
    fillRect (37, 11, 47, 11, kSewerFloor);
    SETSQ (40, 4, kSewerFloor);
    SETSQ (47, 4, kSewerFloor);
    SETSQ (50, 12, kSewerFloor);
    SETSQ (56, 12, kSewerFloor);
    fillRect (41, 1, 46, 7, kSewerFloor);
    fillRect (48, 1, 62, 11, kSewerFloor);
    fillRect (49, 2, 63, 6, /* kVoid*/ kSewage);
    fillRect (48, 13, 52, 15, kSewerFloor);
    fillRect (54, 13, 58, 15, kSewerFloor);
    fillRect (60, 7, 62, 16, kSewerFloor);
    fillRect (0, 17, 62, 18, kSewerFloor);

    fillRect (59, 10, 59, 11, kSewerWall);

    fillRect (0, 17, 62, 18, kSewage);
    fillRect (60, 6, 62, 16, kSewage);
    fillRect (21, 1, 24, 3, kSewage);
    fillRect (28, 1, 31, 3, kSewage);
    fillRect (28, 7, 31, 9, kSewage);
    fillRect (3, 12, 6, 14, kSewage);
    fillRect (4, 15, 5, 16, kSewage);
    
    addDoor (20, 2, 0, 0, 0, 0);
    addDoor (32, 2, 0, 0, 0, 0);
    addDoor (20, 8, 0, 0, 0, 0);
    addDoor (32, 8, 0, 0, 0, 0);
    addDoor (40, 4, 0, 0, 1, 0, 0, 1);
    addDoor (47, 4, 0, 0, 1, 0, 0, 1);
    addDoor (47, 11, 0, 0, 0, 0);
    addDoor (50, 12, 1, 0, 1, 0);
    addDoor (56, 12, 1, 0, 0, 0);

    flagRect (0, 17, 63, 19, shSquare::kDarkNW , 1);
    flagRect (0, 17, 63, 19, shSquare::kDarkNE , 1);
    flagRect (0, 16, 59, 19, shSquare::kDarkSW , 1);
    flagRect (0, 16, 59, 19, shSquare::kDarkSE , 1);

    flagRect (2, 11, 7, 17, shSquare::kDark, 1);
    flagRect (59, 12, 59, 19, shSquare::kDarkNE, 1);
    flagRect (59, 12, 59, 19, shSquare::kDarkSE, 1);
    flagRect (60, 12, 63, 19, shSquare::kDark, 1);

    mNumRooms++;
    for (x = 40; x <= 47; x++) 
        for (y = 0; y <= 8; y++)  
            SETROOM (x, y, mNumRooms);

    for (i = 0; i < 13; i++) 
        putObject (generateObject (mDLevel), RNG (41, 46), RNG (1, 7));

    if (RNG (2)) {
        putObject (createObject ("debugged radiation suit", 0), 50, 14);
        putObject (createObject ("buggy ordinary jumpsuit", 0), 56, 14);
    } else {
        putObject (createObject ("buggy ordinary jumpsuit", 0), 50, 14);
        putObject (createObject ("debugged radiation suit", 0), 56, 14);
    }

    putObject (createObject ("debugged fusion power plant", 0),
               3 + RNG (4), 12 + RNG (3));

    fillRect (41, 1, 46, 7, kSewage);
    makeGarbageCompactor (40, 0, 47, 8);
    flagRect (0, 0, 63, 19, shSquare::kNoLanding, 1);
    flagRect (41, 1, 46, 7, shSquare::kNoLanding, 0);

    //flagRect (41, 2, 46, 7, shSquare::kDarkNW , 1);
    //flagRect (40, 2, 45, 7, shSquare::kDarkNE , 1);
    //flagRect (41, 1, 46, 6, shSquare::kDarkSW , 1);
    //flagRect (40, 1, 45, 6, shSquare::kDarkSE , 1);


//    flagRect (2, 11, 7, 15, shSquare::kDark, 1);
//    flagRect (60, 10, 63, 16, shSquare::kDark, 1);


}



void
shMapLevel::flagRect (int sx, int sy, int ex, int ey, 
                      shSquare::shSquareFlags flag, int value)
{
    int x, y;

    for (x = sx; x <= ex; x++) {
        for (y = sy; y <= ey; y++) {
            if (value) {
                mSquares[x][y].mFlags |= flag;
            } else {
                mSquares[x][y].mFlags &= ~flag;
            }
        }
    }
}



void
shHero::enterCompactor ()
{
    if (Level->mTimeOuts.mCompactor < Clock - FULLTURN) {
        Level->mTimeOuts.mCompactor = Clock + FULLTURN * 3;
        Level->magDoors (1);
    }
}


void
shHero::leaveCompactor ()
{
}
