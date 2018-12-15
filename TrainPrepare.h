#ifndef TrainPrepare_H
#define TrainPrepare_H

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include "ui_TrainPrepare.h"
#include <QComboBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QSignalMapper>

class TiffDataRead;


class TrainPrepare : public QWidget
{
	Q_OBJECT

public:
	TrainPrepare(QWidget *parent);
	~TrainPrepare();

signals:
	void sendWinClose();

private:
	bool probabilityShowCountlist(); // 概率
	void buildconfigfile(); // 概率

public slots:
	void refinished();// 概率
	void closeWin(); // 概率
	void resetInvalidValue(QString position);// 概率

protected:
	QList<QComboBox* > L_comBox;
	QList<QPushButton*> L_btn;
	QList<int> mlRgbLanduseType;
	QList<int> mlStastisticCount;
	
private:
	Ui::TrainPrepare ui;
};


#endif // TrainPrepare_H
