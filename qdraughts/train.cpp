#include "train.h"

INPUTTYPE 	fillbuf(ifstream *gf, char* buf);


/* takes the filename of precomputed training data and trains a net with it */
float TrainfromFile(DirectNet* dnet, char* fname, float gamma, float lambda)
{
	ifstream bf;
   char 		buf[128];
   int 		whowon, rmoves, bmoves;
   int 		i,j;
   int temp;
   BOARD		b;
   QBOARD qb;
   float		err, maxerr=0.0;

   bf.open(fname);
   if(!bf)
   {
   	cerr<<"Error opening training data.\n";
      return -1;
   }
   while(!bf.eof())
	{
	   //read in details first
   	bf>>buf>>buf>>buf;

      if(bf.eof())
      	break;

      bf>>buf>>whowon;
	   bf>>buf>>bmoves;
		bf>>buf>>rmoves;


      //just to keep track of training!
//      if (whowon == BLACKWON)		cout<<"Black won in: "<<bmoves<<" moves\n";
//      else if(whowon == REDWON)	cout<<"Red won in: "<<bmoves<<" moves\n";
//      else 								cout<<"Nobody won after: "<<bmoves<<" moves\n";

		cout<<".";

      for(i=0; i<bmoves; i++)
      {
      	for(j=1; j<=32; j++)
         {
           	bf>>temp;
            b.p[j]=temp;
	      }
         if(i==0)	   	dnet->InitTDTrain(&b, gamma, lambda,0,&qb);
			else				dnet->TDTrain(&b,0,&qb);
      }
      if(whowon == BLACKWON)	dnet->TDFinal(WINVALUE);
      else if(whowon == REDWON)	dnet->TDFinal(LOSEVALUE);
      else	dnet->TDFinal(DRAWVALUE);

      err=dnet->getmaxerr();
      if(err>maxerr) maxerr = err;

      //read in boards(red)

      for(i=0; i<rmoves; i++)
      {
      	for(j=1; j<=32; j++)
         {
         	bf>>temp;
           	b.p[j]=temp;
         }
         if(i==0) dnet->InitTDTrain(&b, gamma, lambda,0,&qb);
         else     dnet->TDTrain(&b,0,&qb);
      }
      if(whowon == BLACKWON)	dnet->TDFinal(LOSEVALUE);
      else if(whowon == REDWON)	dnet->TDFinal(WINVALUE);
      else	dnet->TDFinal(DRAWVALUE);

		err =dnet->getmaxerr();
      if(err>maxerr) maxerr = err;


   }
   bf.close();
   cout<<"\n";
   return maxerr;

}


/* it converts the games to lists of MOVE structs and then to lists of BOARD structs which is passes
	to WriteBoards so that it can be used for future training
   */

void PDNtoBoards(char* fname, char *bfile, int ignore_draws)
{
	MOVE** movelist=NULL;
 	char buf[256]="\0";
	ifstream gf;
   fstream bf;
   INPUTTYPE whowon,r;
   char* t;
   int i,moves;
   int finished =0;
   int finishedfile=0;

   gf.open(fname);
   if(!gf)
   {
   	cerr<<"Error opening PDN file\n";
   	return;
   }

	while(!finishedfile)
   {
	   moves=0;
	   finished=0;
	   while(finished==0)
	   {
			gf>>buf;
	      if(gf.eof())
	      {
            finishedfile=1;
            break;
	      }

	      if(strcmp(buf, "1.") == 0)
	      {
	      	for(i=strlen(buf)-1; i>=0;i--)
		      	gf.putback(buf[i]);
				finished=1;
	      }
	   }
	   finished =0;

	   //parse file...
	   while(!finished && !finishedfile)
	   {
	      r = fillbuf(&gf, buf);

	      if (r <= DRAW)
	      {
	      	finished = 1;
	         whowon = r;
	      }
	      else if (r == BLACKMOVE || r == REDMOVE)
	      {
				moves++;
	         if((movelist=(MOVE**)realloc(movelist, (sizeof(MOVE*))*moves))==NULL)
	         	cerr<<"no memory lefT???\n";
	         if((movelist[moves-1]=(MOVE*)malloc(sizeof(MOVE)))==NULL)
	         	cerr<<"nomem .. doh!\n";
	         movelist[moves-1]->exchange = 0;
	         if((t=strchr(buf, '-'))==NULL)
	         {
	         	t=strchr(buf,'x');
	         	movelist[moves-1]->exchange = 1;
	         }
	         movelist[moves-1]->from = atoi(buf);
	         movelist[moves-1]->to = atoi(t+1);
	      }
	   }
      //map moves to boards and write out to file
      if(moves >0)
      {
      	if(!(ignore_draws && whowon == DRAW))
		      WriteBoards(bfile, movelist, moves, whowon);

			//destroy it!
		   for (i = 0; i<moves; i++)
		   	free(movelist[i]);
      }
   }
   free(movelist);
}

/* update 8/04/97 - board feeding changed - 1st moved board now goes to red! */
void WriteBoards (char* bfile, MOVE** movelist, int moves, INPUTTYPE whowon)
{
   fstream 	bf;
   BOARD**	blacklist;
   BOARD**	redlist;
   XBOARD	xb,temp;
   int		i,j;
   int 		bmoves, rmoves;

	//map list of moves to a list of boards!
   //how many boards each?
   if((moves % 2) == 0)
   	rmoves = bmoves = moves/2;
   else
   {
	   rmoves = moves/2;
   	bmoves = rmoves+1;
   }
   //allocate memory for both lists
   blacklist = (BOARD**)calloc(sizeof(BOARD*), bmoves);
   if(blacklist == NULL)
   	cerr<<"no mem!\n";
   for(i=0; i<bmoves; i++)
   {
    	if((blacklist[i] = (BOARD*)malloc(sizeof(BOARD)))==NULL)
      	cerr<<"no memory!\n";
   }

	redlist = (BOARD**)calloc(sizeof(BOARD*), rmoves);
   if(redlist == NULL)
   	cerr<<"no mem!\n";
   for(i=0; i<rmoves; i++)
   {
    	if((redlist[i] = (BOARD*)malloc(sizeof(BOARD)))==NULL)
      	cerr<<"no memory!\n";
   }

   //init xboard
   initxboard(&xb);

   for (i =0; i<moves; i++)
   {
      //need this for calculating jumps!
      maptoxmove(movelist[i]);

   	//apply the move
      if(movelist[i]->exchange ==1)
        	applyexchange(&xb, movelist[i]->from, movelist[i]->to);
      else
      {
      	applymove(&xb, movelist[i]->from, movelist[i]->to );
      }

      if(i==0 || (i % 2)==0)
	      maptoboard(blacklist[(i/2)], &xb );
      else
      {
      	copyboard(&temp, &xb);

      	//mirror it - reverse the alignment and then add it!
      	mirrorboard(&temp);
         reversealign(&temp);
         maptoboard(redlist[(i/2)], &temp);
      }
	}
   bf.open(bfile, ios::app);
   if(!bf)
   {
   	cerr<<"Error opening output file\n";
      return;
   }
	//write out details first
   bf<<"\nNeuroDraughts Training Board!\n";
   bf<<"Who_Won? "<<whowon<<endl;
   bf<<"Number_of_Black_Boards: "<<bmoves<<endl;
   bf<<"Number_of_Red_Boards:   "<<rmoves<<endl;
   //write out black boards first followed by red boards
   for(i=0; i <bmoves; i++)
   {
   	for(j=1; j<=32; j++)
      {
      	bf<<blacklist[i]->p[j]<<" ";
      }
      bf<<endl;
   }
   bf<<endl;
   for(i=0; i <rmoves; i++)
   {
   	for(j=1; j<=32; j++)
      {
      	bf<<redlist[i]->p[j]<<" ";
      }
      bf<<endl;
   }
   bf.close();

   //free memory for board
   for(i=0; i<bmoves; i++)
   {
   	free(blacklist[i]);
   }
   for(i=0; i<rmoves; i++)
   {
   	free(redlist[i]);
   }
   free(blacklist);
   free(redlist);
}


/* works out what it got -must be altered to allow moves to be diff */
INPUTTYPE fillbuf(ifstream* gf, char* buf)
{
	INPUTTYPE ret;

   ofstream df;

	*gf>>buf;

	//end of game check
   if(strcmp(buf,"1-0")==0)
     	ret = BLACKWON;
   else if(strcmp(buf,"0-1")==0)
     	ret= REDWON;
   else if(strcmp(buf,"1/2-1/2")==0)
	  	ret = DRAW;
	else
   {
 		//the game isn't over - comment check
   	if(buf[0] == '{')
      {
      	//get rid of the thing!
         //make sure the last char isn't the comment finish!!!
         if(buf[strlen(buf)-1] != '}')
			{
	      	gf->get(buf, 256, '}');
	         gf->read(buf, 1);
	         buf[1]=NULL;
         }

	      ret = COMMENT;

      }
      //new move-black first
      else if ((buf[1] == '.') || (buf[2] == '.') || (buf[3] == '.'))
      {
			*gf>>buf;
         ret = BLACKMOVE;
      }
      else
      {
      //it must be a red move... i hope.
      	ret = REDMOVE;
      }
   }
   return ret;
}
