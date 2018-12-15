#ifndef SIMULATIONPROCESS_H
#define SIMULATIONPROCESS_H

#include <QObject>
#include <QList>
#include <vector>
#include <iostream>

class TiffDataRead;

using namespace std;

class SimulationProcess : public QObject
{
	Q_OBJECT

public:
	SimulationProcess(QObject* parent);
	~SimulationProcess();

signals:
	void sendColsandRows(int,int,int);
	void sendparameter(QString);

public slots:
	void acceptFinisheCode(int _finished);
	bool readImageData();
	bool imageOpen(QString filename);
	bool imageOpenConver2uchar(QString filename);
	void startloop();
	void runloop2();

	void stoploop();
	bool getUiparaMat();
	void saveResult(string filename);
	double mypow(double _num,int times);

	unsigned char* uRGB()
	{
		return u_rgb;
	}

	unsigned char* uRGBshow()
	{
		return u_rgbshow;
	}


private:
	size_t _rows;
	size_t _cols;
	int nType;
	int numWindows;
	int sizeWindows;
	int looptime;
	double degree;
	bool isRestrictExit;
	bool isSave;
	int isbreak;
	int* goalNum;
	int* mIminDis2goal;
	string savepath;

protected:
	QList<TiffDataRead*> imgList;
	QList<int> typeInitialCount;
	QList<int> typeIndex;
	QList<QString> typeName;

	QList<int> multipleYear;

	QList<int> multiDemand;

	int* saveCount;
	int* val;
	double* mdNeiborhoodProbability;
	double* mdRoulette;
	double* probability;
	double* sProbability;
	double* normalProbability;
	double* mdNeighIntensity;
	unsigned char* temp;
	double** t_filecost;
	int** direction;
	short** Colour;

protected:
	unsigned char* u_rgbshow;
	unsigned char* u_rgb;
	int finishedCode;

};

#endif // SIMULATIONPROCESS_H
