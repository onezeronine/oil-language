#include "features.h"

//calculates the next bit of a feature
int calculatenextbit(BOARD* b)
{
   b->p[0]/=2;
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

/* set to 1 if person has a piece advantage */
int pieceadvantage(BOARD* b)
{
   b->p[0]=piececount(b);
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//if person has disadvantage
int piecedisadvantage(BOARD* b)
{
   b->p[0]=piececount(b);
   if(b->p[0] >= 0) b->p[0] = 0;
   else b->p[0] = abs(b->p[0]);
   if((b->p[0]==1)||(b->p[0]<0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//advancement - pieces in 5/6 rows gets points, 3/4 get minus
int advancement(BOARD* b)
{
	int i, black=0;

//   for(i=1; i<=32; i++)
//   	if(b->p[i] == REDKING || b->p[i] == BLACKKING)
//			return OFFVALUE;

   for (i=17; i<=24; i++)
   	if(b->p[i] == BLACKMAN)
      	black++;
	for (i=9; i<=16; i++)
   	if(b->p[i] == BLACKMAN)
      	black--;

   //-8 to 0 = represented by 0-8, 0-8 = 9-16
   b->p[0]=black+8;
   if((b->p[0]==1)||(b->p[0]<0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//back row bridge - if bridge exists then +1 else -1
int backrowbridge(BOARD* pboard)
{
	int i;

   for(i=1; i<=32; i++)
   	if(pboard->p[i] == REDKING)
			return OFFVALUE;

   if (pboard->p[1]==BLACKMAN && pboard->p[3] == BLACKMAN)
   	return ONVALUE;
   else return OFFVALUE;
}

//centre control I - more control of centre = +1 , less = -1
int centrecontrol(BOARD* b)
{
   b->p[0]=checkcentre(b, BLACKMAN);
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//centre control II - if occupied by pieces or could move to same squares
int xcentrecontrol(BOARD* b)
{
   XBOARD xb;
   maptoxboard(b, &xb);

   b->p[0]=rcentrecontrolcalc(&xb, b);

   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}


//param credited with one for each extra (black) piece in the double diagonals
int doublediagonal(BOARD* b)
{
	int ret=0;
   XBOARD xb;
   int i;

   maptoxboard(b,&xb);

   for(i=1; i<=31; i+=5)
   {
   	if(xb.p[i]==BLACKMAN || xb.p[i] == BLACKKING) ret++;
   	if(xb.p[i+4]==BLACKMAN || xb.p[i+4] == BLACKKING) ret++;
   }

   b->p[0]=ret;
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//diagonal moment value +1 for two removed +2 for one removed +3 for on it
int diagonalmoment(BOARD* b)
{
	int i, ret=0;

   for(i =1; i<=32; i++)
   {
   	if(b->p[i] == BLACKMAN || b->p[i] == BLACKKING)
      {
      	if(i==4 || i==29) ret+=0;
			else if(i==3 || i==8 || i==12 || i ==21 || i == 25 || i==30) ret+=1;
         else if(i==2 || i==7 || i==11 || i==16 || i==20 || i==13 || i==17 || i==22 || i==26 || i==31) ret+=2;
         else ret+=3;
      }
   }
   b->p[0]=ret;
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//exposure - credited with one if exists black piece that is flanked on either diagonal by empty squares
int exposure(BOARD* b)
{
	int ret=0, i;
   XBOARD xb;

   maptoxboard(b,&xb);

   for (i=6; i<=30; i++)
   {
   	if(xb.p[i] == BLACKMAN || xb.p[i] == BLACKKING)
      {
 	   	if(i==13 || i==14 || i==22 || i==23) ret+=0;
         else
         {
         	if(xb.p[i-4] == EMPTY && xb.p[i+4] == EMPTY)
            	ret++;
            else if (xb.p[i-5] == EMPTY && xb.p[i+5] == EMPTY)
            	ret++;
         }
      }
   }

   b->p[0] = ret;
   if((b->p[0]>7)||(b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//king centre control
int kingcentrecontrol(BOARD* b)
{
	int black;

	black= checkcentre(b, BLACKKING);

   b->p[0]=black;
   if((b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//total mobility - any square where the red player can move to.. uses extended board
int totalmobility(BOARD* b)
{
	int ret=0, i;
	XBOARD xb;

   maptoxboard(b, &xb);

   for (i=1; i <=35; i++)
   {
   	if(xb.p[i] == EMPTY)
      {
        	if(redcanmoveto(&xb, i) > NONE)
         	ret++;
      }
	}
   b->p[0] = ret;
   if((b->p[0]>15)||(b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//added 15/04/97 - could opponent take me .
int piecethreat(BOARD* b)
{
	int ret;
   XBOARD xb;

   maptoxboard(b, &xb);

   ret=calcblackpiecethreat(&xb);

   b->p[0] = ret;

   if((b->p[0]>7)||(b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//added 15/04/97 - if we could move again, could we take a piece? - he couldn't take any
int piecetake(BOARD* b)
{
	int ret;
   XBOARD xb;

   maptoxboard(b, &xb);

   ret=calcredpiecethreat(&xb);

   b->p[0] = ret;

   if((b->p[0]>7)||(b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}

//how many of his pieces are under threat from safe pieces?
int threat(BOARD* b)
{
   XBOARD xb;
 	int i,ret=0,j;


   maptoxboard(b, &xb);

   for(i=1; i<=35; i++)
   {
   	if(i==9 || i==18 || i==27) continue;

   	if(xb.p[i] == BLACKMAN || xb.p[i] == BLACKKING)
      {
      	if((j=jumpsavail(&xb, i))!=NONE)
         {
            if((j&NE) == NE)
            {
            	if(inarea((i+poscalc(NE)),2) && xb.p[i+poscalc(SW)] != EMPTY)
               	ret++;
            }
            if((j&NW) == NW)
            {
            	if(inarea((i+poscalc(NW)),1) && xb.p[i+poscalc(SE)] != EMPTY)
               	ret++;
            }
            if((j&SE) == SE)
            {
            	if(inarea((i+poscalc(SE)),4) && xb.p[i+poscalc(SE)] == REDKING && xb.p[i+poscalc(NW)] != EMPTY)
               	ret++;
            }
            if((j&SW) == SW)
            {
            	if(inarea((i+poscalc(SW)),3) && xb.p[i+poscalc(SW)] == REDKING && xb.p[i+poscalc(NE)] != EMPTY)
               	ret++;
            }
         }
      }
   }
   b->p[0] = ret;

   if((b->p[0]>7)||(b->p[0]==1)||(b->p[0]>0 && (b->p[0]%2)==1)) return ONVALUE;
   else return OFFVALUE;
}


int calcblackpiecethreat(XBOARD* xb)
{
	int i,ret=0,j;

   for(i=1; i<=35; i++)
   {
   	if(xb->p[i] == REDMAN || xb->p[i] == REDKING)
      {
      	if((j=jumpsavail(xb, i))!=NONE)
         {
         	if(j==3 || j==5 || j==6 ||j>8)
            	ret+=2;
            else
            	ret++;
         }
      }
   }
   return ret;
}

int calcredpiecethreat(XBOARD* xb)
{
	int i,ret=0,j;

   for(i=1; i<=35; i++)
   {
   	if(xb->p[i] == BLACKMAN || xb->p[i] == BLACKKING)
      {
      	if((j=jumpsavail(xb, i))!=NONE)
         {
         	if(j==3 || j==5 || j==6 ||j>8)
            	ret+=2;
            else
            	ret++;
         }
      }
   }
   return ret;
}

//return piececount - 2 for man , 3 for king

int piececount(BOARD* b)
{
	int i;
	int count =0;

	for(i=1; i<=32; i++)
	{
		switch(b->p[i])
	   {
	    	case BLACKMAN:	count+=2; break;
	      case REDMAN:	count-=2; break;
			case BLACKKING:count+=3; break;
	      case REDKING:	count-=3; break;
         default:			count+=0;
		}
	}
//   cout<<"PieceCount: "<<count<<endl;
	return count;
}




//returns number of given pieces in the centre squares
int checkcentre(BOARD* pboard, BOARDVALUES b)
{
   int ret =0;

   if(pboard->p[10] == b) ret++;
   if(pboard->p[11] == b) ret++;
   if(pboard->p[14] == b) ret++;
   if(pboard->p[15] == b) ret++;
   if(pboard->p[18] == b) ret++;
   if(pboard->p[19] == b) ret++;
   if(pboard->p[22] == b) ret++;
   if(pboard->p[23] == b) ret++;

	return ret-1;
}



//return directions in which red can move to a square - must make sure it's empty!
int redcanmoveto(XBOARD* xb, int to)
{
	int from;
   int ret=0;

   from = possibledirections(to);

   if((from & NE) == NE)
   	if(xb->p[to-4] == REDKING)
      	ret+= NE;

   if((from & NW) == NW)
   	if(xb->p[to-5] == REDKING)
      	ret+= NW;

   if((from & SE) == SE)
   	if(xb->p[to+5] == REDMAN || xb->p[to+5] == REDKING)
      	ret+= SE;

   if((from & SW) == SW)
   	if(xb->p[to+4] == REDMAN || xb->p[to+4] == REDKING)
      	ret+= SW;

   return ret;
}

//return directions in which black can move to a square - must make sure it's empty!
int blackcanmoveto(XBOARD* xb, int to)
{
	int from;
   int ret=0;

   from = possibledirections(to);

   if((from & SE) == SE)
   	if(xb->p[to+5] == BLACKKING)
      	ret+= SE;

   if((from & SW) == SW)
   	if(xb->p[to+4] == BLACKKING)
      	ret+= SW;

   if((from & NE) == NE)
   	if(xb->p[to-4] == BLACKMAN || xb->p[to-4] == BLACKKING)
      	ret+= NE;

   if((from & NW) == NW)
   	if(xb->p[to-5] == BLACKMAN || xb->p[to-5] == BLACKKING)
      	ret+= NW;

   return ret;
}



//takes xboard coord numbers!! -returns direction place could be moved to in
int possibledirections(int to)
{
	int ret;

   //check single corners
   if(to == 4)
   	ret = SW;
   else if(to == 32)
   	ret = NE;
   //bottom squares
   else if(to <= 3)
   	ret = SE + SW;
   //top squares
   else if(to >= 33)
   	ret = NE + NW;
   //left squares
   else if(to == 13 || to == 22 || to == 31)
   	ret = NW + SW;
   //right sqaures
   else if(to == 5  || to == 14 || to == 23)
   	ret = NE + SE;
   //otherwise in the middle
   else
   	ret = NE + NW + SE + SW;

   return ret;
}

int rcentrecontrolcalc(XBOARD* xb, BOARD* pboard)
{
	int ret =0;

   if((pboard->p[10] == REDMAN) || (pboard->p[10] == EMPTY && redcanmoveto(xb, 11))) ret++;
   if((pboard->p[11] == REDMAN) || (pboard->p[11] == EMPTY && redcanmoveto(xb, 12))) ret++;
   if((pboard->p[14] == REDMAN) || (pboard->p[14] == EMPTY && redcanmoveto(xb, 15))) ret++;
   if((pboard->p[15] == REDMAN) || (pboard->p[15] == EMPTY && redcanmoveto(xb, 16))) ret++;
   if((pboard->p[18] == REDMAN) || (pboard->p[18] == EMPTY && redcanmoveto(xb, 20))) ret++;
   if((pboard->p[19] == REDMAN) || (pboard->p[19] == EMPTY && redcanmoveto(xb, 21))) ret++;
   if((pboard->p[22] == REDMAN) || (pboard->p[22] == EMPTY && redcanmoveto(xb, 24))) ret++;
   if((pboard->p[23] == REDMAN) || (pboard->p[23] == EMPTY && redcanmoveto(xb, 25))) ret++;

   return ret-1;
}

int bcentrecontrolcalc(XBOARD* xb, BOARD* pboard)
{
	int ret =0;

   if((pboard->p[10] == BLACKMAN) || (pboard->p[10] == EMPTY && blackcanmoveto(xb, 11))) ret++;
   if((pboard->p[11] == BLACKMAN) || (pboard->p[11] == EMPTY && blackcanmoveto(xb, 12))) ret++;
   if((pboard->p[14] == BLACKMAN) || (pboard->p[14] == EMPTY && blackcanmoveto(xb, 15))) ret++;
   if((pboard->p[15] == BLACKMAN) || (pboard->p[15] == EMPTY && blackcanmoveto(xb, 16))) ret++;
   if((pboard->p[18] == BLACKMAN) || (pboard->p[18] == EMPTY && blackcanmoveto(xb, 20))) ret++;
   if((pboard->p[19] == BLACKMAN) || (pboard->p[19] == EMPTY && blackcanmoveto(xb, 21))) ret++;
   if((pboard->p[22] == BLACKMAN) || (pboard->p[22] == EMPTY && blackcanmoveto(xb, 24))) ret++;
   if((pboard->p[23] == BLACKMAN) || (pboard->p[23] == EMPTY && blackcanmoveto(xb, 25))) ret++;

   return ret;
}