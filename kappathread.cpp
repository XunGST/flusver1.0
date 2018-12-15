#include "kappathread.h"
#include "kappacalculator.h"
#include "fomcalculator.h"


KappaThread::KappaThread(KappaCalculator* _krc)
{
	mkrc=_krc;
	startWhat=1;
}

KappaThread::KappaThread(FoMcalculator* _frc)
{
	mfrc=_frc;
	startWhat=2;
}

KappaThread::~KappaThread()
{
	
}

void KappaThread::run()
{

	if (startWhat==1)
	{
		mkrc->calculatorKappa();
	}
	else
	{
		mfrc->calculateFoMprocess();
	}
}
