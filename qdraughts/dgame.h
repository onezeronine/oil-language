/* 
 * File:   dgame.h
 * Author: Rarepon
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */


#ifndef __DGAME
#define __DGAME

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "dirnet.h"
#include "player.h"
#include "board.h"
#include "train.h"

#define	TRAIN_OPP1 1
#define	TRAIN_OPP2 2
#define	TRAIN_BOTH 3

class DraughtsGame
{
private:
   DirectNet*			bnet;
   DirectNet*			rnet;
   //Q-mapping - August 29, 2011
   QBOARD qmapboard;
   BOARD	gameboard;
   int		movenum;		//what move num - used for writing out PDN file
   int		training1;	//are we training for opponent 1
   int		training2;	//are we training for opponent 2
   int		recording;	//are we saving the game?
   fstream  pdnfile;		//file to store games
   float		gamma, lambda;
   int 		opp1_is_black;
   int 		pieces;
   float		DRAWB, DRAWR;


public:
	DraughtsGame(); 

   BOARD* GetBoard();
   int	AutoPlay(char* fname, int train, int numofgames, BOARD* board, float gamma, float lambda, DirectNet* opp1, DirectNet* opp2, float drawb, float drawr);
   int   MakeBlackMove(void);
   int	MakeRedMove(void);
   void	InitGame(char* fname, BOARD* board, int train);
   int	GameLength();
   void	EndGame(int whowon);
   int 	checkgameover();

   int 	ForceMove(MOVE* m);
void	MiniInitGame();   
};

#endif