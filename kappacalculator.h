#ifndef KAPPACALCULATOR_H
#define KAPPACALCULATOR_H

#include <QObject>
#include <QVector>

class TiffDataRead;

class KappaCalculator : public QObject
{
	Q_OBJECT

public:
	KappaCalculator(QVector<TiffDataRead*> _turth,QVector<TiffDataRead*> _compd, double _rateofsampling, int _numofsample);
	~KappaCalculator();

	void calculatorKappa();
	void stasticpixel();
	template<class TT> unsigned char convert(QVector<TiffDataRead*> vImage,int ii);
	unsigned char convert2uchar(QVector<TiffDataRead*> vImage,int ii);
	bool setRandomPoint();




private:
	QVector<TiffDataRead*> turth;
	QVector<TiffDataRead*> compd;
	QVector<int> vExitValue;
	QVector<int> vCountTurthExitValue;
	QVector<int> vCountCompdExitValue;

	unsigned char* uturth;
	unsigned char* ucompd;

	unsigned char* sampling_uturth;
	unsigned char* sampling_ucompd;

	int* m_PointCoodinateX;
	int* m_PointCoodinateY;

	double maxmum;
	double minmum;

	double nodatat;
	double nodatac;
	size_t itrows;
	size_t itcols;

	size_t icrows;
	size_t iccols;

	double mdsamplingRate;
	size_t mnsamplingAmountAverage;
	size_t mnNumofSam; // 总样本数
	
};

#endif // KAPPACALCULATOR_H
