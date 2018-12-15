#ifndef COLORTHREAD_H
#define COLORTHREAD_H

#include <QThread>
#include <QList>
#include <QtCore>

class GeoSimulator;
class DynaSimulation;
class TiffDataRead;

class PixCal : public QThread
{
	Q_OBJECT

public:

	PixCal(QObject *parent,QString logFilePath,int numLine);
	~PixCal();
	template<class TT> bool readData();

	bool lauLoadImage2( QString _fileName );


public:
	void pixcalculate();

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
