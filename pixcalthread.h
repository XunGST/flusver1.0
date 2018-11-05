#ifndef PIXCALTHREAD_H
#define PIXCALTHREAD_H

#include <QThread>

class PixCal;

class PixCalThread : public QThread
{
	Q_OBJECT

public:
	PixCalThread(QObject *parent,PixCal* pcl);
	~PixCalThread();

public:
	void run();

private:
	PixCal* m_pcl;
};

#endif // PIXCALTHREAD_H
