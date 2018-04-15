#ifndef COLORPLATE_H
#define COLORPLATE_H

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include "ui_colorplate.h"
#include <QComboBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QSignalMapper>

class TiffDataRead;
class ColorThread;
class GeoSimulator;
class DynaSimulation;

class ColorPlate : public QWidget
{
	Q_OBJECT

public:

	ColorPlate(QWidget *parent,DynaSimulation* _dsl);
	ColorPlate(QWidget *parent,GeoSimulator* _gsr,bool patten);  // 重载
	~ColorPlate();

public slots:
	bool showCountlist(); // 模拟
	bool probabilityShowCountlist(); // 概率
	void setInvalidValue(QString position); // 模拟
	void resetInvalidValue(QString position);// 概率
	void setColor(QString position); // 模拟
	void finished(); // 模拟
	void refinished();// 概率
	void buildconfigfile(); // 模拟
	void loadconfigfile(); // 模拟
	void closeWin(); // 通用

protected:
	QList<QComboBox* > L_comBox;
	QList<QPushButton*> L_btn;
	
private:
	Ui::ColorPlate ui;
	GeoSimulator* m_gslr;
	DynaSimulation* m_dsl;
};


#endif // COLORPLATE_H
