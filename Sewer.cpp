#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"

#include "MapBuilder.h"

#define ptox(_p) (_p % mColumns)
#define ptoy(_p) (_p / mColumns)
#define xytop(_x, _y) (_y * mColumns + _x)



struct SewerRoom {
    int mBulbous;
    int mVisited;
    int mWalls[4]; // North, East, West, South
    int mDepth;
    int mDeadEnd;

    SewerRoom () 
    { 
        mBulbous = 0;
        mVisited = 0;
        mDeadEnd = 0;
        mWalls[0] = mWalls[1] = mWalls[2] = mWalls[3] = 1; 
    }
};


#define NODECOLS 10
#define NODEROWS 4
typedef SewerRoom (*NodeGrid)[NODEROWS];


static int
distanceToNode (NodeGrid nodes, int x, int y, int x2, int y2)
{
    int val;
    int best = 9999;

    if (x == x2 && y == y2) 
        return 0;

    nodes[x][y].mVisited++; //visit this node

    if (y > 0 && !nodes[x][y-1].mVisited) {
        val = distanceToNode (nodes, x, y-1, x2, y2);
        if (val < best) best = val;
    }
    if (y < NODEROWS-1 && !nodes[x][y+1].mVisited) {
        val = distanceToNode (nodes, x, y+1, x2, y2);
        if (val < best) best = val;
    }
    if (x < NODECOLS-1 && !nodes[x+1][y].mVisited) {
        val = distanceToNode (nodes, x+1, y, x2, y2);
        if (val < best) best = val;
    }
    if (x > 0 && !nodes[x-1][y].mVisited) {
        val = distanceToNode (nodes, x-1, y, x2, y2);
        if (val < best) best = val;
    }

    nodes[x][y].mVisited--; //

    return best + 1;
}



void
shMapLevel::buildSewerHelper (void *user, int x, int y, int depth)
{
    int dirs[4];
    int i, n, r;

    NodeGrid nodes = (NodeGrid) user;

    nodes[x][y].mVisited++; //visit this node
    nodes[x][y].mDepth = depth;
    
    while (1) {
        /* pick a random neighbor and visit it */
        dirs[0] = (y > 0 &&          !nodes[x][y-1].mVisited) ? 1 : 0;
        dirs[3] = (y < NODEROWS-1 && !nodes[x][y+1].mVisited) ? 1 : 0;
        dirs[1] = (x < NODECOLS-1 && !nodes[x+1][y].mVisited) ? 1 : 0;
        dirs[2] = (x > 0 &&          !nodes[x-1][y].mVisited) ? 1 : 0;
        n = dirs[0] + dirs[1] + dirs[2] + dirs[3];
        if (!n) { /* no (more) unvisited neighbors, backtrack */
            n = 0;
            for (i = 0; i < 4; i++)
                n += nodes[x][y].mWalls[i];
            if (3 == n) 
                nodes[x][y].mDeadEnd = 1;
            return;
        }
        r = RNG (n);
        i = -1;

        do {
            while (!dirs[++i]) ;
        } while (r--);

        int u = -1, v = -1;
        switch (i) {
        case 0:  u = x;      v = y - 1;  break;
        case 3:  u = x;      v = y + 1;  break;
        case 1:  u = x + 1;  v = y;      break;
        case 2:  u = x - 1;  v = y;      break;
        }
        //break the wall
        nodes[x][y].mWalls[i] = 0;
        nodes[u][v].mWalls[3-i] = 0;
        buildSewerHelper (nodes, u, v, depth + 1);
    }
}


int
shMapLevel::buildSewer ()
{
    int x, y, i, n;
    int lighting = RNG (2) ? -1 : 1;
    SewerRoom nodes[NODECOLS][NODEROWS];

    mMapType = kSewer;

    x = RNG (NODECOLS);
    y = RNG (NODEROWS);

    //first pass
    buildSewerHelper (nodes, x, y, 0);

    //second pass, make more tightly connected
    for (n = 10; n; n--) {
        if (RNG(3)) {
            x = RNG (NODECOLS-1);
            y = RNG (NODEROWS);
            nodes[x][y].mWalls[1] = 0;
            nodes[x+1][y].mWalls[2] = 0;
        } else {
            x = RNG (NODECOLS);
            y = RNG (NODEROWS-1);
            nodes[x][y].mWalls[3] = 0;
            nodes[x][y+1].mWalls[0] = 0;
        }
    }

    for (x = 0; x < NODECOLS; x++) {
        for (y = 0; y < NODEROWS; y++) {
            if (nodes[x][y].mDeadEnd || !RNG(7)) {
                nodes[x][y].mBulbous = 1;
            }
        }
    }

    // force 2 bulbous rooms
    x = RNG (NODECOLS/3); y = RNG (NODEROWS);
    nodes[x][y].mBulbous = 1;
    x = NODECOLS - 1 - RNG (NODECOLS/3); y = RNG (NODEROWS);
    nodes[x][y].mBulbous = 1;

    // don't have adjacent bulbous rooms (looks bad)
    for (x = 0; x < NODECOLS-1; x++) {
        for (y = 0; y < NODEROWS; y++) {
            if (nodes[x][y].mBulbous && nodes[x+1][y].mBulbous &&
                nodes[x][y].mWalls[1]) 
            {
                if (RNG(2)) 
                    nodes[x][y].mBulbous = 0;
                else 
                    nodes[x+1][y].mBulbous = 0;
            }
        }
    }

    for (x = 0; x < NODECOLS; x++) {
        for (y = 0; y < NODEROWS-1; y++) {
            if (nodes[x][y].mBulbous && nodes[x][y+1].mBulbous &&
                nodes[x][y].mWalls[3]) 
            {
                if (RNG(2)) 
                    nodes[x][y].mBulbous = 0;
                else 
                    nodes[x][y+1].mBulbous = 0;
            }
        }
    }

    for (x = 0; x < NODECOLS; x++) {
        for (y = 0; y < NODEROWS; y++) {
            buildSewerRoom (nodes, x, y);
        }
    }

    for (n = NDX(2,4); n; n--) {
        floodMuck (RNG(NODECOLS)*6+5, RNG(NODEROWS)*4+2, kSewage, NDX(8,20));
    }

    for (i = NDX (2, 3); i; --i) {
        findUnoccupiedSquare (&x, &y);
        switch (RNG (4)) {
        case 0:
            addTrap (x, y, shFeature::kPit); break;
        case 1:
            addTrap (x, y, shFeature::kHole); break;
        case 2:
            addTrap (x, y, shFeature::kTrapDoor); break;
        case 3:
            addTrap (x, y, shFeature::kRadTrap); break;
        }
    }

    for (i = 0; i < 8; i++) {
        findUnoccupiedSquare (&x, &y);
        putObject (generateObject (mDLevel), x, y);
    }

    for (x = 0; x < mColumns; x++)
        for (y = 0; y < mRows; y++) 
            setLit (x, y, lighting, lighting, lighting, lighting);



    return 1;
}


#define ptox(_p) (_p % mColumns)
#define ptoy(_p) (_p / mColumns)
#define xytop(_x, _y) (_y * mColumns + _x)

//flood fills some muck starting at x,y
int
shMapLevel::floodMuck (int sx, int sy, shTerrainType type, int amount)
{
    int x, y, i;
    shDirection dirs[4] = { kNorth, kSouth, kEast, kWest };
    shDirection dir;
    int n = 0;
    shVector <int> points;

    points.add (xytop (sx, sy));
    
    for (n = 0; n < amount;) {
        int p;
        if (0 == points.count ()) {
            return n;
        }
        p = points.removeByIndex (RNG (points.count ()));
        x = ptox (p);
        y = ptoy (p);

        if (!isFloor (x, y) || kSewage == getSquare (x, y) ->mTerr) {
            continue;
        } 
        n++;

        if (RNG(2))
            SETSQ (x, y, kSewage);

        for (i = 0; i < 4; i++) {
            dir = dirs[i];
            int x2 = x;
            int y2 = y;

            
            if (moveForward (dir, &x2, &y2) &&
                RNG (20) >= points.count ())
            {
                points.add (xytop (x2, y2));
            }
        }
    }
    return n;
}


//
// 6x5 bulbous
//          #  #    ####  
//         #    #  #    # 
//              #       # 
//         #    #  #    # 
//          ####    ####  
// 6x5 straight
//          #  #   
//         ##  #   #####
//             #     ..#
//         ### #   #####
//         


int
shMapLevel::buildSewerRoom (void *user, int col, int row)
{
    NodeGrid nodes = (NodeGrid) user;
    int x = col * 6 + 2;
    int y = row * 5;
    int i, j;

    SewerRoom *room = &nodes[col][row]; 

    for (i = x + 1; i <= x + 4; i++)
        for (j = y + 1; j <= y + 3; j++)
            SETSQ (i, j, kSewerFloor);
    
    if (room->mBulbous) {
        flagRect (x+2, y+1, x+3, y+3, shSquare::kStairsOK, 1);
        if (room->mWalls[0]) {  // NORTH WALL?
            SETSQ (x+1, y, kSewerWall);
            SETSQ (x+2, y, kSewerWall);
            SETSQ (x+3, y, kSewerWall);
            SETSQ (x+4, y, kSewerWall);
        } else {
            SETSQ (x+1, y, kSewerWall);
            SETSQ (x+2, y, kSewerFloor);
            SETSQ (x+3, y, kSewerFloor);
            SETSQ (x+4, y, kSewerWall);
        }
        if (room->mWalls[3]) {  // SOUTH WALL?
            SETSQ (x+1, y+4, kSewerWall);
            SETSQ (x+2, y+4, kSewerWall);
            SETSQ (x+3, y+4, kSewerWall);
            SETSQ (x+4, y+4, kSewerWall);
        } else {
            SETSQ (x+1, y+4, kSewerWall);
            SETSQ (x+2, y+4, kSewerFloor);
            SETSQ (x+3, y+4, kSewerFloor);
            SETSQ (x+4, y+4, kSewerWall);
        }
        if (room->mWalls[1]) {  // EAST WALL?
            SETSQ (x+5, y+1, kSewerWall);
            SETSQ (x+5, y+2, kSewerWall);
            SETSQ (x+5, y+3, kSewerWall);
        } else {
            SETSQ (x+5, y+1, kSewerWall);
            SETSQ (x+5, y+2, kSewerFloor);
            SETSQ (x+5, y+3, kSewerWall);
        }
        if (room->mWalls[2]) {  // WEST WALL?
            SETSQ (x, y+1, kSewerWall);
            SETSQ (x, y+2, kSewerWall);
            SETSQ (x, y+3, kSewerWall);
        } else {
            SETSQ (x, y+1, kSewerWall);
            SETSQ (x, y+2, kSewerFloor);
            SETSQ (x, y+3, kSewerWall);
        }
        for (i = x + 0; i <= x + 5; i++) {
            for (j = y + 0; j <= y + 4; j++) {
                if (kSewerFloor == GETSQ (i, j)) {
                    SETROOM (i, j, mNumRooms);
                }
            }
        }        

        mRooms[mNumRooms].mType = shRoom::kSewer;
        mNumRooms++;
    } else { //NOT BULBOUS
        if (room->mWalls[0]) {  // NORTH WALL?
            SETSQ (x+2, y+1, kSewerWall);
            SETSQ (x+3, y+1, kSewerWall);
            if (room->mWalls[1]) 
                SETSQ (x+4, y+1, kSewerWall);
            else 
                SETSQ (x+4, y+1, kSewerWall);
            if (room->mWalls[2]) 
                SETSQ (x+1, y+1, kSewerWall);
            else 
                SETSQ (x+1, y+1, kSewerWall);
        } else {
            SETSQ (x+1, y, kSewerWall);
            SETSQ (x+2, y, kSewerFloor);
            SETSQ (x+3, y, kSewerFloor);
            SETSQ (x+4, y, kSewerWall);
            if (room->mWalls[1]) 
                SETSQ (x+4, y+1, kSewerWall);
            else 
                SETSQ (x+4, y+1, kSewerWall);  //SW corn
            if (room->mWalls[2]) 
                SETSQ (x+1, y+1, kSewerWall);  //SE corn
            else 
                SETSQ (x+1, y+1, kSewerWall);
        }
        if (room->mWalls[3]) {  // SOUTH WALL?
            SETSQ (x+2, y+3, kSewerWall);
            SETSQ (x+3, y+3, kSewerWall);
            if (room->mWalls[1]) 
                SETSQ (x+4, y+3, kSewerWall);  
            else 
                SETSQ (x+4, y+3, kSewerWall);
            if (room->mWalls[2]) 
                SETSQ (x+1, y+3, kSewerWall); //SW corner
            else 
                SETSQ (x+1, y+3, kSewerWall);
        } else {
            SETSQ (x+1, y+4, kSewerWall);
            SETSQ (x+2, y+4, kSewerFloor);
            SETSQ (x+3, y+4, kSewerFloor);
            SETSQ (x+4, y+4, kSewerWall);
            if (room->mWalls[1]) 
                SETSQ (x+4, y+3, kSewerWall);
            else 
                SETSQ (x+4, y+3, kSewerWall);
            if (room->mWalls[2]) 
                SETSQ (x+1, y+3, kSewerWall);
            else 
                SETSQ (x+1, y+3, kSewerWall);
        }
        if (room->mWalls[1]) {  // EAST WALL?
            SETSQ (x+4, y+2, kSewerWall);
        } else {
            SETSQ (x+5, y+1, kSewerWall);
            SETSQ (x+5, y+2, kSewerFloor);
            SETSQ (x+5, y+3, kSewerWall);
        }
        if (room->mWalls[2]) {  // WEST WALL?
            SETSQ (x+1, y+2, kSewerWall);
        } else {
            SETSQ (x, y+1, kSewerWall);
            SETSQ (x, y+2, kSewerFloor);
            SETSQ (x, y+3, kSewerWall);
        }

        for (i = x + 0; i <= x + 5; i++) {
            for (j = y + 0; j <= y + 4; j++) {
                if (kSewerFloor == GETSQ (i, j)) {
                    SETSQFLAG (i, j, kHallway);
                }
            }
        }        
    }            


    return 1;
}


