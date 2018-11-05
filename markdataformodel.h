#ifndef MARKDATAFORMODEL_H
#define MARKDATAFORMODEL_H

#include <QWidget>
#include "bandlist.h"
#include "geodpcasys.h"

class MarkDataForModel : public QWidget
{
	Q_OBJECT

public:
	MarkDataForModel(QWidget *parent = 0);
	MarkDataForModel(QList<TiffDataRead*> _inUi_poDataset, int _serialnum);
	~MarkDataForModel();

public slots:
	void move2TextInitial();
	void move2TextlauToItem();
 	void move2TextmskToItem();
 	void move2TextdivToItem();
 	void move2TextconToItem();
 	void move2TextajtToItem();

	void lauItem2TreeIni();
	void mskItem2TreeIni();
	void divItem2TreeIni();
	void conItem2TreeIni();
	void ajtItem2TreeIni();

	void lauItem2Tree();
	void mskItem2Tree();
	void divItem2Tree();
	void conItem2Tree();
	void ajtItem2Tree();


	void allInputBtnEnable(bool _btnendable);
	void radioConAjtGropAble();
	void judgeListViewNull();
	bool judgeIfSelect(int _selectSrl);

	void winClose();
	void whenfinishclicked();

signals:
	void sandallmarkdata(int _lau,int _msk,QList<int> _div,QList<int> _con,QList<int> _ajt);

private:
	int lauSrlnum;
	int mskSrlnum;
	QList<int> divSrlnum;
	QList<int> conSrlnum;
	QList<int> ajtSrlnum;
	QList<int> beMarkedNbrs;
	QStandardItemModel* _fileLauList;
	QStandardItemModel* _fileMskList;
	QStandardItemModel* _fileDivList;
	QStandardItemModel* _fileConList;
	QStandardItemModel* _fileAjtList;
	QStandardItem* _fileNode;
	bool btnEnable;
	QModelIndex index;	

};

#endif // MARKDATAFORMODEL_H
