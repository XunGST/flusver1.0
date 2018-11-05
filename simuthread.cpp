#include "simuthread.h"
#include "simulationprocess.h"


SimuThread::SimuThread( QObject *parent,SimulationProcess* _slp )
{
	m_slp=_slp;
}

SimuThread::~SimuThread()
{

}

void SimuThread::run()
{
	m_slp->startloop();
	m_slp->runloop();
}
