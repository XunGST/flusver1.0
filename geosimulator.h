#ifndef GEOSIMULATOR_H
#define GEOSIMULATOR_H

#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include "ui_geosimulator.h"
#include <QtCore>
#include <QList>

class TiffDataRead;
class GeoDpCAsys;

class GeoSimulator : public QWidget
{
	Q_OBJECT

public:
	GeoSimulator(GeoDpCAsys* _gdp);
	~GeoSimulator();

public slots:
	// tool
	void hideLayer();
	// lau
	bool lauLoadImage(QString* _fileName);
	void lauClearall();
	void openLauFile();
	void setLauColor();

	// div
	bool divLoadImage(QString* _fileName);
	void divClearall();
	void selectDivFiles();
	void addOneDivFile();
	void getBasicDivInfo();
	void substractDivFileInitial();
	void substractDivFile();

	// int&out
	void saveProb_PreData();
	void trainAndSaveAsTif();

	// get para
	void getParameter(QString _str);
	void write2file();
	void closeGeoSimulator();


public:// <Àà¶ÔÏó>
	QList<TiffDataRead*> lau_poDataset;
	int lauNumImage;
	int lauSerialNum;
	unsigned char* _landuse;

    QStandardItemModel *divMetaTable;
	QList<TiffDataRead*> div_poDataset;
	int divNumImage;
	int divSerialNum;


public:
	QList<double> rgbLanduseType;
	QList<int> staCount;
	float* p0;
	double* dp0;
	unsigned short* usSp0;
	double nodatavalue;
	bool nodataexit;
	GeoDpCAsys* m_gdp;

public:
	Ui::GeoSimulator ui;
	
	

};

#endif // GEOSIMULATOR_H
