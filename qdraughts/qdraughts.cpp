/* 
 * File:   qdraguhts.cpp
 * Author: Kenneth Bastian
 *
 * The code is derived from Mark Lynch's Neurodraughts
 *
 * Run on command prompt using the following command:
 * qdraughts.exe TEST.CRT TEST.PRM TEST.BRD OPP1.FTR OPP1.NET 10 10
 */

#include "qdraughts.h"

int main(int argc, char** argv) {
	//general bits
time_t 	s,e;
char buf[64];

//the networks
DirectNet*		opp1, *opp2;
DraughtsGame*	game;
//important numbers
float	lr1, lr2, momentum;
float	gamma, lambda;
int 	testafter, gamelimit;

//network dimensions
int 	numinputs, numhidden;
bool	directlinks;
unsigned int netseed;
float 	lowr, highr;
char	netname[64];

//tournament numbers
int	clonenum, matchno;

//take time
s = time(NULL);

cout<<"\n\tNeuroDraughts, (c) 1997 Mark Lynch.";
cout<<"\n\tQ-draughts, (c) 2011 Kenneth Bastian.\n\n";

//check input
if(argc != 8){
   	cerr<<"Incorrect Parameters\n Usage: "<<argv[0]<<" createfile, paramfile, boardfile, featurefile, savegamefile, testafter, gamelimit\n";
    exit(-1);
}

//the files
char	*createfile, *boardfile, *paramfile, *savegamefile, *featurefile;
ifstream boardf, createf, paramf;

//assign pointers
createfile = argv[1];
paramfile  = argv[2];
boardfile  = argv[3];
featurefile= argv[4];
savegamefile=argv[5];
testafter  = atoi(argv[6]);
gamelimit  = atoi(argv[7]);

//need to get networks first.
createf.open(createfile);
if(!createf){
   	cerr<<"Error opening Net Creation File: "<<createfile<<"\n";
    exit(-1);}
	
createf>>buf;
//is it being created from scratch... or are we loading?
if(buf[0] == 'N'){
   	cout<<"Creating Network from:\t\t"<<createfile<<endl;
   	createf>>numinputs>>buf>>numhidden>>buf>>directlinks;
    createf>>buf>>netseed>>buf>>lowr>>buf>>highr>>buf>>netname;
    opp1 = new DirectNet(numinputs, numhidden, directlinks, 433920, lowr, highr, 1, netname, featurefile,1);
    //alter name of 2nd network
	strcat(netname, " - Clone");
    opp2 = new DirectNet(numinputs, numhidden, directlinks, 400168, lowr, highr, 1, netname, featurefile,1);
    createf.close();
}
else{
    createf.close();
   	cout<<"Loading Network from:\t\t"<<createfile<<endl;
   	opp1 = new DirectNet(createfile, featurefile, 1);
    //alter name of 2nd network
    opp1->GetName(netname);
    strcat(netname, " - Clone");
    opp2 = new DirectNet(createfile, featurefile, 1);
    opp2->SetName(netname);}

///read in all the parameters.
paramf.open(paramfile);
if(!paramf){
   	cerr<<"Error opening Parameter File: "<<paramfile<<"\n";
    exit(-1);
}

//inform what we're doing
cout<<"Loading Parameters from:\t"<<paramfile<<endl;
cout<<"Loading Board Data from:\t"<<boardfile<<endl;
cout<<"Saving Test Games to:\t\t"<<savegamefile<<endl;
cout<<"Limit of "<<gamelimit<<" sets of "<<testafter<<" games. Total of "<<testafter*gamelimit<<" training games.\n\n";

//sort out learning rates
paramf>>buf>>lr1;
if(lr1 == 0.0) //if zero use default
   lr1 = 1.0/opp1->NumInputs();
paramf>>buf>>lr2;
if(lr2 == 0.0)
   lr2 = 1.0/opp1->NumHidden();

opp1->SetLRate1(lr1);
opp1->SetLRate2(lr2);
opp2->SetLRate1(lr1);
opp2->SetLRate2(lr2);

//sort out momentum term
paramf>>buf>>momentum;
opp1->SetMomentum(momentum);
opp2->SetMomentum(momentum);

//TD parameters
paramf>>buf>>gamma>>buf>>lambda;
paramf.close();

//create the tournament;
game = new DraughtsGame();
clonenum = 0;

//the tournament running loop!
for (matchno = 1; matchno<=gamelimit; matchno++){
   	//local variables
    int 		pointstoclone, won, boards ,t, j, p;
    BOARD 	gameboard;
    float		drawb, drawr;

    //open board file
    boardf.open(boardfile);
 	if(!boardf){
	   	cerr<<"Error opening board file: "<<boardfile<<"!!!\n";
        exit(-1);
    }
    cout<<"Match Number "<<matchno<< " commencing...\n";

    won = 1;

    //how many boards in file, reads the first integer in .BRD file
    boardf>>boards;
	
    for(j=0; j<boards; j++){
     	//read in board
 		gameboard.p[0] = INVALID;
		for(p=1; p<=32; p++){
				boardf>>t;
				gameboard.p[p] = t;
		}
        //read extra parameters
        boardf>>drawb>>drawr>>pointstoclone;
     	game->AutoPlay(NULL, TRAIN_OPP1, testafter, &gameboard, gamma, lambda, opp1, opp2, drawb, drawr);
	    int wins = game->AutoPlay(savegamefile, NULL, 2, &gameboard, 0.0, 0.0, opp1, opp2, 0.0, 0.0);
		//if(wins<pointstoclone && won == 1) 
		//	won = 0;
		}
		boardf.close();

		//opp1 is the winner 90% win ratio
		/*if(won){
			char fname[64];
			sprintf(fname, "clone%d.net",++clonenum);
			cout<<"\n\t***Clone "<<clonenum<<" after "<<matchno<<" matches***\n\n";
			//swap the weights
			opp1->SaveNet(fname);
			opp2->LoadWeights(fname);
		}
		else{
       		if(matchno % 10 ==0 ){
         		char fname[64];
				sprintf(fname, "save%d.net",matchno);
          		opp1->SaveNet(fname);
			}
		}*/
	}

	//make a final copy of the nets
	opp1->SaveNet("Final1.net");
	opp2->SaveNet("Final2.net");

	//output some info
	e=time(NULL);
	cout<<"************************ Finished *******************************\n";
	cout<<"Time elapsed for "<<matchno<<" sets of matches: "<<difftime(e,s)<<endl;
	cout<<"Average of "<<difftime(e,s)/matchno<<" secs per "<<testafter<<" games, ";
	cout<<difftime(e,s)/matchno/testafter<<" per game\n";
	//cout<<"Network was Cloned: "<<clonenum<<" times.\n";
	//cleanup
	free(opp1);
	free(opp2);
	free(game);

	getchar();
    return (EXIT_SUCCESS);
}

