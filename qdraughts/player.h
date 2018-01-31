#ifndef __DPLAYERNET
#define __DPLAYERNET

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "dirnet.h"      
#include "board.h"

#define BLACKPLAYER	1
#define REDPLAYER	2

#define MAXJUMPS	16
#define MAXMOVES	64

#define NOVALUE 0
#define JUMPVALUE 1
#define MOVEVALUE 1

float SuggestMove(BOARD* b, MOVE* m, DirectNet* d, int player, int totaldepth, int depth, float* best, float* worst, QBOARD* qmapboard);

#endif