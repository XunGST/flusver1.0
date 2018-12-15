#ifndef NNTRAIN_H
#define NNTRAIN_H

#include <gdal.h>
#include <QWidget>
#include <QtCore>

class GeoSimulator;
class TiffDataRead;

class NNtrain : public QWidget
{
	Q_OBJECT

public:
	NNtrain(QObject* parent);
	~NNtrain();

signals:
	void sendParameter(QString);

public:
	void trainprocess();
private:
	void nnTrain();
	bool normalizationdata();
	bool setRandomPoint(bool _stra);
	size_t randomFunction(size_t _lon);
	template<class TT> bool dataCopyConvert(unsigned char* buffer);
	void saveNormalizedData();
	bool imageOpen(QString filename);
	bool imageOpenConver2uchar(QString filename);
	bool getAllParameter();


private:
	int* m_PointCoodinateX;
	int* m_PointCoodinateY;
	size_t nWidth;
	size_t nHeight;
 	size_t bandCount; // 总波段数，不一定等于数据数
	int nData; // 数据数

 	size_t currentBand;
 	size_t currentPos;
	size_t numof;
	QList<double> minmaxsave;

	float* f_allBandData;
	double* d_allBandData;
	unsigned short* us_allBandData;

private:
	QList<int> mvLanduseType;
	QList<int> mvCountType;

	QString pathoflanduse;
	QString pathofsimresult;
	QList<QString> pathofdivingfactor;
	QString datatype;
	bool isNomalized;
	bool isUnifomSam;
	bool isNoDataExit;
	double noDataValue;
	double samplingRate;
	int numHiddenLayer;
	QList<TiffDataRead*> imgList;
	QList<TiffDataRead*> divingList;
	QList<QString> bandName;

	float* saveMemF;
	double* saveMemD;
	unsigned short* saveMemUs;

};

#endif // NNTRAINTHREAD_H
