#include "PixTrainThread.h"
#include "PixCal.h"

PixTrainThread::PixTrainThread(QObject *parent,PixCal* pcl)
	: QThread(parent)
{
	m_pcl=pcl;
}


PixTrainThread::~PixTrainThread()
{

}

void PixTrainThread::run()
{
	m_pcl->pixcalculate();
}
