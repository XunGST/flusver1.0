#ifndef KAPPATHREAD_H
#define KAPPATHREAD_H

#include <QThread>
#include "kappacalculator.h"

class FoMcalculator;
class KappaCalculator;

class KappaThread : public QThread
{
	Q_OBJECT

public:
	KappaThread(KappaCalculator* _krc);
	KappaThread(FoMcalculator* _frc);
	~KappaThread();
	void run();

private:
	KappaCalculator* mkrc;
	FoMcalculator* mfrc;
	int startWhat;
};

#endif // KAPPATHREAD_H
