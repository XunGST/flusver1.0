#ifndef NNTRAIN_H
#define NNTRAIN_H

#include <gdal.h>
#include <QWidget>
#include <QtCore>

class GeoSimulator;

class NNtrain : public QWidget
{
	Q_OBJECT

public:
	NNtrain(GeoSimulator* _gsl);
	~NNtrain();

signals:
	void sendParameter(QString);

public slots:
	//void run();
	void trainprocess();
public:
	void nnTrain();
	bool normalizationdata();
	bool setRandomPoint(bool _stra);
	int randomFunction(int _lon);
	template<class TT> bool dataCopyConvert(unsigned char* buffer);

private:
	GeoSimulator* m_gsl;
	int* m_PointCoodinateX;
	int* m_PointCoodinateY;
	int nData;
	int nWidth;
	int nHeight;
	int bandCount;
	int currentBand;
	int currentPos;
	int numof;
	bool mskcon;
	double perofrp;
	QList<double> minmaxsave;

	float* f_allBandData;
	double* d_allBandData;
};

#endif // NNTRAINTHREAD_H
