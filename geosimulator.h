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

	void hideLayer();
	bool lauLoadImage(QString* _fileName);
	void lauClearall();
	void openLauFile();
	void setLauColor();

	bool divLoadImage(QString* _fileName);
	void divClearall();
	void selectDivFiles();
	void addOneDivFile();
	void getBasicDivInfo();
	void substractDivFileInitial();
	void substractDivFile();

	void saveProb_PreData();
	void trainAndSaveAsTif();

	void getParameter(QString _str);

	void closeGeoSimulator();


public:
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
	double nodatavalue;
	bool nodataexit;
	GeoDpCAsys* m_gdp;

public:
	Ui::GeoSimulator ui;
	
	

};

#endif // GEOSIMULATOR_H
