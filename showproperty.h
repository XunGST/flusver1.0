#ifndef SHOWPROPERTY_H
#define SHOWPROPERTY_H

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include "ui_showproperty.h"
#include "TiffDataRead.h"

class ShowProperty : public QWidget
{
	Q_OBJECT

public:
	ShowProperty(QWidget *parent = 0);
	ShowProperty(QList<TiffDataRead*> _inUi_poDataset, int _serialnum);
	~ShowProperty();
	bool getImgProperty(QList<TiffDataRead*> _inUi_poDataset, int _serialnum);

private:
	Ui::ShowProperty ui;
	/// <summary>
	/// <图像元数据模型>
	/// </summary>
	QStandardItemModel *imgMetaModel;
};

#endif // SHOWPROPERTY_H
