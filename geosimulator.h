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
	GeoSimulator( QObject* parent);
	~GeoSimulator();

public slots:
	void getParameter(QString _str);
	void openLauFile();
	void setLauColor();
	void selectDivFiles();
	void addOneDivFile();
	void substractDivFile();
	// int&out
	void saveProb_PreData();
	void trainAndSaveAsTif();
	void substractDivFileInitial();
	void activatedModule();

private:
	// get para
	void write2file();
	void closeGeoSimulator();
	
	void writeNetworkParameter();
	// tool
	void hideLayer();
	// lau
	bool lauLoadImage(QString* _fileName);
	void lauClearall();
	// div
	bool divLoadImage(QString* _fileName);
	void divClearall();

	void getBasicDivInfo();
	

private:// <Àà¶ÔÏó>
	QList<TiffDataRead*> lau_poDataset;
	int lauNumImage;
	int lauSerialNum;
	unsigned char* _landuse;

    QStandardItemModel *divMetaTable;
	QList<TiffDataRead*> div_poDataset;
	int divNumImage;
	int divSerialNum;



private:
	Ui::GeoSimulator ui;
	
	

};

#endif // GEOSIMULATOR_H
