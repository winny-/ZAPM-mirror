/* code to determine if there is a line of sight between two points 
   on the map.

  .....#....   In this diagram, A should be able to see D, but not B (behind 
  A....#B...   obstacle X) - but what about C?  The rules we'll use:
  .........C    - obstacles fill their entire square.
  .........D    - vision is calculated from the center of the observer's square
                - an object is visible iff any part of a sub square (0.2,0.2;
                  0.8,0.8) can be intersected by a ray cast from the observer

  .........  In this diagram, B is hiding in an alcove, in a position where
  ..A......  A might be able to see B and not vice-versa.  Can we implement
  ######B##  some sort of peeking system?  
  .....###.  



  Well, I went with a beam-casting algorithm posted by Isaac Kuo on 
  rec.games.roguelike.development - my implementation's got some bugs
  in the corner cases, but it'll do for now.

*/


#include "Global.h"
#include "Map.h"
#include "Hero.h"


/* computes all the squares visible by the Hero */
void
shMapLevel::computeVisibility ()
{
    int slope;     /* slope in v coordinate */
    int u, v;
    int x, y;
    int corner;
    int min, max;     /* width of beam along v axis */

    //I->debug ("**** computing vis ****");

    for (x = 0; x < MAPMAXCOLUMNS; x++) {
        for (y = 0; y < MAPMAXROWS; y++) {
            mVisibility[x][y] = 0;
        }
    }

    /* orthogonal first */

    mVisibility[Hero.mX][Hero.mY] = 1;

    for (x = Hero.mX;
         mVisibility[x][Hero.mY] = 1, !isOcclusive (x, Hero.mY); 
         x++);
    for (x = Hero.mX; 
         mVisibility[x][Hero.mY] = 1, !isOcclusive (x, Hero.mY);
         x--);  
    for (y = Hero.mY; 
         mVisibility[Hero.mX][y] = 1, !isOcclusive (Hero.mX, y); 
         y++);
    for (y = Hero.mY;
         mVisibility[Hero.mX][y] = 1, !isOcclusive (Hero.mX, y); 
         y--);

    /* now each quadrant */

    for (slope = 1; slope < 32; slope++) {
        for (u = 1, v = slope, min = 0, max = 31; 
             u < 50 && min <= max;
             u++, v += slope)
        {
            y = v / 32;
            x = u - y;
            x += Hero.mX;
            y += Hero.mY;

            corner = 32 - v % 32;

            if (min < corner) {
                mVisibility[x][y] = 1;
                if (isOcclusive (x,y)) { 
                    min = corner;
                }
            }
            if (max > corner) {
                mVisibility[x-1][y+1] = 1;
                if (isOcclusive (x - 1, y + 1)) {
                    max = corner;
                }
            }
        }
    }
    for (slope = 1; slope < 32; slope++) {
        for (u = 1, v = slope, min = 0, max = 31; 
             u < 50 && min <= max;
             u++, v += slope)
        {
            y = v / 32;
            x = u - y;
            x = Hero.mX - x;
            y += Hero.mY;

            corner = 32 - v % 32;

            if (min < corner) {
                mVisibility[x][y] = 1;
                if (isOcclusive (x,y)) { 
                    min = corner;
                }
            }
            if (max > corner) {
                mVisibility[x+1][y+1] = 1;
                if (isOcclusive (x + 1, y + 1)) {
                    max = corner;
                }
            }
        }
    }

    for (slope = 1; slope < 32; slope++) {
        for (u = 1, v = slope, min = 0, max = 31; 
             u < 50 && min <= max;
             u++, v += slope)
        {
            y = v / 32;
            x = u - y;
            x = Hero.mX - x;
            y = Hero.mY - y;

            corner = 32 - v % 32;

            if (min < corner) {
                mVisibility[x][y] = 1;
                if (isOcclusive (x,y)) { 
                    min = corner;
                }
            }
            if (max > corner) {
                mVisibility[x+1][y-1] = 1;
                if (isOcclusive (x + 1, y - 1)) {
                    max = corner;
                }
            }
        }
    }

    for (slope = 1; slope < 32; slope++) {
        for (u = 1, v = slope, min = 0, max = 31; 
             u < 50 && min <= max;
             u++, v += slope)
        {
            y = v / 32;
            x = u - y;
            x = x + Hero.mX;
            y = Hero.mY - y;

            corner = 32 - v % 32;

            if (min < corner) {
                mVisibility[x][y] = 1;
                if (isOcclusive (x,y)) { 
                    min = corner;
                }
            }
            if (max > corner) {
                mVisibility[x-1][y-1] = 1;
                if (isOcclusive (x - 1, y - 1)) {
                    max = corner;
                }
            }
        }
    }

    Hero.spotStuff ();
#if 0
    if (Hero.hasXRayVision ()) {
        int radius = 4;// + Hero.mCLevel / 4;
        for (x = Hero.mX - radius; x <= Hero.mX + radius; x++) {
            for (y = Hero.mY - radius; y <= Hero.mY + radius; y++) {
                if (Level->isInBounds (x, y) 
                    && distance (&Hero, x, y) < 5 * radius)
                {
                    mVisibility[x][y] = 1;
                }
            }
        }
    }
#endif
}


/* returns 100 if clear line of sight between the points, 0 o/w */
int
shMapLevel::existsLOS (int x1, int y1, int x2, int y2)
{
    int n;
    int i;
    double x, y;
    double dx, dy;

    n = abs (x2 - x1);
    i = abs (y2 - y1);
    if (i > n) n = i;

    dx = (double) (x2 - x1) / (double) n;
    dy = (double) (y2 - y1) / (double) n;

    x = x1 + 0.5; 
    y = y1 + 0.5;

    for (i = 0; i < n; i++) {
        int ix = (int) x;
        int iy = (int) y;
        if ((ix == x1 && iy == y1) ||
            (ix == x2 && iy == y2)) 
        {
            /* we only check in between squares, not start and end */
        } else if (isOcclusive (ix, iy)) {
            return 0;
        }
        x += dx;
        y += dy;
    }
    return 100;
}


