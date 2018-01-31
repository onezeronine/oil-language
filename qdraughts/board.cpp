/* 
 * File:   board.cpp
 * Author: Kenneth Bastian
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */

#include "board.h"

/*initlialises a board to standard starting position */
void initxboard(XBOARD* xb)
{
	int i;

   xb->p[0] = INVALID;

   for(i=1; i<=13; i++)
   	xb->p[i] = BLACKMAN;
   xb->p[9] = INVALID;

   for(i=14; i<=22; i++)
   	xb->p[i] = EMPTY;
   xb->p[18] = INVALID;

   for(i=23; i<=35; i++)
   	xb->p[i] = REDMAN;
   xb->p[27] = INVALID;
}

/*initlialises a board to standard starting position */
void initboard(BOARD* xb)
{
	int i;

   xb->p[0] = INVALID;
   for(i=1; i<=12; i++)
   	xb->p[i] = BLACKMAN;
   for(i=13; i<=20; i++)
   	xb->p[i] = EMPTY;
   for(i=21; i<=32; i++)
   	xb->p[i] = REDMAN;
}


/*copies two XBOARD's*/
void copyboard(XBOARD* dest, XBOARD* src)
{
	int i;

   for(i=1; i<=35; i++)
   {
   	dest->p[i] = src->p[i];
   }
}

/*copies two BOARD's*/
void copyordboard(BOARD* dest, BOARD* src)
{
	int i;

   for(i=1; i<=32; i++)
   {
   	dest->p[i] = src->p[i];
   }
}

/* maps an ordinary board to an XBOARD */
void maptoxboard(BOARD* pboard, XBOARD* pxboard)
{
	int i;

   pxboard->p[0] = INVALID;
   //positions 1-8 remain unchanged, 9-16 are plus 1, 17-24 are plus 2, 25-32 plus 3
   for (i=1; i<=8; i++)
   	pxboard->p[i] = pboard->p[i];

   pxboard->p[9] = INVALID;
   for (i =9; i<=16; i++)
   {
      pxboard->p[i+1] = pboard->p[i];
   }
	pxboard->p[18] = INVALID;
	for (i =17; i<=24; i++)
   {
      pxboard->p[i+2] = pboard->p[i];
   }
   pxboard->p[27] = INVALID;
   for (i =25; i<=32; i++)
   {
      pxboard->p[i+3] = pboard->p[i];
   }
}

void MapQActions(BOARD* pboard, QBOARD* qmapboard)
{	
	//This should convert the board configuration to Q(s,a)
	//Conditions:
	//1. If action is a jump, 3
	//2. If action is a move, 2
	//3. If action is no move, 1
	//4. If pos is empty, 0
	MOVE	jumps[16];
	MOVE	moves[64];
	int	nummoves, numjumps;

	int		i, man = 2,king = 4;
	XBOARD 	xb,temp;
	int 	j,mdir,pos,dir;
	
	nummoves=numjumps=0;
	maptoxboard(pboard, &xb);
	
	//Note: Mapping the actions happens here.
	//qmapboard locates if jumps, moves or none at all
	for(i=1; i<=32; i++){
   		if(pboard->p[i]==man || pboard->p[i]==king){
			pos = maptoxcoord(i);
			j = jumpsavail(&xb, pos);
			mdir = possiblemoves(&xb, pos);
			if (j != NONE){
				for(dir=1; dir<=8; dir=dir*2){
					if((j & dir)==dir){
               			copyboard(&temp,&xb);
				 		jumps[numjumps].from = pos;
						jumps[numjumps].to = jumpQcalc(&temp,pos,dir);
						qmapboard->p[i][dir] = 3;
						//cout<<i<<"-"<<dir<<"-"<<qmapboard->p[i][dir]<<" ";
   						numjumps++;
					}
				}
			}
			else if(mdir!=NONE){

				for(dir=1; dir<=8; dir=dir*2){
					if((mdir & dir)==dir){
						moves[nummoves].from = pos;
						moves[nummoves].to = pos+poscalc(dir);
						qmapboard->p[i][dir] = 2;
						//cout<<i<<"-"<<dir<<"-"<<qmapboard->p[i][dir]<<" ";
						nummoves++;
					}
				}
			}
			else{
				//for no move
				for(dir=1;dir<=8; dir=dir*2){
					qmapboard->p[i][dir] = 1;
					//cout<<i<<"-"<<dir<<"-"<<qmapboard->p[i][dir]<<" ";
				}
			}
		}
		else{
			//for empty
			for(dir=1;dir<=8;dir=dir*2){
				qmapboard->p[i][dir] = 0;
				//cout<<i<<"-"<<dir<<"-"<<qmapboard->p[i][dir]<<" ";
			}
		}
	}
}

//Added because of MapQActions function
int jumpQcalc(XBOARD* xb, int pos, int dir){
	int finished;
	int nowat;
	int j;

	finished = jumppiece(xb, pos, dir);
	nowat = pos + (poscalc(dir) * 2);
	while(!finished){
		if((j = jumpsavail(xb,nowat))!=NONE){
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




//reverse board mapping
void maptoboard(BOARD* pboard, XBOARD* pxboard)
{
	int i;

   //positions 1-8 remain unchanged, 9-16 are plus 1, 17-24 are plus 2, 25-32 plus 3
   for (i=1; i<=8; i++)
   	pboard->p[i] = pxboard->p[i];

   for (i =9; i<=16; i++)
   {
      pboard->p[i] = pxboard->p[i+1];
   }
	for (i =17; i<=24; i++)
   {
      pboard->p[i] = pxboard->p[i+2];
   }
   for (i =25; i<=32; i++)
   {
      pboard->p[i] = pxboard->p[i+3];
   }
}

//mirror board - over the double diagonal
void mirrorboard(XBOARD* b)
{
	int i,j;
	//t is temporary
	int t;

   j=35;
   for(i=1; i<=31; i+=5)
   {
		//swap the value of p[i] to p[j]
		//p[1] swap j[35]
	   t=b->p[j];
	   b->p[j] = b->p[i];
	   b->p[i] = t;

      j-=5;
   }
   j=34;
   for(i=2; i<=22; i+=5)
   {
	   t=b->p[j];
	   b->p[j] = b->p[i];
	   b->p[i] = t;
      j-=5;
   }
   j=33;
   for(i=3; i<=13; i+=5)
   {
	   t=b->p[j];
	   b->p[j] = b->p[i];
	   b->p[i] = t;
      j-=5;
   }
   t=b->p[32];
   b->p[32] = b->p[4];
   b->p[4] = t;
}

//reverse the alignment of the pieces
void reversealign(XBOARD* b)
{
	int i;

   for(i=1; i<=35; i++)
   {
   	if(b->p[i] == BLACKMAN) b->p[i] = REDMAN;
      else if(b->p[i] == REDMAN) b->p[i] = BLACKMAN;
      else if(b->p[i] == BLACKKING) b->p[i] = REDKING;
      else if(b->p[i] == REDKING) b->p[i] = BLACKKING;
   }
}

//convert move to Xtended representation
void maptoxmove(MOVE* m)
{
	m->from = maptoxcoord(m->from);
	m->to = maptoxcoord(m->to);
}
//convert move to ordinary representation
void maptomove(MOVE* m)
{
	m->from = maptocoord(m->from);
	m->to = maptocoord(m->to);
}

//over to XCOORD
int maptoxcoord(int i)
{
	if(i>=9 && i<=16)
   	return i+1;
   if(i>=17 && i<=24)
   	return i+2;
   if(i>=25 && i<=32)
   	return i+3;

   return i;
}
//over to ordinary COORD
int maptocoord(int i)
{
   if(i>=10 && i<=17)
   	return i-1;
   if(i>=19 && i<=26)
   	return i-2;
   if(i>=28 && i<=35)
   	return i-3;

   return i;
}

/*print to file, boardlog - note: must change to take a file name */
void printxboard(XBOARD* b)
{
	int i;
   fstream df;

   df.open("c:\\ul\\fyp\\nd\\boardlog.txt", ios::app);

   df<<endl<<endl;

   for (i=35; i>=32; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=31; i>=28; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=26; i>=23; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=22; i>=19; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=17; i>=14; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=13; i>=10; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=8; i>=5; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=4; i>=1; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;

   df.close();
}

/*print to file, boardlog - note: must change to take a file name */
void printboard(BOARD* b)
{
	int i;
   fstream df;

   df.open("c:\\ul\\fyp\\nd\\boardlog.txt", ios::app);

   for (i=32; i>=29; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=28; i>=25; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=24; i>=21; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=20; i>=17; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=16; i>=13; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=12; i>=9; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl;
	for (i=8; i>=5; i--)
      df<<" "<<printpiece(b->p[i]);
   df<<endl;
   for (i=4; i>=1; i--)
      df<<printpiece(b->p[i])<<" ";
	df<<endl<<endl;

   df.close();
}

/*print a move*/
void printmove(MOVE* m)
{
	int i;
   fstream df;

   df.open("c:\\ul\\fyp\\nd\\boardlog.txt", ios::app);

   df<<"MOVE from:"<<m->from<<" to:"<<m->to<<" ex:"<<m->exchange<<endl;

   df.close();
}

//print a piece- used by print board functions.
char printpiece(int p)
{
	if(p == BLACKMAN)
     	return 'b';
   else if(p == BLACKKING)
     	return 'B';
   else if(p == REDMAN)
     	return 'r';
   else if(p == REDKING)
     	return 'R';
   else
     	return '#';
}

/* is he a bad guy??! */
int badguy(int me, int opp)
{
	if (me == BLACKMAN || me ==BLACKKING)
   {
   	if(opp == REDKING || opp == REDMAN)
      	return 1;
      else return 0;
   }
   else if(me == REDMAN || me == REDKING)
   {
   	if(opp == BLACKMAN || opp == BLACKKING)
      	return 1;
      else return 0;
   }
   return 0;
}

/* check for kings */
int promocheck(XBOARD* b)
{
	int i,found =0;

   for (i = 32; i<=35; i++)
   {
   	if(b->p[i] == BLACKMAN)
      {
      	b->p[i] = BLACKKING;
         found=1;
      }
   }
	for (i = 1; i<=4; i++)
   {
   	if(b->p[i] == REDMAN)
      {
      	b->p[i] = REDKING;
         found=1;
      }
   }
   return found;
}

//apply a move
void applymove(XBOARD* b, int from, int to)
{

   b->p[to] = b->p[from];
   b->p[from] = EMPTY;
   promocheck(b);

}

/* recursive function to find the path of a jump*/
int applyexchange(XBOARD* b, int from, int to)
{	 
   fstream df;
   int j,dir;

   j = jumpsavail(b, from);

   //serious bug trap!, occurs when some function calls with illegal positions.
   //left in so that it NEVER happens again!
   if(j==-1)
   {
   	cout<<"ITWAS ME - APPYLEXCHANGE! - called with "<<from<<" "<<to<<"\n\n";
	   return -1;
   }
   
   if(j==NONE)
   {
   	if(from == to)
      	return 1;
      else
      	return 0;
	}
   //single option
	else if(j == NE || j == NW || j == SE || j== SW)
	{
   		if(jumppiece(b, from, j)) return 1;
   		return applyexchange(b, from +(poscalc(j) * 2), to);
	}
   //multiple
   else
   {
   		XBOARD backup;
	  	copyboard(&backup, b);

	   //try each direction - restoring board in case of failure
		for(dir = 1; dir <= 8; dir = dir*2)
		{
      		if((j & dir) == dir)
			{
         		if(jumppiece(b, from, dir)) return 1;
				if(applyexchange(b, from+(poscalc(dir)*2), to) == 1)
	         		return 1;
				else
	         		copyboard(b, &backup);
   	   }
      }
   }
   return 0;
} 


/* applys a jump in a given direction --- returns promocheck() */
int jumppiece(XBOARD* b, int from, int j)
{
	int x;

   x= poscalc(j);
	//move piece, erase his piece and your old piece position
   b->p[(from+(x*2))] = b->p[from];
   b->p[(from+x)] = EMPTY;
   b->p[from] = EMPTY;
	//check for promotions
   return promocheck(b);
}

//applys a move in a given direction (m)
void movepiece(XBOARD* b, int from, int m)
{
	int x=poscalc(m);

   b->p[from+x] = b->p[from];
   b->p[from] = EMPTY;
   promocheck(b);
}

/* returns the pos to the NE/NW/SE/SW of current pos */
int poscalc(int j)
{
	int x;

  	if(j==NE) x = 4;
   else if(j==NW) x = 5;
   else if(j==SE) x = -5;
   else if(j==SW) x = -4;
   else x=0;

   return x;
}

/* calculates possible jumps from pos given ... returns combination of SE SW NW NE */
int jumpsavail(XBOARD* b, int p)
{
	int ret = NONE;

   if(p>35 || p <1 || p == 9 ||p == 18 || p ==27)
   {
   	cout<<"Jumps avail! from : "<<p<<" for: "<<b->p[p]<<endl;
      printxboard(b);
      return -1;
   }

   //forward jumps
	if(b->p[p] == BLACKMAN || b->p[p] == BLACKKING || b->p[p] == REDKING)
   {
		if(inarea(p, 3) && badguy(b->p[p], b->p[p+4]) && b->p[p+8] == EMPTY)
      	ret += NE;
  		if(inarea(p, 4) && badguy(b->p[p], b->p[p+5]) && b->p[p+10] == EMPTY)
  			ret += NW;
  	}
   //backward jumps
   if(b->p[p] == REDMAN || b->p[p] == REDKING || b->p[p] == BLACKKING)
	{
   	if(inarea(p, 1) && badguy(b->p[p], b->p[p-5]) && b->p[p-10] == EMPTY)
      	ret += SE;
    	if(inarea(p, 2) && badguy(b->p[p], b->p[p-4]) && b->p[p-8] == EMPTY)
    		ret += SW;
   }
   return ret;
}

/* checks if its in the 'greater area' specified */
int inarea(int p, int area)
{
	int r;

   r=inquadrant(p);
   //centre tiles are part of all greater areas
	if(r == 5)
   	return 1;

   switch (area)
   {
   	case 1:	if(r==1 || r==2 || r==4) return 1;
      			return 0;
      case 2:	if(r==2 || r==3 || r==6) return 1;
      			return 0;
      case 3:	if(r==4 || r==7 || r==8) return 1;
      			return 0;
      case 4:	if(r==6 || r==8 || r==9) return 1;
      			return 0;
      default:	return 0;
   }
}

/* takes an position and returns which qudrant its in -see notes for quad info*/
int inquadrant(int p)
{
	switch(p)
   {
 	 	case 1:  case 5: 							return 9;
      case 2:  case 3: 	case 6: 	case 7: 	return 8;
      case 4:  case 8:							return 7;
      case 10: case 14: case 19: case 23:	return 6;
      case 13: case 17: case 22: case 26:	return 4;
      case 28: case 32: 						return 3;
      case 29: case 30: case 33: case 34: return 2;
      case 31: case 35:							return 1;
      //otherwise in centre
      default: return 5;
   }
}


//returns possible moves for apiece (xboard coords)
int possiblemoves(XBOARD* xb, int from)
{
	int ret=NONE;
   int forward =0, backward=0;

   if(xb->p[from] == BLACKMAN || xb->p[from] == BLACKKING || xb->p[from] == REDKING)
   	forward =1;
   if(xb->p[from] == REDMAN || xb->p[from] == BLACKKING || xb->p[from] == REDKING)
   	backward=1;

   //check single corners
   if(from == 4)
   {
   	if(xb->p[from + poscalc(NE)] == EMPTY && forward ==1)
      	ret = NE;
   }
   else if(from == 32 && backward ==1)
   {
   	if(xb->p[from + poscalc(SW)] == EMPTY)
      	ret = SW;
   }
   //bottom squares
   else if(from <= 3 && forward == 1)
   {
    	if(xb->p[from + poscalc(NE)] == EMPTY)
      	ret = NE;
     	if(xb->p[from + poscalc(NW)] == EMPTY)
      	ret += NW;
   }
   //top squares
   else if(from >= 33 && backward ==1)
   {
     	if(xb->p[from + poscalc(SE)] == EMPTY)
      	ret = SE;
     	if(xb->p[from + poscalc(SW)] == EMPTY)
      	ret += SW;
   }
   //left squares
   else if(from == 13 || from == 22 || from == 31)
   {
     	if(xb->p[from + poscalc(SE)] == EMPTY && backward==1)
      	ret = SE;
     	if(xb->p[from + poscalc(NE)] == EMPTY && forward ==1)
      	ret += NE;
   }
   //right sqaures
   else if(from == 5  || from == 14 || from == 23)
   {
     	if(xb->p[from + poscalc(NW)] == EMPTY && forward ==1)
      	ret = NW;
     	if(xb->p[from + poscalc(SW)] == EMPTY && backward ==1)
      	ret += SW;
   }
   //otherwise in the middle
   else
   {
     	if(xb->p[from + poscalc(SE)] == EMPTY && backward==1)
      	ret = SE;
     	if(xb->p[from + poscalc(SW)] == EMPTY && backward==1)
      	ret += SW;
     	if(xb->p[from + poscalc(NE)] == EMPTY && forward==1)
      	ret += NE;
     	if(xb->p[from + poscalc(NW)] == EMPTY && forward==1)
      	ret += NW;
   }
   return ret;
}


void ResetQBoard(QBOARD* qmapboard){


}