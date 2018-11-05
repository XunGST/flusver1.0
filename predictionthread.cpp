#include "predictionthread.h"
#include "markovplayer.h"

predictionthread::predictionthread(QObject *parent)
	: QThread(parent)
{

}

predictionthread::predictionthread(QObject *parent,markovPlayer* _mpr)
	: QThread(parent)
{
	mpr=_mpr;
}

predictionthread::~predictionthread()
{
	
}

void predictionthread::run()
{
	mpr->runPredict();
}
