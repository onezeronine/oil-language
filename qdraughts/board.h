/* 
 * File:   board.h
 * Author: Kenneth Bastian
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */

#ifndef __BOARDHEADER
#define __BOARDHEADER

#include <fstream>
#include <iostream>

using namespace std;

//im the board header file... weee

enum	BOARDVALUES {INVALID = -1, EMPTY = 0, BLACKMAN, REDMAN, BLACKKING, REDKING};
enum	JUMPS	{NONE =0 , NE=1, NW=2, SE=4, SW=8};

//current board structs... extra piece because arrays start at 1. makes calculations
//that bit easier ! subject to change :-)
struct BOARD
{
	int	p[33];
};

struct XBOARD
{
	int	p[36];
};

//For Q-mapping
struct QBOARD
{
	int p[33][8];
};

struct MOVE
{
	int from;
	int to;
	int exchange;
};

//For Q-mapping in red player
void MapQActions(BOARD* pboard, QBOARD* qmapboard);
int jumpQcalc(XBOARD* xb, int pos, int dir);
void ResetQBoard(QBOARD* qmapboard);

//init board
void initboard(BOARD* b);
void initxboard(XBOARD* xb);

//maps normal representation to the extended one (see Samuels)
void maptoxboard(BOARD* pboard, XBOARD* pxboard);
void maptoboard(BOARD* pboard, XBOARD* pxboard);

void maptoxmove(MOVE* m);
void maptomove(MOVE* m);

int maptoxcoord(int x);
int maptocoord(int x);

void printxboard(XBOARD* b);
void printboard(BOARD* b);
void printmove(MOVE* m);

void copyboard(XBOARD* dest, XBOARD* src);
void copyordboard(BOARD* dest, BOARD* src);
char printpiece(int p);

/* takes an position and returns which quadrant its in -see notes for quad info*/
int inquadrant(int p);

/* is he a bad guy??! */
int badguy(int me, int opp);

/* check for kings */
int promocheck(XBOARD* b);

/* jump a piece */
int jumppiece(XBOARD* b, int from, int j);
void movepiece(XBOARD* b, int from, int m);

/* in particular area? */
int inarea(int p, int area);

/* returns available jumps for particular piece */
int jumpsavail(XBOARD* b, int p);
int possiblemoves(XBOARD* b, int from);

/* help functions */
int poscalc(int j);

void applymove(XBOARD* b, int from, int to);

/* recursive function to find the path of a jump given start and finish */
int applyexchange(XBOARD* b, int from, int to);

void mirrorboard(XBOARD* b);
void reversealign(XBOARD* b);
inline void swapvalues(int b1, int b2);

#endif

