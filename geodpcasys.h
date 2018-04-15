#ifndef GEODPCASYS_H
#define GEODPCASYS_H

#include <gdal_priv.h>
#include <QtCore/QList>
#include <QtGui/QMainWindow>
#include "ui_geodpcasys.h"

class ShowProperty;
class GeoSimulator;
class DynaSimulation;

class GeoDpCAsys : public QMainWindow
{
	Q_OBJECT

public:
	GeoDpCAsys(QWidget *parent = 0, Qt::WFlags flags = 0);
	~GeoDpCAsys();

private: // <加载图像数据>
	bool loadImage(const char* _fileName);

public slots:
	/*Image Scan*/
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


	


protected:// <类对象>
	QList<TiffDataRead*> Ui_poDataset;
	QList<int> ischecked;
	int NumImage;
	int serialNum;
	int child_serialNum;
	bool ifPickFile;

private:
	ShowProperty* imgProperty;
	GeoSimulator* geosim;
	DynaSimulation* dynasim;

private:
	Ui::GeoDpCAsys ui;
};

#endif // GEODPCASYS_H
