#ifndef GEODPCASYS_H
#define GEODPCASYS_H

#include <gdal_priv.h>
#include <QtCore/QList>
#include <QtGui/QMainWindow>
#include "ui_geodpcasys.h"
#include "geosimulator.h"

class predictionform;
class ShowProperty;
class GeoSimulator;
class DynaSimulation;
class med;

class GeoDpCAsys : public QMainWindow
{
	Q_OBJECT

public:
	GeoDpCAsys(QWidget *parent = 0, Qt::WFlags flags = 0);
	~GeoDpCAsys();

private: 
	bool loadImage(const char* _fileName);

public slots:
	void pickOpenFile();
	void showImageOrBand();
	void closeChoosenData();
	void showMouseRightMenu(const QPoint &pos);
	void showImgProperty();
	void comboBoxAdd();
	void comboBoxChange();
	void setCheck();
	void setGroupName();
	void clearAll();
	void geosimulatorstart();
	void dynasimulationprocess();
	void infomation();
	void restart();
	void openMarkov();
	void openMED();

	


protected:// <Àà¶ÔÏó>
	QList<TiffDataRead*> Ui_poDataset;
	QList<int> ischecked;
	int NumImage;
	int serialNum;
	int child_serialNum;
	bool ifPickFile;


	int deadline_year;
	int deadline_month;
	int deadline_day;

private:
	ShowProperty* imgProperty;
	GeoSimulator* geosim;
	DynaSimulation* dynasim;
	predictionform* predf;
	med* mMorEroDla;

private:
	Ui::GeoDpCAsysClass ui;
};

#endif // GEODPCASYS_H
