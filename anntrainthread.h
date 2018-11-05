#ifndef ANNTRAINTHREAD_H
#define ANNTRAINTHREAD_H

#include <QThread>

class NNtrain;
class GeoSimulator;

class AnnTrainThread : public QThread
{
	Q_OBJECT

public:
	AnnTrainThread(QObject *parent,NNtrain* _ntrn);
	~AnnTrainThread();
	void run();

private:
	NNtrain* m_ntrn;
	
};

#endif // ANNTRAINTHREAD_H
