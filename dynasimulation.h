#ifndef DYNASIMULATION_H
#define DYNASIMULATION_H

#include <QVector>
#include <QWidget>
#include "ui_dynasimulation.h"


class TiffDataRead;
class GeoDpCAsys;
class QTableWidget;
class SimulationProcess;
class QLabel;

class DynaSimulation : public QWidget
{
	Q_OBJECT

public:
	DynaSimulation(QObject* parent);
	~DynaSimulation();

signals:
	void sendfinishedcode(int _finished);

public slots:
	void openLauFile2();
	void setLauColor2();
	void input_Future_Land_Area();
	void openProbFile2();
	void saveSimScenario();
	void openRestFile();
	void modeSelect();
	void inisimmulate();
	void startsimulate();
	void stopmodel();
	void getColsandRowsandshowDynamicOnUi( int __cols,int __rows ,int _k);
	void getParameter( QString _str );
	void restoreOneYears();
	
private:	
	void readDemandFile(QString filename);
	void write2file();
	void appendconfig(); 
	void loadconfig(); 
	void checkSettings();
	void readImageData();
	
private:
	QList<TiffDataRead*> imglist;
	QVector<double> xval; 
	QVector<double> yval; 
	QVector<double> extract_yval; 
	QList<int> countAvaliable;
	QList<int> oneyearFuture;
	QList<QString> NameAvaliable;
	QList<QString> imgPath;
	QList<QColor> rgbAvaliable;
	double nodatavalue2;
	bool nodataexit2;
	bool isMultiYears;
	int currentheight;
	int xlength;

protected:
	int countfinished;
	QTableWidget* futuretableWidget;
	QTableWidget* switchcost;
	QTableWidget* mqIntenofneigh;
	SimulationProcess* slp;

private:
	Ui::DynaSimulation ui;
};

#endif // DYNASIMULATION_H
