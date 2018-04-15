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
	DynaSimulation(GeoDpCAsys* _gdp);
	~DynaSimulation();


public slots:
	bool lauLoadImage2(QString* _fileName);
	void lauClearall2();
	void openLauFile2();
	void setLauColor2();
	void input_Future_Land_Area();

	bool probLoadImage2(QString* _fileName);
	void probClearall2();
	void openProbFile2();

	void saveSimScenario();

	bool restLoadImage(QString* _fileName);
	void restClearall();
	void openRestFile();

	void modeSelect();
	void inisimmulate();
	void startsimulate();
	void appendconfig(); // <追加参数配置>
	void loadconfig(); // <加载参数配置>
	void stopmodel();
	void getColsandRowsandshowDynamicOnUi( int __cols,int __rows ,int _k);
	void getParameter( QString _str );
	void closeDynaSimulation();
	void write2file();
	
private:
	QVector<double> xval; 
	QVector<double> yval; 
	QVector<double> extract_yval; 
	int xlength;

public:// <类对象>
	QList<TiffDataRead*> lau_poDataset2;
	int lauNumImage2;
	int lauSerialNum2;
	unsigned char* _landuse2;

	QList<TiffDataRead*> prob_poDataset2;
	int probNumImage2;
	int probSerialNum2;

	QList<TiffDataRead*> rest_poDataset;
	int restNumImage;
	int restSerialNum;
	bool restexit;

	int currentheight;

public:
	QList<double> rgbLanduseType2;
	QList<double> mvItensityofneighboreffect;
	QList<QString> lauTypeName2;
	QList<QColor> rgbType2;
	QList<int> staCount2;

public:
	double nodatavalue2;
	bool nodataexit2;
	bool isfinished;

public:
	unsigned char* u_rgbshow;
	unsigned char* u_rgb;


	QTableWidget* futuretableWidget;
	QTableWidget* restricttableWidget;
	QTableWidget* switchcost;
	QTableWidget* mqIntenofneigh;
	
	GeoDpCAsys* m_gdp2;
	SimulationProcess* slp;

public:
	Ui::DynaSimulation ui;
};

#endif // DYNASIMULATION_H
