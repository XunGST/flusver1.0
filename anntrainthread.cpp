#include "anntrainthread.h"
#include "nntrain.h"

AnnTrainThread::AnnTrainThread(QObject *parent,NNtrain* _ntrn)
	: QThread(parent)
{
	m_ntrn=_ntrn;
	//run();
}

AnnTrainThread::~AnnTrainThread()
{

}

void AnnTrainThread::run()
{
	m_ntrn->trainprocess();
}
