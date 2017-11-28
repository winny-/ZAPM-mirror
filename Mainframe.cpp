#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"

#include "MapBuilder.h"

/******


################################################################
#           ##                                                 #
#  ### ###  ##   ####                                          #
#####   ######   ####                                          #
#  ### ###                                                     #
#           #### #### ####################                     #
# #### #### #### #  # ####            ####                     #
# #  # #  #      ####   ## ####  #### ##                       #
# #### #  # ####        ## ##      ## ##                       #
#      #  # #### ####      ##      ##                          #
# #### ####      #  #      ##      ##                          #
# #  #      #### #  #   ## ##      ## ##                       #
# #  # #### #  # ####   ## ####  #### ##                       #
# #### #### #  #      ####            ####                     #
#           #### #### ####################                     #
#  ### ###       #  # ####                                     #
##### < ######   #  # #### ## #### ## ####                     #
#  ### ###  ##   ####      ## #### ## ####                     #
#           ##        ####    ####    ####                     #
################################################################
                                          
                                          

################################################################
#           ##    ##############################    #### ## ## #
#  ### ###  ## ##                             ## ##       # #  #
#####   ###### ## ########################### ## ## #### ## ## #
#  ### ###     ## ##                       #   # #   ##  #   # #
#           ##    #### ################## ### ## ## #### ## ## #
# ## ### ## ########## ###            ### ### ##### ###   # ## #
# ##     ## ##      #   ## ####  #### ##   #         ### ## ## #
# ## ### ##    ## # # # ## ##      ## ## # # # # # # ##        #
# ## ### ######## # # #    ##      ##    # # # # # # ## ### ## #
# ##     ######## # # #    ##      ##    # # # # # # ## ### ## #
# ## ### ##    ## # # # ## ##      ## ## # # # # # # ##        #
#           ##      #   ## ####  #### ##   #         ### ## ## #
###### ############### ###            ### ### ##### ###   # ## #
#           ##    #### ################## ### ##### #### ## ## #
#  ### ###  ## ## #                            # #   ##  #   # #
##### < ###### ## ############################## ## #### ## ## #
#  ### ###     ##                             ##          # #  #
#           ##    #############  ############    ####### ## ## #
################################################################

*********/


void
shMapLevel::fillRect (int sx, int sy, int ex, int ey, shTerrainType t)
{
    int x;
    int y;

    for (x = sx; x <= ex; x++) {
        for (y = sy; y <= ey; y++) {
            SETSQ (x, y, t);
        }
    }
}


void
shMapLevel::buildMainframe ()
{
    int x, y, i;

    mMapType = kTown;

    layRoom (0, 0, 63, 19);

    fillRect (0, 0, 63, 19, kVirtualWall);

    for (x = 1; x < 55; x += 5) {
        fillRect (x, 1, x, 18, kVirtualFloor);
        for (y = RNG (1, 4); y < 18; y += RNG (3, 6)) {
            fillRect (x, y, x + 4, y, kVirtualFloor);
        }
    }
    fillRect (x, 1, x, 18, kVirtualFloor);



    fillRect (1, 14, 15, 18, kVirtualFloor);
    fillRect (3, 15, 5, 17, kVirtualWall);
    fillRect (7, 15, 9, 17, kVirtualWall);
    SETSQ (1, 16, kVirtualWall);
    SETSQ (2, 16, kVirtualWall);
    SETSQ (5, 16, kVirtualFloor);
    SETSQ (7, 16, kVirtualFloor);
    SETSQ (10, 16, kVirtualWall);
    SETSQ (11, 16, kVirtualWall);
    fillRect (12, 16, 13, 18, kVirtualWall);


/*
    fillRect (32, 5, 50, 14, kVirtualWall);
    fillRect (32, 7, 33, 12, kVirtualFloor);
    fillRect (34, 9, 35, 10, kVirtualFloor);
    fillRect (36, 6, 47, 13, kVirtualFloor);
    fillRect (37, 7, 46, 12, kVirtualWall);
    fillRect (39, 8, 44, 11, kVirtualFloor);
    SETSQ (41, 7, kVirtualFloor);
    SETSQ (42, 7, kVirtualFloor);
    SETSQ (41, 12, kVirtualFloor);
    SETSQ (42, 12, kVirtualFloor);

*/

    for (i = 0; i < 22; i++) {
        int x, y;
        shMonsterIlk *ilk;

        do {
            ilk = pickAMonsterIlk (i < 5 ? RNG (15, 20)
                                         : RNG (1, 15));
        } while (!ilk || !IS_PROGRAM (ilk->mType));
        shMonster *m = new shMonster (ilk);
        findUnoccupiedSquare (&x, &y);
        putCreature (m, x, y);
    }

}
