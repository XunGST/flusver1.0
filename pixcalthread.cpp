#include "pixcalthread.h"
#include "PixCal.h"

PixCalThread::PixCalThread(QObject *parent,PixCal* pcl)
	: QThread(parent)
{
	m_pcl=pcl;
}


PixCalThread::~PixCalThread()
{

}

void PixCalThread::run()
{
	m_pcl->pixcalculate();
}
