#ifndef THREADMED_H
#define THREADMED_H

#include <QThread>

class MorphEroDla;

class threadmed : public QThread
{
	Q_OBJECT

public:
	threadmed(QObject *parent);
	threadmed(QObject *parent,MorphEroDla* _Med);
	~threadmed();
	void run();

private:
	MorphEroDla* mMed;
	
};

#endif // THREADMED_H
