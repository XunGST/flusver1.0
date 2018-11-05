#ifndef PREDICTIONFORM_H
#define PREDICTIONFORM_H

#include <QWidget>
#include "ui_predictionform.h"

class TiffDataRead;
class markovPlayer;


class predictionform : public QWidget
{
	Q_OBJECT

public:
	predictionform(QWidget *parent = 0);
	~predictionform();

public slots:
	void openStartFile();
	void openEndFile();
	void decideEnable();
	void runMakovChain();

private:
	bool startLoadImage( QString* _fileName );
	void clearAllStart();
	bool endLoadImage( QString* _fileName );
	void clearAllEnd();
	void generatefutureyear();
	void read2textedit();
	

private:
	Ui::predictionform ui;
	QVector<TiffDataRead*> vStartpoDataset;
	QVector<TiffDataRead*> vEndpoDataset;
	int startyear;
	int endyear;

	markovPlayer* mkp;

	
};

#endif // PREDICTIONFORM_H
