#ifndef COLORTHREAD_H
#define COLORTHREAD_H

#include <QThread>
#include <QList>
#include <QtCore>

class GeoSimulator;
class DynaSimulation;
class TiffDataRead;

class PixCaltrain : public QThread
{
	Q_OBJECT

public:

	PixCaltrain(QObject *parent,QString logFilePath,int numLine);
	~PixCaltrain();
	template<class TT> bool readData();

	bool lauLoadImage2( QString _fileName );


public:
	void PixCaltrainculate();

signals:
	void countThreadSendValue(int _Val);

private:
	QList<QString> mvLineList;
	int mdNumline;
	TiffDataRead* pread;


	GeoSimulator* m_gsl;
	DynaSimulation* m_ds;
};

#endif // COLORTHREAD_H
