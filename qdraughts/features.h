
#ifndef __FEATURESHEADER
#define __FEATURESHEADER

#include <stdlib.h>
#include "board.h"
#include "evalnet.h"

#define NUMOFFEATURES	13

int advancement(BOARD* pboard);
int backrowbridge(BOARD* pboard);
int centrecontrol(BOARD* pboard);
int xcentrecontrol(BOARD* pboard);
int totalmobility(BOARD* pboard);

//added 7/04/97
int pieceadvantage(BOARD* b);
int piecedisadvantage(BOARD* b);

//added 10/3/97
int doublediagonal(BOARD* pb);
int diagonalmoment(BOARD* pb);
int exposure(BOARD* pb);
int kingcentrecontrol(BOARD* pboard);

//added 15/04/97
int piecethreat(BOARD* b);
int piecetake(BOARD* b);
int threat(BOARD* b);

//caculate next bit
int calculatenextbit(BOARD* b);


//helper functions
int checkcentre(BOARD* pb, BOARDVALUES b);
int redcanmoveto(XBOARD* xb, int to);
int blackcanmoveto(XBOARD* xb, int to);
int possibledirections(int to);
int piececount(BOARD* b);

int rcentrecontrolcalc(XBOARD* xb, BOARD* pboard);
int bcentrecontrolcalc(XBOARD* xb, BOARD* pboard);

int calcblackpiecethreat(XBOARD* xb);
int calcredpiecethreat(XBOARD* xb);


//dead stuff
//int testjumps(XBOARD* xb, int pos, int jump);
//int possiblejumps(XBOARD* xb, int p);


#endif
