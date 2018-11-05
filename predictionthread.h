#ifndef PREDICTIONTHREAD_H
#define PREDICTIONTHREAD_H

#include <QThread>

class markovPlayer;

class predictionthread : public QThread
{
	Q_OBJECT

public:
	predictionthread(QObject *parent);
	predictionthread(QObject *parent,markovPlayer* _mpr);
	~predictionthread();
	void run();

private:
	markovPlayer* mpr;
	
};

#endif // PREDICTIONTHREAD_H
