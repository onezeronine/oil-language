#include "player.h"

inline int otherplayer(int p);
int jumpcalc(XBOARD* xb, int pos, int dir);

/* the func - minimum and maximum correspond to alpha and beta repsectively.
	These names used instead as they better convey their use
 */
float SuggestMove(BOARD* b, MOVE* m, DirectNet* net, int player, int totaldepth, int depth, float* minimum, float* maximum, QBOARD* qmapboard)
{
	//arrays of moves and jumps
	MOVE	jumps[MAXJUMPS];
	MOVE	moves[MAXMOVES];
	int	nummoves, numjumps, numnone;

	//general var's
	float 	besteval, res;
	float qvalue_jump[256];
	float qvalue_move[256];
	float qvalue_none[256];
	int		i, man,king;
	XBOARD 	xb,temp;
	int 	j,mdir,pos,dir,mini,top;
	BOARD 	normalboard;

	//are we at the top of the tree?
	if(totaldepth == depth)
		//always 1 if depth =0 - August 28,2011
		top = 1;
	else 
		top = 0;

	//min or max ply?
	if((depth > 0) && ((depth%2)==1))
   		mini = 1;
	else
		//always 0 - August 28,2011
   		mini = 0;

   //which player are we?
	if(player == BLACKPLAYER){
		man = BLACKMAN;
		king = BLACKKING;
	}
	else{
   		man = REDMAN;
		king = REDKING;
	}

	//init stuff
	nummoves=numjumps=numnone=0;
	maptoxboard(b, &xb);

	//Note: Mapping the actions happens here.
	//qmapboard locates if jumps, moves or none at all
	for(i=1; i<=32; i++){
   		if(b->p[i]==man || b->p[i]==king){
			pos = maptoxcoord(i);
			j = jumpsavail(&xb, pos);
			mdir = possiblemoves(&xb, pos);
			if (j != NONE){
				for(dir=1; dir<=8; dir=dir*2){
					if((j & dir)==dir){
               			copyboard(&temp,&xb);
				 		jumps[numjumps].from = pos;
						jumps[numjumps].to = jumpcalc(&temp,pos,dir);
						//q-maps the available jumps
						//Q-mapping is not using greedy, error - September 8, 2011
						qmapboard->p[i][dir] = 3;
						qvalue_jump[numjumps] = net->EvaluateQ(qmapboard);						
						qmapboard->p[i][dir] = 0;
   						numjumps++;
					}
				}
			}
			else if(mdir!=NONE){
				for(dir=1; dir<=8; dir=dir*2){
					if((mdir & dir)==dir){
						moves[nummoves].from = pos;
						moves[nummoves].to = pos+poscalc(dir);
						//q-maps the available moves
						qmapboard->p[i][dir] = 2;
						qvalue_move[nummoves] = net->EvaluateQ(qmapboard);	
						qmapboard->p[i][dir] = 0;
						nummoves++;
					}
				}
			}
			else{
				for(dir=1;dir<=8; dir=dir*2){
					qmapboard->p[i][dir] = 1;
					qvalue_none[numnone] = net->EvaluateQ(qmapboard);	
					qmapboard->p[i][dir] = 0;
					numnone++;
				}
			}
		}
		else{
			for(dir=1;dir<=8; dir=dir*2){
				qmapboard->p[i][dir] = 0;
			}
		}
	}


	//single jump - return straight no choice
	//although it returns hastily, it trains the network through dgame functions
	//MakeBlackMove and MakeRedMove - August 29, 2011
	if(top && (numjumps == 1)){
		m->from = jumps[0].from;
		m->to = jumps[0].to;
   		m->exchange = 1;
		return 0;
	}

	//single move - return straight no choice
	if(top && (nummoves == 1)){
		m->from = moves[0].from;
		m->to = moves[0].to;
   		m->exchange = 0;
		return 0;
	}

	//min ply
	if(mini)
   		besteval = +2;
	//max ply
	else
   		besteval = -2;

	if(numjumps > 0)
	{	
		for(i=0; i<numjumps; i++){
			copyboard(&temp, &xb);
			applyexchange(&temp, jumps[i].from, jumps[i].to);

			//only want to evaluate on depth =0
			//depth is now always equal to 0 - August 28, 2011
			if(depth == 0){
         		//reverse board for eval if red
				if(player==REDPLAYER){
			 		mirrorboard(&temp);
					reversealign(&temp);
				}

				maptoboard(&normalboard, &temp);
				//uses what the network evaluates as outputunit
				if(net->GetQLearn() == 0)
     				res = net->EvaluateNet(&normalboard, qmapboard);
				else
					res = qvalue_jump[i]; 
			}
			else{
		   		MOVE dummymove;
		 		maptoboard(&normalboard, &temp);

				//recursion on suggestmove, slows down computation - August 28, 2011
				res = SuggestMove(&normalboard, &dummymove, net, otherplayer(player), 5000, depth-1, minimum, maximum, qmapboard);
			}
			//mini is always 0 - August 28, 2011
			if(mini){
				if(res < besteval)
            		besteval = res;
			}
			else{
				//Here happens the evaluation if res is greater than besteval...
         		if(res > besteval){
	           		besteval = res;
					if(top){
						m->from = jumps[i].from;
						m->to = jumps[i].to;
						m->exchange = 1;
					}
				}
			}
		}
	}


	else if(nummoves > 0){
	   for(i=0; i<nummoves; i++){
			copyboard(&temp, &xb);
			applymove(&temp, moves[i].from, moves[i].to);
			//only want to evaluate on depth = 0
			//depth is now always equal to 0 - August 28, 2011
			if(depth == 0){
				if(player==REDPLAYER){
			 		mirrorboard(&temp);
					reversealign(&temp);
				}
				maptoboard(&normalboard, &temp);
				if(net->GetQLearn() == 0)
     				res = net->EvaluateNet(&normalboard,qmapboard);
				else
					res = qvalue_move[i];
			}
			else{
		   		MOVE dummymove;
		 		maptoboard(&normalboard, &temp);
				//recursion on suggestmove, slows down computation - August 28, 2011
				res = SuggestMove(&normalboard, &dummymove, net, otherplayer(player), 5000, depth-1, minimum, maximum, qmapboard);
			}

			//mini is always 0 - August 28, 2011
			if(mini){
         		//alpha beta prune!
				//if(res <= *minimum)
				//return res;
				
				if(res < besteval)
            		besteval = res;
			}
			else{
           	//alpha beta prune!
			//if((res >= *maximum) && !top)
			//return res;
         		if(res > besteval){
	           		besteval = res;
					if(top){
						m->from = moves[i].from;
						m->to =  moves[i].to;
						m->exchange = 0;
					}
				}
			}
		}
	}
	
	//no moves
	else{
   		if(mini)
      		besteval = 1;
		else{
      		m->from = -1;
			besteval = -1;
		}
	}
  	//alpha beta stuff!
	//if(mini)
		//*minimum = besteval;
	//else
		//*maximum = besteval;
	return besteval;
}

//switch players
inline int otherplayer(int p){
	if(p==BLACKPLAYER) return REDPLAYER;
	else return BLACKPLAYER;
}

/* calculate the finishing position for ajump */
int jumpcalc(XBOARD* xb, int pos, int dir){
	int finished;
	int nowat;
	int j;

	finished = jumppiece(xb, pos, dir);
	nowat = pos + (poscalc(dir) * 2);
	while(!finished){
		if((j = jumpsavail(xb,nowat))!=NONE){
			//because it's unlikely to branch at this stage... (fingers crossed) only one is considered
			if ((j & NE)==NE) j = NE;
			if ((j & NW)==NW) j = NW;
			if ((j & SE)==SE) j = SE;
			if ((j & SW)==SW) j = SW;
			finished = jumppiece(xb, nowat, j);
			nowat = nowat + (poscalc(j) * 2);
		}
		else
      		finished = 1;
   }
   return nowat;
}