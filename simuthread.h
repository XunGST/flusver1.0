#ifndef SIMUTHREAD_H
#define SIMUTHREAD_H

#include <QThread>

class SimulationProcess;

class SimuThread : public QThread
{
	Q_OBJECT

public:
	SimuThread(QObject *parent,SimulationProcess* _slp);
	~SimuThread();
	void run();

private:
	SimulationProcess* m_slp;
};

#endif // SIMUTHREAD_H
