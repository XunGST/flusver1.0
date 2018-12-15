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

	ColorPlate(QWidget *parent);
	~ColorPlate();

signals:
	void sendWinClose();

private:
	bool showCountlist(); // 模拟
	void loadconfigfile(); // 模拟
	void buildconfigfile(); // 模拟

public slots:	
	void finished(); // 模拟
	void closeWin(); // 通用
	void setInvalidValue(QString position); // 模拟
	void setColor(QString position); // 模拟

protected:
	QList<QComboBox* > L_comBox;
	QList<QPushButton*> L_btn;
	QList<int> mlRgbLanduseType;
	QList<int> mlStastisticCount;
	QList<QString> mlLauTypeName;
	QList<QColor> mlColorType;
	
private:
	Ui::ColorPlate ui;
	GeoSimulator* m_gslr;
	DynaSimulation* m_dsl;
};


#endif // COLORPLATE_H
