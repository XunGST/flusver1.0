#ifndef KAPPACALCULATE_H
#define KAPPACALCULATE_H

#include <QWidget>
#include <QVector>
#include "ui_kappacalculate.h"
#include "kappacalculator.h"
#include <vector>
#include <iostream>
using namespace std;

class FoMcalculator;
class TiffDataRead;
class KappaCalculator;

struct fomReader  
{
	TiffDataRead* _pread;
	int serialnumber;
	string _path;
};


class KappaCalculate : public QWidget
{
	Q_OBJECT

public:
	KappaCalculate(QWidget *parent = 0);
	~KappaCalculate();

public slots:
	void openTurthFile();
	void openCompdFile();
	void startcalculate();
	void startcalculateFoM();
	void switchEnable();
	void readfileforFoM();



	

private:
	bool turthLoadImage( QString* _fileName );
	void clearAllTurth();
	bool compdLoadImage( QString* _fileName );
	void clearAllCompd();
	void read2textedit();
	void read2texteditFoM();
	void clearFoMfile();
	void openfileforFoM(QString* _fileName,int snum);




private:

	QVector<TiffDataRead*> vTurth_poDataset;

	QVector<TiffDataRead*> vCompd_poDataset;

	vector<int> v1;

	vector<fomReader> sFoM_Reader;

	double dNodataValue_Turth;

	double dNodataValue_Compd;

	double dNodataValue_Start;

	double mdsamplingrate;

	int misamplingamount;

	KappaCalculator* kcr;

	FoMcalculator* fcr;

	int mode;

	Ui::KappaCalculate ui;
};

#endif // KAPPACALCULATE_H
