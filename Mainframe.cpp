#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Util.h"
#include "Map.h"
#include "Monster.h"
#include "Object.h"

#include "MapBuilder.h"

struct Junction
{
    int mVisited;
    int mWalls[4]; // North, East, West, South
    int mDeadEnd;

    void init ()
    {
        mVisited = 0;
        mDeadEnd = 0;
        mWalls[0] = mWalls[1] = mWalls[2] = mWalls[3] = 1; 
    }

    Junction () 
    { 
        init ();
    }
};

#define NODECOLS 8
#define NODEROWS 5
typedef Junction (*NodeGrid)[NODEROWS];


void
shMapLevel::buildMainframeHelper (void *user, int x, int y, int depth)
{
    int dirs[4];
    int i, n, r;

    NodeGrid nodes = (NodeGrid) user;

    nodes[x][y].mVisited++; //visit this node
    //nodes[x][y].mDepth = depth;
    
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

        int u = 0, v = 0;
        switch (i) {
        case 0:  u = x;      v = y - 1;  break;
        case 3:  u = x;      v = y + 1;  break;
        case 1:  u = x + 1;  v = y;      break;
        case 2:  u = x - 1;  v = y;      break;
        }
        //break the wall
        nodes[x][y].mWalls[i] = 0;
        nodes[u][v].mWalls[3-i] = 0;
        buildMainframeHelper (nodes, u, v, depth + 1);
    }
}


void
shMapLevel::buildMainframe ()
{
    int x, y, n;
    //int lighting = RNG (3) ? 0 : 1;
    Junction nodes[NODECOLS][NODEROWS];
    int widths[NODECOLS] = { 7, 7, 4, 6, 8, 7, 7 };
    int heights[NODEROWS] = { 3, 3, 4, 3 };
    int offx[NODECOLS];
    int offy[NODEROWS];

    mMapType = kMainframe;
    //mFlags |= shMapLevel::kNoTransport;

retry:
    mNumRooms = 0;
    for (x = 0; x < NODECOLS; x++) 
        for (y = 0; y < NODEROWS; y++) 
            nodes[x][y].init ();

    shuffle (widths+1, NODECOLS-3, sizeof(int));
    shuffle (heights+1, NODEROWS-3, sizeof(int));

    n = 3;
    for (x = 0; x < NODECOLS; x++) {
        offx[x] = n;
        n += widths[x] + 2;
    }
    n = 2;
    for (y = 0; y < NODEROWS; y++) {
        offy[y] = n;
        n += heights[y] + 1;
    }

    fillRect (0, 0, 63, 19, kVirtualWall);
    fillRect (1, 1, 62, 18, kVirtualFloor);
    flagRect (1, 1, 62, 18, shSquare::kHallway, 1);
    flagRect (1, 1, 62, 18, shSquare::kStairsOK, 0);

    // Build the maze.  First, remove some nodes to prevent long
    // hallways
    int blockedcols[7] = { 3, 4, 2, 5, 3,   0, 7 };
    int blockedrows[7] = { 0, 4, 1, 3, 2,   2, 2 };
    
    if (RNG(2)) 
        blockedcols[4] = 4;
    shuffle (blockedcols, 2, sizeof(int));
    shuffle (blockedcols+2, 2, sizeof(int));
    
    for (n = 0; n < 7; n++) {
        nodes[blockedcols[n]][blockedrows[n]].mVisited = 1;
    }
    
    buildMainframeHelper (nodes, RNG (NODECOLS), RNG (NODEROWS), 0);
    
    for (n = 0; n < 7; n++) {
        x = blockedcols[n];
        y = blockedrows[n];
        
        int dir; //0123 : NESW
        if (!y) 
            dir = 3;
        else if (y==NODEROWS-1) 
            dir = 0;
        else if (RNG (2))
            dir = 3;
        else
            dir = 0;
        
        int u, v;;
        switch (dir) {
        case 0:  u = x;      v = y - 1;  break;
        case 3:  u = x;      v = y + 1;  break;
        case 1:  u = x + 1;  v = y;      break;
        case 2:  u = x - 1;  v = y;      break;
        }
        
        nodes[x][y].mWalls[dir] = 0; 
        nodes[u][v].mWalls[3-dir] = 0;
    }
    
    
    for (x = 0; x < NODECOLS; x++) {
        for (y = 0; y < NODEROWS; y++) {
            buildMainframeJunction (nodes, x, y,
                                    offx[x]-2, offy[y]-1,
                                    widths, heights);
        }
    }
    
    
    for (x = 0; x < NODECOLS-1; x++) {
        for (y = 0; y < NODEROWS-1; y++) {
            buildMainframeRoom (nodes, x, y,
                                offx[x], offy[y],
                                widths[x], heights[y]);
        }
    }
    
    //do we like this map?
    for (x = 0; x < 64; x++) {
        n = 0;
        for (y = 0; y < 20; y++) {
            if (isFloor (x, y)) {
                if (++n > 12) {
                    I->debug ("retrying y");
                    goto retry;
                }
            } else {
                n = 0;
            }
        }
    }
    for (y = 0; y < 20; y++) {
        n = 0;
        for (x = 0; x < 64; x++) {
            if (isFloor (x, y)) {
                if (++n > 30) {
                    I->debug ("retrying x");
                    goto retry;
                }
            } else {
                n = 0;
            }
        }
    }


    //add some monsters
    int i;
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





int
shMapLevel::buildMainframeRoom (void *user, int col, int row, 
                                int x, int y,
                                int width, int height)
{
    //NodeGrid nodes = (NodeGrid) user;
    fillRect (x, y, x+width-1, y+height-1, kVirtualWall);
    setRoomId (x, y, x+width-1, y+height-1, mNumRooms);
    mNumRooms++;

    if (width > 4) {
        switch (RNG (width > 6 ? 7 : 3)) {
        case 0: 
            fillRect (x+2, y, x+width-3, y+height-2, kVirtualFloor); 
            if (8 == width && RNG (2) && isFloor (x+3, y-2)) {
                fillRect (x+3, y-1, x+4, y+height-3, kVirtualWall); 
            } else if (isFloor (x+width, y) && isFloor (x+width-1, y-1)
                       && RNG (2)) 
            {
                SETSQ (x+width-2, y-1, kVirtualWall);
                SETSQ (x+width-1, y-1, kVirtualWall);
                SETSQ (x+width-1, y+height-2, kVirtualFloor);
                SETSQ (x+width-2, y+height-2, kVirtualFloor);
                flagRect (x+2, y, x+width-3, y+height-2, 
                          shSquare::kHallway, 0); 
                flagRect (x+3, y, x+width-4, y+height-2, 
                          shSquare::kStairsOK, 1); 
            } else if (isFloor (x-1, y) && isFloor (x, y-1)
                       && RNG (3)) 
            {
                SETSQ (x, y-1, kVirtualWall);
                SETSQ (x+1, y-1, kVirtualWall);
                SETSQ (x, y+height-2, kVirtualFloor);
                SETSQ (x+1, y+height-2, kVirtualFloor);
                flagRect (x+2, y, x+width-3, y+height-2, 
                          shSquare::kHallway, 0); 
            } else {
                flagRect (x+2, y, x+width-3, y+height-2, 
                          shSquare::kHallway, 0); 
                flagRect (x+3, y, x+width-4, y+height-2, 
                          shSquare::kStairsOK, 1); 
            }
            if (6 == width) {
                flagRect (x+2, y+1, x+width-3, y+height-2, 
                          shSquare::kStairsOK, 1); 
            }
            break;
        case 1: 
            fillRect (x+2, y+1, x+width-3, y+height-1, kVirtualFloor); 
            if (8 == width && RNG (2)) {
                fillRect (x+3, y+2, x+4, y+height+1, kVirtualWall); 
            } else if (isFloor (x-1, y+height-1) && isFloor (x, y+height)
                       && RNG (2)) 
            {
                SETSQ (x, y+height, kVirtualWall);
                SETSQ (x+1, y+height, kVirtualWall);
                SETSQ (x, y+1, kVirtualFloor);
                SETSQ (x+1, y+1, kVirtualFloor);
                flagRect (x+2, y+1, x+width-3, y+height-1, 
                          shSquare::kHallway, 0); 
                flagRect (x+3, y+1, x+width-4, y+height-1, 
                          shSquare::kStairsOK, 1); 
            } else if (isFloor (x+width, y+height-1) && 
                       isFloor (x+width-1, y+height) && RNG (3)) 
            {
                SETSQ (x+width-2, y+height, kVirtualWall);
                SETSQ (x+width-1, y+height, kVirtualWall);
                SETSQ (x+width-2, y+1, kVirtualFloor);
                SETSQ (x+width-1, y+1, kVirtualFloor);
                flagRect (x+2, y+1, x+width-3, y+height-1, 
                          shSquare::kHallway, 0); 
                flagRect (x+3, y+1, x+width-4, y+height-1, 
                          shSquare::kStairsOK, 1); 
            } else {
                flagRect (x+2, y+1, x+width-3, y+height-1, 
                          shSquare::kHallway, 0); 
                flagRect (x+3, y+1, x+width-4, y+height-1, 
                          shSquare::kStairsOK, 1); 
            }
            if (6 == width) {
                flagRect (x+2, y+1, x+width-3, y+height-2, 
                          shSquare::kStairsOK, 1); 
            }
            break;
        case 2: 
            if (isFloor (x-1, y) && isFloor (x-1, y+height-1) && 
                !isFloor (x-3, y+1)) 
            {
                fillRect (x, y, x+width-3, y+height-1, kVirtualFloor); 
                fillRect (x-2, y+1, x+width-5, y+1, kVirtualWall);
                fillRect (x+width-6, y+height-2, x+width-5, y+height-2, 
                          kVirtualWall);
                if (isFloor (x, y+height)) {
                    fillRect (x+width-6, y+height-1, x+width-5, y+height-1, 
                              kVirtualWall);
                }
                if (isFloor (x, y-1)) {
                    fillRect (x+width-6, y, x+width-5, y, kVirtualWall);
                }
                flagRect (x, y, x+width-3, y+height-1, 
                          shSquare::kHallway, 0); 
              //flagRect (x, y, x+width-3, y+height-1, shSquare::kStairsOK, 1);
            } else if (isFloor (x+width, y) && isFloor (x+width, y+height-1) 
                       && isFloor (x+width-1, y+height) 
                       && isFloor (x+width-1, y-1))
            {
                int z = RNG (2) ? y : y+height-1;
                fillRect (x+2, y, x+width-1, y+height-1, kVirtualFloor); 
                fillRect (x+4, z, x+width+1, z, kVirtualWall);
                fillRect (x+4, y+1, x+5, y+height-2, kVirtualWall);
                if (isFloor (x+width-1, y+height)) {
                    fillRect (x+4, y+height-1, x+5, y+height-1, kVirtualWall);
                }
                if (isFloor (x+width-1, y-1)) {
                    fillRect (x+4, y, x+5, y, kVirtualWall);
                }
                flagRect (x, y, x+width-1, y+height-1, 
                          shSquare::kHallway, 0); 
            } else {
                fillRect (x+2, y+1, x+width-3, y+height-2, kStone); 
            }
            break;
        case 3: /* ] [ */
            fillRect (x+3, y, x+width-4, y+height-1, kVirtualFloor); 
            fillRect (x, y+1, x, y+height-2, kVirtualFloor); 
            fillRect (x+width-1, y+1, x+width-1, y+height-2, kVirtualFloor); 
            flagRect (x+3, y, x+width-4, y+height-1, shSquare::kHallway, 0);
            flagRect (x+3, y+1, x+width-4, y+height-2, shSquare::kStairsOK, 1);
            break;
        case 4: /* U */
            fillRect (x+2, y, x+width-3, y+height-2, kVirtualFloor); 
            flagRect (x+2, y, x+width-3, y+height-1, shSquare::kHallway, 0);
            flagRect (x+3, y+1, x+width-4, y+height-2, shSquare::kStairsOK, 1);
            if (isFloor (x+2, y-1)) {
                SETSQ (x+2, y, kVirtualWall);
                SETSQ (x+width-3, y, kVirtualWall);
            }
            break;
        case 5: /* upside down U */
            fillRect (x+2, y+1, x+width-3, y+height-1, kVirtualFloor); 
            flagRect (x+2, y+1, x+width-3, y+height-1, shSquare::kHallway, 0);
            flagRect (x+3, y+1, x+width-4, y+height-1, shSquare::kStairsOK, 1);
            break;
            SETSQ (x+2, y+height-1, kVirtualWall);
            SETSQ (x+width-3, y+height-1, kVirtualWall);
            break;
        case 6: 
            fillRect (x+2, y, x+width-3, y+height-1, kVirtualFloor); 
            flagRect (x+2, y, x+width-3, y+height-1, shSquare::kHallway, 0);
            flagRect (x+3, y+1, x+width-4, y+height-2, shSquare::kStairsOK, 1);
            if (isFloor (x+3, y-1)) { /* [ ] */
                SETSQ (x+2, y, kVirtualWall);
                SETSQ (x+width-3, y, kVirtualWall);
                SETSQ (x+2, y+height-1, kVirtualWall);
                SETSQ (x+width-3, y+height-1, kVirtualWall);
            } else { /* upside down U */
                fillRect (x+2, y, x+width-3, y, kVirtualWall);
            }
            break;
        }
    } else { /* narow room */
        switch (RNG (2)) {
        case 0: 
            fillRect (x,   y+1, x+width-3, y+height-2, kVirtualFloor); 
            flagRect (x,   y+1, x+width-3, y+height-2, shSquare::kHallway, 0);
            flagRect (x+1, y+1, x+width-3, y+height-2, shSquare::kStairsOK, 1);
            break;
        case 1: 
            fillRect (x+2, y+1, x+width-1, y+height-2, kVirtualFloor); 
            flagRect (x+2, y+1, x+width-1, y+height-2, shSquare::kHallway, 0);
            flagRect (x+2, y+1, x+width-2, y+height-2, shSquare::kStairsOK, 1);
            break;
        }
    }
    return 1;
}


int
shMapLevel::buildMainframeJunction (void *user, int col, int row, 
                                    int x, int y, int *widths, int *heights)
{
    NodeGrid nodes = (NodeGrid) user;

    Junction *jct = &nodes[col][row]; 


    if (jct->mWalls[3] && row < NODEROWS - 1) { // South?
        int h = heights[row];
        int j = RNG (2) ? 0 : h-1;
        SETSQ (x, y+j+1, kVirtualWall);
        SETSQ (x+1, y+j+1, kVirtualWall);
    }

    if (jct->mWalls[1] && col < NODECOLS -1) { // East?
        int w = widths[col];
        int j = RNG (2) ? 0 : w-2;
        SETSQ (x+j+2, y, kVirtualWall);
        SETSQ (x+j+3, y, kVirtualWall);
    }
    return 1;
}





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









