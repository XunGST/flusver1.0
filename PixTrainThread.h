#ifndef PixTrainThread_H
#define PixTrainThread_H

#include <QThread>

class PixCal;

class PixTrainThread : public QThread
{
	Q_OBJECT

public:
	PixTrainThread(QObject *parent,PixCal* pcl);
	~PixTrainThread();

public:
	void run();

private:
	PixCal* m_pcl;
};

#endif // PixTrainThread_H
