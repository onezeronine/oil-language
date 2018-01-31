/* 
 * File:   dirnet.h
 * Author: Kenneth Bastian
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */

#ifndef __DIRECTNET
#define __DIRECTNET

#include "board.h"
#include "evalnet.h"
#include "features.h"
#include "qnet.h"
#include <iostream>

#define	NET_BINARYMAP	-1
#define	NET_DIRECTMAP	0
#define	NET_FEATUREMAP	1

typedef int (*ffunc)(BOARD*);


class DirectNet
{
private:
	EvalNet* net;
	QNet* qnet;
	ofstream df;
	int		whatami;
	float* iv;

	//Q-vectors
	float* qv;
	//IF Q-learning
	int qlearn;

	//feature stuff

	//ffunc*  feature_funcs[NUMOFFEATURES+1];
	int (*feature_funcs[NUMOFFEATURES+1]) (BOARD*);
	int	bitstouse[NUMOFFEATURES];

	ffunc*	input_map;
	int	numfound;


	float*	floatalloc(size_t size);
	float** pfloatalloc(size_t size);

	int	SetupFeatures(char* fname);
	//float** ComputeTrainingV(BOARD** pboard, int moves);
	void mapfeatures(BOARD* pboard,QBOARD* qmapboard);

public:
	DirectNet(int typeofnet, int nhidden, bool dlinks, unsigned seed,
   	float lowrange, float highrange, float bias, char* name, char* featurefile, int q);
	DirectNet(char* fname, char* featuref, float bias);
	~DirectNet();

	float	getmaxerr(void); //returns the maximum error encountered since init
	float	getavgerr(void); //returns a cumulation of error

	void	GetName(char* name);
	void	SetName(char* name);

	void 	InitTDTrain(BOARD* board, float gamma, float lambda, int swapboard, QBOARD* qmapboard);
	void 	TDTrain(BOARD* board, int swapboard, QBOARD* qmapboard);
	void 	TDFinal(float evalue);

	float	EvaluateNet(BOARD* pboard,QBOARD* qmapboard);
	void 	SaveNet(char* fname);
	void 	LoadWeights(char* fname);
	void 	SetBias(float bias);
	void 	SetLRate1(float lrate);
	void 	SetLRate2(float lrate);
	void 	SetMomentum(float mom);
	int 	NumInputs();
	int 	NumHidden();
	int		GetQLearn();
	float	EvaluateQ(QBOARD* qmapboard);
};

#endif