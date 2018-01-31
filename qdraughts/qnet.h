/* 
 * File:   qnet.h
 * Author: Rarepon
 *
 * The code is derived from Mark Lynch's Neurodraughts
 */

#ifndef _QNET_H
#define	_QNET_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>

#include <time.h>

using namespace std;

class QNet{
    private:
    int numinputs;          //network dimensions
    int numhidden;
    bool directlinks;       //direct in-out connections?
    char name[64];          //name of the network

    float** weights1;       //input layer - hidden weights
    float* weights2;        //hidden layer - ouput weights
    float* weights3;        //in layer - ouput weights
    float** oldweights1;    //input layer - hidden weights -old
    float* oldweights2;     //hidden layer - ouput weights -old
    float* oldweights3;     //in layer - ouput weights -old
    float* hiddenunits;     //hidden unit values
    float outerror;         //output error

    float** elig1;          //eligibility trace for in - hid layer
    float* elig2;           //eligibility trace for hid - out layer
    float* elig3;           //eligibility trace for in - out layer

    float oldoutput;        //previous output - for TD
    float outputunit;       //output value. ie. evaluation

    float maxerr, avgerr;   //keep track of error in net

    float BIAS;             //value of BIAS (normally 1)
    float LRATE1;           //learning rate of the net - in - hid links (1/inputs)
    float LRATE2;           //hid- out lrate 1/hidunits
    float MOMENTUM;         //momentun value
    float GAMMA;            //rate of decay in TD training
    float LAMBDA;           //how much the sum of deriv (elig) trickles back

    unsigned seed;           //the seed for random number gen

    void AllocateMem();
    float* floatalloc(size_t size);
    float** pfloatalloc(size_t size);
    void InittoRandom(float lowrange, float highrange);
    void InitElig();
    void InitError();
    void InitMomentum();
    void UpdateElig(float* iv);
    void UpdateWeights();

public:

    QNet();
    QNet(int numinputs, int numhidden, bool dlinks, unsigned seed, float lowrange, float highrange, float bias, char* n);
    QNet(char* loadfile, float bias );
    ~QNet();

    void GetName(char* name);
    void SetName(char* name);

    float EvaluateNet(float* qvector);

	//Q functions
	void InitQTrain(float* qv, float gamma, float lambda);
	void QTrain(float* qv);
	void QFinal(float evalue);
	float GetHighestWeights(float* qvector);
	//end Q

    void SetLRate1(float lrate);
    void SetLRate2(float lrate);
    void SetMomentum(float momentum);
    void SetBias(float bias);
    void SetLambda(float l);
    void SetDecay(float decay);

    float getmaxerr(void);
    float getavgerr(void);
    float getrealrandom(int irange);

    void SaveNet(char* savefile);
    void LoadWeights(char* loadfile);

    void Report();
    int	NumInputs();
    int	NumHidden();
	
};

#endif	/* _QNET_H */

