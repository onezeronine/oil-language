/* 
 * File:   dirnet.cpp
 * Author: Kenneth Bastian
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */

#include "dirnet.h"

/* typeofnet specifies the type of mapping to be used , the rest map directly to their evalnet counterparts */
DirectNet::DirectNet(int typeofnet, int numhidden, bool dlinks, unsigned seed,
							float lowrange, float highrange, float bias, char* n, char* featuref,int q){
	int numinputs;
	whatami = typeofnet;
	//if qlearn = 1, use q-learning
	if(q){
		qlearn=1;
	}

	//depending on typeofnet, allocate correct inputs
	if(typeofnet == NET_BINARYMAP){
   		numinputs = 96;
	}
	else if(typeofnet == NET_DIRECTMAP){
   		numinputs = 4;
	}
	else
	{
  	   //sort out features
	   numinputs = SetupFeatures(featuref);
	}

	if(qlearn==1){
		numinputs = 256;
		qnet = new QNet(numinputs,numhidden, dlinks, seed, lowrange, highrange, bias, n);
		//qv is now one-dimensional array - September 1, 2011
		qv = floatalloc(qnet->NumInputs()+1);
	}
	else{
		net = new EvalNet(numinputs,numhidden, dlinks, seed, lowrange, highrange, bias, n);
		//allocate an input vector - not forgetting bias unit!
	//iv is the inputvector passed to evalnet
		iv = floatalloc(net->NumInputs()+1);
	}
	
}

/* load from file constructor */
DirectNet::DirectNet(char* fname, char* featuref, float bias)
{
	int numinputs, numbits;

	//load it up
 	net = new EvalNet(fname, bias);

	//how many inputs
	numinputs = net->NumInputs();

	//set what it is
	if(numinputs == 96)
		whatami = NET_BINARYMAP;
	else if(numinputs == 32)
   		whatami = NET_DIRECTMAP;
	else
   		whatami = NET_FEATUREMAP;

	if(whatami == NET_FEATUREMAP)
   		numbits = SetupFeatures(featuref);

	if(numbits != numinputs)
   		cerr<<"\tDiffering Number of Inputs!!!\nFeature Definition File and Network File do not match!\n";
   
	//allocate an input vector - not forgetting bias unit!
	iv = floatalloc(net->NumInputs()+1);
	
}

int DirectNet::SetupFeatures(char* featurefile)
{
	//feature names
	char* featurelist[NUMOFFEATURES] = {"PieceAdvantage", "PieceDisadvatange",
               "PieceThreat", "PieceTake", "Advancement", "DoubleDiagonal",
               "BackrowBridge", "CentreControl", "XCentreControl",
          "TotalMobility", "Exposure", "KingCentreControl", "DiagonalMoment"};

 	//setup features
	feature_funcs[0] = pieceadvantage;
	feature_funcs[1] = piecedisadvantage;
	feature_funcs[2] = piecethreat;
	feature_funcs[3] = piecetake;
	feature_funcs[4] = advancement;
	feature_funcs[5] = doublediagonal;
	feature_funcs[6] = backrowbridge;
	feature_funcs[7] = centrecontrol;
	feature_funcs[8] = xcentrecontrol;
	feature_funcs[9] = totalmobility;
	feature_funcs[10] = exposure;
	feature_funcs[11] = kingcentrecontrol;
	feature_funcs[12] = diagonalmoment;
	feature_funcs[13] = calculatenextbit;

	char	feature[64];
	int	numofbits,i,j;
	ifstream ff;

	//keep track of use
	for(i=0; i<NUMOFFEATURES; i++)
   		bitstouse[i] = 0;

	numfound = 0;

	//down to business
	ff.open(featurefile);
	ff>>feature>>numofbits;
	while(numofbits == 0)
   		ff>>feature>>numofbits;

	//do the entire file
	while(!ff.eof())
	{
   		//find the feature!
   		for(i=0; i<NUMOFFEATURES; i++)
      		if(!strcmp(featurelist[i],feature))
			{
				numfound+=numofbits;
         		break;
			}
		//store result;
		bitstouse[i] = numofbits;

		numofbits = 0;
		//get next non zero feature
		while(numofbits == 0 && ff)
	   		ff>>feature>>numofbits;
	}
	ff.close();

   /* we now have an index of features and how many bits to allocate to each
   	aswell as knowing how many inputs to allocate

 	now that we know this we can set up a list of functions to be called
   in a row so that no more calculation is required at the evaluation state */

   //allocate array of function indexes
	input_map = (ffunc*)calloc(numfound, sizeof(ffunc));

	int currentbit = 0;
	for(i=0; i<NUMOFFEATURES; i++)
	{
   		if(bitstouse[i] >= 1)
		{
      		input_map[currentbit++] = feature_funcs[i];
			for(j = 2; j<=bitstouse[i]; j++)
         		input_map[currentbit++] = calculatenextbit;
		}
	}
	return numfound;
}

DirectNet::~DirectNet()
{
	delete net;
  	free(input_map);
	free(iv);
	free(qv);
}

/* Init TD Train: Sets up the entire process of TD training */
void DirectNet::InitTDTrain(BOARD* pboard, float gamma, float lambda, int swapboard, QBOARD* qmapboard)
{
	XBOARD xb;
	BOARD b;

	//if the board needs swapping
	if(swapboard == 1)
	{
   		maptoxboard(pboard, &xb);
   		mirrorboard(&xb);
		reversealign(&xb);
		maptoboard(&b, &xb);
		MapQActions(&b,qmapboard);
		mapfeatures(&b,qmapboard);
	}
	else
		mapfeatures(pboard,qmapboard);

	if(qlearn==1)
		qnet->InitQTrain(qv,gamma,lambda);
	else
		net->InitTDTrain(iv, gamma, lambda);
}

/* TDTrain: Training of intermediate boards */
void DirectNet::TDTrain(BOARD* pboard, int swapboard, QBOARD* qmapboard)
{
	XBOARD xb;
	BOARD b;

	//if the board needs swapping
	if(swapboard==1)
	{	
   		maptoxboard(pboard, &xb);
   		mirrorboard(&xb);
		reversealign(&xb);
		maptoboard(&b, &xb);
		MapQActions(&b,qmapboard);
  		mapfeatures(&b,qmapboard);
	}
	else{
		mapfeatures(pboard,qmapboard);
	}
	if(qlearn==1)
		qnet->QTrain(qv);
	else
		net->TDTrain(iv);
}

/* TDFinal: Give final reward to the network */
void DirectNet::TDFinal(float evalue)
{	
	if(qlearn==1)
		qnet->QFinal(evalue);
	else
		net->TDFinal(evalue);
}


/* Evaluate A Direct Network. Maps board and calls EvaluateNet */
float DirectNet::EvaluateNet(BOARD* pboard,QBOARD* qmapboard)
{
	float	 f;


	mapfeatures(pboard, qmapboard);
	
	if(qlearn==1)
		f = qnet->EvaluateNet(qv);
	else
		f = net->EvaluateNet(iv);

	return f;
}

/* mapfeatures: This function, called by ComputeIV performs one of three
	feature mappings, direct, binary or feature mapping. All persuasions
   are catered for :)
*/
void DirectNet::mapfeatures(BOARD* pboard, QBOARD* qmapboard)
{
	int i,j,inputs=1;
	if(qlearn==1){
		//direct mapping of Q-values - September 3, 2011
		//binary mapping of Q-values - September 4, 2011
		for(int i=1;i<33;i++){
			for(int j=1;j<=8;j=j*2){		
				qv[inputs] = qv[inputs+1] = 0;
				if(qmapboard->p[i][j] == 1){
					qv[inputs] = 0;
					qv[inputs+1] = 1;
				}
				else if(qmapboard->p[i][j] == 2){
					qv[inputs] = 1;
					qv[inputs+1] = 0;
				}
				else if(qmapboard->p[i][j] == 3){
					qv[inputs] = 1;
					qv[inputs+1] = 1;
				}
				inputs=inputs+2;
			}
		}
	}

	else{
		//what kind of mapping do we perform?
		if(whatami == NET_BINARYMAP)
		{	
			for (i=0; i<32; i+=3)
			{
				iv[i] = iv[i+1] = iv[i+2] = OFFVALUE;
	   			if(pboard->p[i+1] == BLACKMAN)
					iv[i+2]=ONVALUE;
				else if(pboard->p[i+1] == REDMAN)
	      			iv[i+1]=ONVALUE;
				else if(pboard->p[i+1] == BLACKKING)
	      			iv[i+1] = iv[i+2] = ONVALUE;
				else if(pboard->p[i+1] == REDKING)
	      			iv[i] = ONVALUE;

			}
		}
	
		//direct mapping is unuseful - August 29, 2011
		//direct mapping is now Q-mapping - August 30, 2011
		//Back to direct mapping, creating new mapping function is necessary - August 31, 2011
		else if(whatami == NET_DIRECTMAP)
		{	
			for(i=0; i<32; i++)
      		iv[i] = pboard->p[i+1] / 4;
		}
		//must be feature mapping
		else
		{
   			for(i=0; i<numfound; i++)
      			iv[i] = input_map[i](pboard);
		}
	}
}

//saves the net
void DirectNet::SaveNet(char* fname)
{
	if(qlearn==1)
		qnet->SaveNet(fname);
	else
		net->SaveNet(fname);
}

//loads the net
void DirectNet::LoadWeights(char* fname)
{
	if(qlearn==1)
		qnet->LoadWeights(fname);
	else
		net->LoadWeights(fname);
}

//error functions, for plotting graphs etc.
float	DirectNet::getmaxerr(void)
{
	if(qlearn==1)
		return qnet->getmaxerr();
	else
		return net->getmaxerr();
}
float DirectNet::getavgerr(void)
{

	if(qlearn==1)
		return qnet->getavgerr();
	else
		return net->getavgerr();
}

//who am i?
void DirectNet::GetName(char* name)
{	
	if(qlearn==1)
		qnet->GetName(name);
	else
		net->GetName(name);
}

void DirectNet::SetName(char* name)
{	
	if(qlearn==1)
		qnet->SetName(name);
	else
		net->SetName(name);
}

/* floatalloc accepts size of float array to be created and return pointer to it */
float* DirectNet::floatalloc(size_t size)
{
	float* ret;

	if ( (ret = (float*) calloc(size, sizeof(float))) == NULL )
	{
   		cout<<"Not enough memory!!!";
		exit(-1);
	}
	return ret;
}

//change parameters on the fly
void DirectNet::SetBias(float bias)
{
	if(qlearn==1)
		qnet->SetBias(bias);
	else
		net->SetBias(bias);
}
void DirectNet::SetLRate1(float lrate)
{
	if(qlearn==1)
		qnet->SetLRate1(lrate);
	else
		net->SetLRate1(lrate);
}
void DirectNet::SetLRate2(float lrate)
{
	if(qlearn==1)
		qnet->SetLRate2(lrate);
	else
		net->SetLRate2(lrate);
}
void DirectNet::SetMomentum(float mom)
{	
	if(qlearn==1)
		qnet->SetMomentum(mom);
	else
		net->SetMomentum(mom);
}

//get dimensions
int DirectNet::NumInputs()
{
	if(qlearn==1)
		return qnet->NumInputs();
	else
		return net->NumInputs();
}
int DirectNet::NumHidden()
{
	if(qlearn==1)
		return qnet->NumHidden();
	else
		return net->NumHidden();
}

int DirectNet::GetQLearn(){
	return qlearn;
}

//This should be called carefully, buggy function - September 8, 2011
float DirectNet::EvaluateQ(QBOARD* qmapboard){
	float	 f;

	mapfeatures(NULL, qmapboard);
	
	if(qlearn==1)
		f = qnet->GetHighestWeights(qv);
	

	return f;
}
