#ifndef COLORTHREAD_H
#define COLORTHREAD_H

#include <QThread>
#include <QList>
#include <QList>

class GeoSimulator;
class DynaSimulation;

class PixCal : public QThread
{
	Q_OBJECT

public:
	PixCal(GeoSimulator* _gsl);
	PixCal(DynaSimulation* _ds,bool pattern);
	~PixCal();
	template<class TT> bool readData();
	template<class TT> bool readData2();

public:
	void pixcalculate();

signals:
	void countThreadSendValue(int _Val);

private:
	GeoSimulator* m_gsl;
	DynaSimulation* m_ds;
};

#endif // COLORTHREAD_H
