/* 
 * File:   dgame.cpp
 * Author: Rarepon
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */


#include "dgame.h"
#include <fstream>
#include <time.h>


//init the board is redundant - August 28, 2011
DraughtsGame::DraughtsGame()
{
   //initboard(&gameboard);
}

/* Autoplay - This is the main function of the DraughtGame Class. It plays
	a set number of games between two DirectNet's, using a specified board.
   It ouputs the results and returns numwins*2 + numdraws.
*/
int DraughtsGame::AutoPlay(char* fname, int train, int numofgames, BOARD* board, float g, float l, DirectNet* opp1, DirectNet* opp2, float drawb, float drawr)
{
	int 	opp1won, opp2won, draw;
	int 	i, over;
	float	avgpieces, avgmoves;

	//init var's
	opp1won = opp2won = draw = 0;
	avgpieces = avgmoves = 0.0;

	//setup parameters
	gamma=g;
	lambda=l;
	DRAWB = drawb;
	DRAWR = drawr;

	for(i =0; i<numofgames; i++)
	{
   		//opp1 starts even games
	  	if(i%2 ==0)
		{
      		bnet = opp1;
			rnet = opp2;
			opp1_is_black = 1;
		}
		else
		{
      		bnet = opp2;
			rnet = opp1;
			opp1_is_black = 0;
		}

		//init the game, and save file

   		InitGame(fname, board, train);

		over = 0;
		while (!over)
		{
      		//make a black move
      		over = MakeBlackMove();

			//check if game over
			if(over == BLACKWON)
			{
         		if(opp1_is_black) 
					opp1won++;
				else	  		
					opp2won++;
			}
			else if(over == DRAW) 
				draw++;
			else if(over == REDWON)
			{
         		if(opp1_is_black) 
					opp2won++;
				else			
					opp1won++;
			}
			else
			{
         		//make red move
         		over = MakeRedMove();
				//check if over
				if(over == REDWON)
					{
            		if(opp1_is_black) 
						opp2won++;
					else
						opp1won++;
					}
            else if(over == BLACKWON)
            {
				if(opp1_is_black) 
					opp1won++;
				else
					opp2won++;
            }
            else if(over == DRAW) 
				draw++;
			}
      }
      //cleanup game
      EndGame(over);
      //keep talies of piece and moves
      avgpieces += pieces;
      avgmoves  += movenum;
   }
   //give the results
   cout<<"\tResults: "<<opp1won<<" "<<opp2won<<" "<<draw;
   cout<<"\tAvgs: "<<avgmoves/numofgames<<"\t"<<avgpieces/numofgames<<endl;
   return ((opp1won*2)+draw);
}

/* Init a game: set's up the board and PDN file for recording */
void DraughtsGame::InitGame(char* fname, BOARD* board, int train)
{
	char mydate[9];
	char buf[64];

	_strdate(mydate);

	copyordboard(&gameboard, board);

	//current game doesn't enter the if stmt - August 28, 2011
	if(fname != NULL)
	{
   		//save PDN info
   		recording = 1;
		pdnfile.open(fname, ios::app);
   		if(!pdnfile)
      		cerr<<"Error opening: "<<fname<<" for output!\n";

		bnet->GetName(buf);
		pdnfile<<"[Event \"NeuroDraughts playing against itself using: \"]\n\n";
		pdnfile<<"[Black \""<<buf<<"\"]\n";

		rnet->GetName(buf);
		pdnfile<<"[Red   \""<<buf<<"\"]\n";
		pdnfile<<"[Date  \""<<mydate<<"\"]\n\n";
	}
	training1 = training2 = 0;

	//Turns on the TD Training - August 28, 2011
	//whose being trained?
	if((train & TRAIN_OPP1) == TRAIN_OPP1){
   		training1 = 1;
	}
	if((train & TRAIN_OPP2) == TRAIN_OPP2){
   		training2 = 1;
	}
	movenum = 0;
}

/* Endgame - cleans up the game, giving final rewards, aswell as closing up
	the record PDN file.
 */
void DraughtsGame::EndGame(int whowon)
{
	//if we're training it
	if(training1)
   {
   	//if he's black
	  	if(opp1_is_black)
	   {
	    	if(whowon == BLACKWON)
		     	bnet->TDFinal(WINVALUE);
	      else if(whowon == REDWON)
	      	bnet->TDFinal(LOSEVALUE);
	   	else
         	bnet->TDFinal(DRAWB);
	   }
	   else
	   {
      	//otherwise he's red
	   	if(whowon == REDWON)
				rnet->TDFinal(WINVALUE);
	      else if(whowon == BLACKWON)
	      	rnet->TDFinal(LOSEVALUE);
	   	else
         	rnet->TDFinal(DRAWR);
	   }
   }

   //cleanup file
   if(recording)
   {
   	if (whowon == BLACKWON)
        	pdnfile<<" 1-0 \n\n";
      else if(whowon == REDWON)
      	pdnfile<<" 0-1 \n\n";
      else
      	pdnfile<<" 1/2-1/2 \n\n";
   }
   pdnfile.close();
}

/* make blacks move - uses SuggestMove from player.cpp */
int DraughtsGame::MakeBlackMove(void)
{
	MOVE 	smove;
	XBOARD 	xb;
	float 	best, worst;
	//All s does not work
	//int s;

	best = -1;
	worst =+1;

	maptoxboard(&gameboard, &xb);
	
	//now follows a greedy policy - August 29, 2011
	SuggestMove(&gameboard, &smove, bnet, BLACKPLAYER,0, 0, &best, &worst, &qmapboard);

	//check for stuck!
	if(smove.from == -1)
		return REDWON;
	else if(smove.exchange)
		//s = applyexchange(&xb, smove.from, smove.to);
		applyexchange(&xb, smove.from, smove.to);
	else
		applymove(&xb, smove.from, smove.to );
   

	//historical bug check
	//if(s == -1) cout<<"it was here in dgame black!!!!"<<smove.from<<" "<<smove.to<<"\n\n";

	maptoboard(&gameboard, &xb);

	//train the net
	//opp1_is_black alternately trains the network to opp1 and to opp2 by using modulo(2) - August 29, 2011
	if(opp1_is_black && training1)
	{	
   		if(movenum == 0){
			bnet->InitTDTrain(&gameboard, gamma, lambda, 0, &qmapboard);
		}
		else{
        	bnet->TDTrain(&gameboard,0,&qmapboard);
		}
	}

	//record PDN data
	if(recording)
	{
   		pdnfile<<" "<<(movenum+1)<<". "<<smove.from;
		if(smove.exchange)
      		pdnfile<<"x";
		else
      		pdnfile<<"-";
		pdnfile<<smove.to;
	}

	//are we fininshed
	return checkgameover();
}

//same as for makeblackmove
int DraughtsGame::MakeRedMove(void)
{
	MOVE smove;
	XBOARD xb;

	float best, worst;
	best = -1;
	worst =+1;
	//All s does not run, compiler error probably
	//int s;

	maptoxboard(&gameboard, &xb);

	//now follows a greedy policy - August 29, 2011
	SuggestMove(&gameboard, &smove, rnet, REDPLAYER,0, 0, &best, &worst, &qmapboard);
	
	//check for stuck!
	if(smove.from == -1)
   		return BLACKWON;
	else if(smove.exchange)
		//s = applyexchange(&xb, smove.from, smove.to);
    	applyexchange(&xb, smove.from, smove.to);
	else
	 	applymove(&xb, smove.from, smove.to );

	//if(s == -1) cout<<"it was here in dgame!!!!"<<smove.from<<" "<<smove.to<<"\n\n";

	maptoboard(&gameboard, &xb);

	//opp1_is_black alternately trains the network to opp1 and to opp2 by using modulo(2) - August 29, 2011
	if(!opp1_is_black && training1)
	{
   		if(movenum == 0){
      		rnet->InitTDTrain(&gameboard, gamma, lambda,1, &qmapboard);
		}
		else{
        	rnet->TDTrain(&gameboard,1,&qmapboard);
		}
	}

	movenum++;
	if(recording)
	{
   		pdnfile<<" "<<smove.from;
		if(smove.exchange)
      		pdnfile<<"x";
		else
      		pdnfile<<"-";
		pdnfile<<smove.to;

		if(movenum > 1 && movenum % 4 ==0)
			pdnfile<<endl;
	}
	return checkgameover();
}

/*	checkgameover - checks if anyone has lost, or if the movelimit has been
	exceeded
 */
int DraughtsGame::checkgameover()
{
	int i,b,r;

	b=r=0;
	for(i=1; i<=32; i++)
	{
   		switch(gameboard.p[i])
		{
      		case	BLACKMAN:	b+=2;break;
			case	BLACKKING:	b+=3;break;
			case	REDMAN:		r+=2;break;
			case	REDKING:		r+=3;break;
		}
	}
	if(opp1_is_black)
   		pieces = b-r;
	else
   		pieces = r-b;

	if(b==0)
   		return REDWON;
	else if(r==0)
   		return BLACKWON;
	else if(movenum > 50)
   		return DRAW;
	else
   		return 0;
}
//obsolete function, returns the board struct
BOARD* DraughtsGame::GetBoard()
{
	return &gameboard;
}
//obsolete function, forces a direct move
int	DraughtsGame::ForceMove(MOVE* m)
{
	XBOARD xb;

   maptoxboard(&gameboard, &xb);
   maptoxmove(m);

   if(m->exchange)
   	applyexchange(&xb, m->from, m->to);
   else
   	applymove(&xb, m->from, m->to);

   maptoboard(&gameboard, &xb);
   return checkgameover();
}