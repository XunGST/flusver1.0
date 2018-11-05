#ifndef MARKOVPLAYER_H
#define MARKOVPLAYER_H

#include <QObject>
#include <QVector>

class TiffDataRead;

class markovPlayer : public QObject
{
	Q_OBJECT

public:
	markovPlayer(QObject *parent);

	markovPlayer(QVector<TiffDataRead*> _start,QVector<TiffDataRead*> _end, int _startyear,int _endyear, int _predictyear);

	~markovPlayer();

	void runPredict();

private:
	void checkInput();
	void getConvMatrix();
	template<class TT> unsigned char convert(QVector<TiffDataRead*> vImage,int ii);
	unsigned char convert2uchar(QVector<TiffDataRead*> vImage,int ii);
	void markovRun();

private:
	QVector<TiffDataRead*> startImg;
	QVector<TiffDataRead*> endImg;
	int startyear;
	int endyear;
	int predictyear;

	double nodataS;
	double nodataE;

	double maxmum;
	double minmum;

	size_t iSrows;
	size_t iScols;

	size_t iErows;
	size_t iEcols;

	unsigned char* uImgStart;
	unsigned char* uImgEnd;

	QVector<int> vExitValue;
	QVector<double> vSumStart;
	QVector<double> vSumEnd;
	double **pixelstatistic;
	size_t _sumValue;
};

#endif // MARKOVPLAYER_H
