/* 
 * File:   evalnet.cpp
 * Author: Rarepon
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */


#include "qnet.h"

#define TANH

#ifdef TANH1
#define SQUASH(x) (float) ((2.0 / (1.0 + exp(-2.0*(double)x))) - 1.0)
#define DERIV(x)  (float) ((1.0 + (double)x) * (1.0 - (double)x))
#elif defined TANH
#define SQUASH(x) (float) (tanh((double)x))
#define DERIV(x)  (float) (1.0 - ((double)x*(double)x))
#endif




/* The Destructor simply frees the memory allocated to the weights */

QNet::~QNet()
{
	int i;

	//free weights1 - oldweights1 and elig1
	for (i = 0; i <= numinputs;  i++)
	{
       free(weights1[i]);
       free(oldweights1[i]);
       free(elig1[i]);
	}

	free(weights1);
	free(oldweights1);
	free(elig1);

	//free up weights2 - oldweights2 & elig2
	free(weights2);
	free(oldweights2);
	free(elig2);

	//free up weights3 - oldweights3 & elig3
	free(weights3);
	free(oldweights3);
	free(elig3);

	//free up hidden unit storage
	free(hiddenunits);
}


/* The constructor allocates memory for the Net aswell as initialising all
	the weights to a random value between lowrange and highrange. It also
   sets up the debug file if any. */
QNet::QNet(int ninputs, int nhidden, bool dlinks, unsigned theseed, float lowrange, float highrange, float bias, char* n)
{
   SetBias(bias);

   numinputs = ninputs;
   numhidden = nhidden;
   directlinks = dlinks;
   seed = theseed;

   //set the name
   strcpy(name,n);

   //allocate all the memory required!
   AllocateMem();

   InittoRandom(lowrange, highrange);
}

/* The constructor allocates memory for the Net aswell as reading in the
	the weights from the file specified. */
QNet::QNet(char* loadfile, float bias)
{
   ifstream nf;			//load the network from this file

   SetBias(bias);

   nf.open(loadfile);
   if(!nf)
   {
   	cerr<<"Error opening "<<loadfile<<" for input!";
      return;
   }
   //get name
   nf.getline(name, sizeof(name));

   //get dimensions
   nf>>numinputs>>numhidden>>directlinks; //add error check later!

   //allocate all the memory required!
   AllocateMem();
   nf.close();

   LoadWeights(loadfile);
}

/* Load Weights takes a filename.*/
void QNet::LoadWeights(char* loadfile)
{
	float dummy;
   char buf[64];
   ifstream nf;
   int i,j;

	//open file and load weights
   nf.open(loadfile);
	if(!nf)
   {
   	cerr<<"Error opening "<<loadfile<<" for input!\n";
    	return;
   }
   //skip the name
   nf.getline(buf, sizeof(buf));

   //we must skip the dimensions as space is already there (should be!)
   nf>>dummy>>dummy>>dummy;

   //initialize weights of input - hidden links
   for (i=0; i<=numinputs; i++)
   {
   	for (j=0; j<numhidden; j++)
      {
         nf>>weights1[i][j];
         oldweights1[i][j] = 0.0;
      }
   }
   //initialize weights of hidden - output links
   for (i=0; i<=numhidden; i++)
   {
     	nf>>weights2[i];
      oldweights2[i] = 0.0;
   }
	//init weights if in - out links
   if(directlinks)
   {
	   for (i=0; i<=numinputs; i++)
	   {
	     	nf>>weights3[i];
	      oldweights3[i] = 0.0;
	   }
   }
   nf.close();
}

void QNet::GetName(char* n)
{
	strcpy(n, name);
}

void QNet::SetName(char* n)
{
	strcpy(name, n);
}


/* Initialises the TDTrain variables for incremental training */
void QNet::InitQTrain(float* qv, float gamma, float lambda)
{
	InitElig();
	InitMomentum();
	InitError();

	SetDecay(gamma);
	SetLambda(lambda);

	oldoutput = EvaluateNet(qv);
	UpdateElig(qv);
}


/* Trains the net based on error between previous output and current output and reward */
void QNet::QTrain(float* qv)
{
	EvaluateNet(qv);
	outerror = (GAMMA*outputunit)-oldoutput;

	//keep track of error
	if(fabs(outerror) > maxerr)	maxerr = fabs(outerror);
	avgerr += outerror;

	UpdateWeights();

	//needed 2nd time for TD errors
	oldoutput = EvaluateNet(qv);
	UpdateElig(qv);
}

void QNet::QFinal(float evalue)
{
	outerror = evalue - oldoutput;

	//keep track of error
	if(fabs(outerror) > maxerr)	maxerr = fabs(outerror);
	avgerr += outerror;

	UpdateWeights();
}

void QNet::Report()
{
	fstream fp;

	fp.open("report.txt", ios::app);
	if(!fp)
   		cerr<<"Error openin report file!\n";

	fp<<"\nThe outerror is: "<<outerror<<endl;

	fp.close();
}

/* EvaluateNet takes a pointer to an array of floats as it's argument.
	It assumes that the inputvector size corresponds to the numinputs supplied
   when the net was created. */
float QNet::EvaluateNet(float* qvector)
{
	int i,j;

	//set the BIAS
	qvector[numinputs] = BIAS;
	hiddenunits[numhidden] = BIAS;

	//calculate hidden values
	for (i=0; i<numhidden; i++)
	{
		hiddenunits[i] = 0.0;
		//each input (not forgetting BIAS unit)
		for (j=0; j<=numinputs; j++)
		{
      		hiddenunits[i] += weights1[j][i] * qvector[j];
		}

		//no longer sigmoid - now hyperbolic tangent (tanh) range -1 to +1
		
		hiddenunits[i] = SQUASH(hiddenunits[i]);
	}

	//calculate output value- again not forgetting BIAS
	outputunit = 0.0;
	for (i=0; i<=numhidden; i++)
   		outputunit += weights2[i] * hiddenunits[i];

	//in-out links
	if(directlinks)
	{
		for (i=0; i<=numinputs; i++)
	   		outputunit += weights3[i] * qvector[i];
	}

	//no longer sigmoid - now hyperbolic tangent (tanh) range -1 to +1
	outputunit = SQUASH(outputunit);
	return (outputunit);
}

/* updates the weights based on eligibilities and outerror - uses momentum*/
void QNet::UpdateWeights()
{
	int i,j;
	float temp;

	if(directlinks)
	{
		//update in-out weights
		for (i=0; i<=numinputs; i++)
		{
	   		temp = LRATE1 * outerror * elig3[i];
			if((oldweights3[i] > 0 && temp > 0) || (oldweights3[i] < 0 && temp <0 ))
	      		oldweights3[i] = (temp)+ (MOMENTUM * oldweights3[i]);
			else
	      		oldweights3[i] = temp;
	   		weights3[i] = weights3[i] + oldweights3[i];
   		}
	}

	//update hid-out weights
	for (i=0; i<=numhidden; i++)
	{
   		temp = LRATE2 * outerror * elig2[i];
		if((oldweights2[i] > 0 && temp > 0) || (oldweights2[i] < 0 && temp <0 ))
      		oldweights2[i] = (temp)+ (MOMENTUM * oldweights2[i]);
		else
      		oldweights2[i] = temp;
   		weights2[i] = weights2[i] + oldweights2[i];
	}
	for (i=0; i<numhidden; i++)
	{
		//update in - hid weights
		for (j=0; j<=numinputs; j++)
		{
      		temp = LRATE1 * outerror * elig1[j][i];
  			if((oldweights1[j][i] > 0 && temp > 0) || (oldweights1[j][i] < 0 && temp <0 ))
	      		oldweights1[j][i] = temp+ (MOMENTUM * oldweights1[j][i]);
			else
	      		oldweights1[j][i] = temp;
      		weights1[j][i] = weights1[j][i] + oldweights1[j][i];
		}
	}
}

/* updates the eligibility trace of the network - basically a running sum of the derivatives */
void QNet::UpdateElig(float* qv)
{
	int 	i,j;
	float deriv;

  	deriv=DERIV(outputunit);

	if(directlinks)
	{
		for (i=0; i<=numinputs; i++)
		   elig3[i] = (LAMBDA*elig3[i]) + (deriv*qv[i]);
	}

	for (i=0; i<=numhidden; i++)
	    elig2[i] = (LAMBDA*elig2[i]) + (deriv*hiddenunits[i]);

	for (i=0; i<numhidden; i++)
		for (j=0;j<=numinputs;j++)
			elig1[j][i]= (LAMBDA * elig1[j][i]) + (deriv * weights1[j][i] * DERIV(hiddenunits[i]) * qv[j]);
}



/* save the net to a file */
void QNet::SaveNet(char* savefile)
{
	int i,j;
 	ofstream sf;

   sf.open(savefile);
   if(!sf)
   {
   	cerr<<"Cannot open "<<savefile<<" for output!\n";
      return;
   }
	//save name!
   sf<<name<<endl;

   //save dimensions first
   sf<<numinputs<<" "<<numhidden<<" "<<directlinks<<endl;

   //save weights of in - hidden
   for (i=0; i<=numinputs; i++)
 	{
   	for (j=0; j<numhidden; j++)
      	sf<<weights1[i][j]<<"\t";
      sf<<endl;
   }
   //save hid - out
   sf<<endl;
   for (i=0; i<=numhidden; i++)
   	sf<<weights2[i]<<"\t";

   if(directlinks)
   {
      sf<<endl;
	   for (i=0; i<=numinputs; i++)
	   	sf<<weights3[i]<<"\t";
   }

}

/* Initialises net's weights to a random number based on the seed */
void QNet::InittoRandom(float lowrange, float highrange)
{
	int i,j;
	int	irange;		//integer range for random numbers
	float rfloat;

	//have to change to int for random number gen
	irange = (highrange*1000) - (lowrange * 1000);

	srand(seed);

	//initialize weights of input - hidden links
	for (i=0; i<=numinputs; i++)
	{
   		for (j=0; j<numhidden; j++)
		{
			rfloat = this->getrealrandom(irange);
      		weights1[i][j] = lowrange + rfloat;
		}
	}

	//initialize weights of hidden - output links
	for (i=0; i<=numhidden; i++)
	{
		rfloat = getrealrandom(irange);
     	weights2[i] = lowrange + rfloat;
	}

	//initialize weights of in - output links
	if(directlinks)
	{
		for (i=0; i<=numinputs; i++)
		{
			rfloat = getrealrandom(irange);
	     	weights3[i] = lowrange + rfloat;
		}
	}
}

void QNet::InitError()
{
	maxerr = avgerr = 0.0;
}

/* Initialises net's momentum arrays */
void QNet::InitMomentum()
{
	int i,j;

   //initialize weights of input - hidden links
   for (i=0; i<=numinputs; i++)
   	for (j=0; j<numhidden; j++)
         oldweights1[i][j] = 0.0;
   //initialize weights of hidden - output links
   for (i=0; i<=numhidden; i++)
      oldweights2[i] = 0.0;

   //initialize weights of in - output links
   if(directlinks)
   {
	   for (i=0; i<=numinputs; i++)
	      oldweights3[i] = 0.0;
   }
}
/* initliases the eligibility trace */
void QNet::InitElig()
{
	int i,j;

   for (i=0; i<=numinputs; i++)
   	for(j=0; j<numhidden; j++)
         elig1[i][j] = 0.0;

   for (i=0; i<=numhidden; i++)
   	elig2[i] = 0.0;

   if(directlinks)
   {
	   for (i=0; i<=numinputs; i++)
	   	elig3[i] = 0.0;
   }
}

/* Allocate memory allocates the correct amount of storage for all weight arrays
	and other storage needed - called by both constructors */
void QNet::AllocateMem()
{
	int i,dir;

	//allocate space for input - hidden weights
	weights1 = 	 	pfloatalloc(numinputs+1);
	oldweights1 = 	pfloatalloc(numinputs+1);

   for (i=0; i <= numinputs; i++)
   {
		weights1[i] = 		floatalloc(numhidden);
		oldweights1[i] = 	floatalloc(numhidden);
   }

   //allocate space for hidden - output (single) weights
   weights2 = 		floatalloc ( numhidden+1 );
   oldweights2 = 	floatalloc ( numhidden+1 );

   //allocate space for in - output (single) weights
   weights3 = 		floatalloc ( numinputs+1 );
   oldweights3 = 	floatalloc ( numinputs+1 );

   //allocate space for hidden units activations
	hiddenunits = floatalloc (numhidden+1);

   //space for eligibilities
   elig1	= pfloatalloc(numinputs+1);
   for (i=0; i<= numinputs; i++)
   	elig1[i] = floatalloc(numhidden);

   elig2	= floatalloc(numhidden+1);
   elig3	= floatalloc(numinputs+1);
}

/* floatalloc accepts size of float array to be created and return pointer to it */
float* QNet::floatalloc(size_t size)
{
	float* ret;

   if ( (ret = (float*) calloc(size, sizeof(float))) == NULL )
   {
   	cout<<"Not enough memory!!!";
      exit(-1);
   }
   return ret;
}

/* floatalloc accepts size of float pointer array to be created and return pointer to it */
float** QNet::pfloatalloc(size_t size)
{
	float** ret;

   if ( (ret = (float**) calloc(size, sizeof(float*))) == NULL )
   {
   	cout<<"Not enough memory!!!";
      exit(-1);
   }
   return ret;
}

/* getrealrandom returns a real random number in the range (0 - irange) /1000 */

float QNet::getrealrandom(int irange)
{
   int r;
   float rfloat;

   r = rand() % irange + 0 ;
   rfloat=r;
   rfloat /= 1000;

   return rfloat;
}

float QNet::getmaxerr(void)
{
	return maxerr;
}
float QNet::getavgerr(void)
{
	return avgerr;
}


//the following three functions are provided incase we need to change things during training
void QNet::SetLRate1(float lrate)
{
   LRATE1 = lrate;
}

void QNet::SetLRate2(float lrate)
{
   LRATE2 = lrate;
}

void QNet::SetMomentum(float momentum)
{
	MOMENTUM = momentum;
}

void QNet::SetBias(float bias)
{
	BIAS = bias;
}
void QNet::SetDecay(float decay)
{
	GAMMA = decay;
}

void QNet::SetLambda(float l)
{
	LAMBDA = l;
}

int QNet::NumInputs()
{
	return numinputs;
}
int QNet::NumHidden()
{
	return numhidden;
}

float QNet::GetHighestWeights(float* qvector){
	int i,j;

	//set the BIAS
	qvector[numinputs] = BIAS;
	hiddenunits[numhidden] = BIAS;

	//calculate hidden values
	for (i=0; i<numhidden; i++)
	{
		hiddenunits[i] = 0.0;
		//each input (not forgetting BIAS unit)
		for (j=0; j<=numinputs; j++)
		{
      		hiddenunits[i] += weights1[j][i] * qvector[j];
		}

		//no longer sigmoid - now hyperbolic tangent (tanh) range -1 to +1
		
		hiddenunits[i] = SQUASH(hiddenunits[i]);
	}

	//calculate output value- again not forgetting BIAS
	outputunit = 0.0;
	for (i=0; i<=numhidden; i++)
   		outputunit += weights2[i] * hiddenunits[i];

	//in-out links
	if(directlinks)
	{
		for (i=0; i<=numinputs; i++)
	   		outputunit += weights3[i] * qvector[i];
	}

	//no longer sigmoid - now hyperbolic tangent (tanh) range -1 to +1
	outputunit = SQUASH(outputunit);
	return (outputunit);

}