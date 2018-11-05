#include "threadmed.h"
#include "morpherodla.h"

threadmed::threadmed(QObject *parent)
	: QThread(parent)
{

}

threadmed::threadmed(QObject *parent,MorphEroDla* _Med)
{
	mMed=_Med;
}

threadmed::~threadmed()
{

}

void threadmed::run()
{
	mMed->runMED();
}
