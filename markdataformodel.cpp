#include "markdataformodel.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <iostream>
using namespace std;

MarkDataForModel::MarkDataForModel(QWidget *parent)
	: QWidget(parent)
{

}
/// <summary>
/// <重载构造函数>
/// </summary>
MarkDataForModel::MarkDataForModel( QList<TiffDataRead*> _inUi_poDataset, int _serialnum )
{
	// <左表初始化>
	ui.setupUi(this);
	ui.pro_mrkTree->setModel(ui.pro_mrkTree->FileListModel());/// <初始化文件列表>
	ui.pro_mrkTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.pro_mrkTree->setContextMenuPolicy(Qt::CustomContextMenu);
	//ui.pro_mrkTree->setStyleSheet("");
	ui.pro_mrkTree->getOtherListModel(_inUi_poDataset,_serialnum);

	// <右表1>
	_fileLauList=new QStandardItemModel;
	ui.listVlau->setModel(this->_fileLauList);
	// <右表2>
	_fileMskList=new QStandardItemModel(this);
	ui.listVmsk->setModel(this->_fileMskList);
	// <右表3>
	_fileDivList=new QStandardItemModel(this);
	ui.listVdiv->setModel(this->_fileDivList);
	// <右表4>
	_fileConList=new QStandardItemModel(this);
	ui.listVcon->setModel(this->_fileConList);
	// <右表5>
	_fileAjtList=new QStandardItemModel(this);
	ui.listVajt->setModel(this->_fileAjtList);


	this->allInputBtnEnable(false);
	this->judgeListViewNull();

	// <右下角按钮>
	connect(ui.cancelButton,SIGNAL(clicked()),this,SLOT(winClose()));
	connect(ui.nextButton,SIGNAL(clicked()),this,SLOT(whenfinishclicked()));

	connect(ui.pro_mrkTree,SIGNAL(clicked(const QModelIndex &)),this,SLOT(move2TextInitial()));
	connect(ui.listVlau,SIGNAL(clicked(const QModelIndex &)),this,SLOT(lauItem2TreeIni()));
	connect(ui.listVmsk,SIGNAL(clicked(const QModelIndex &)),this,SLOT(mskItem2TreeIni()));
	connect(ui.listVdiv,SIGNAL(clicked(const QModelIndex &)),this,SLOT(divItem2TreeIni()));
	connect(ui.listVcon,SIGNAL(clicked(const QModelIndex &)),this,SLOT(conItem2TreeIni()));
	connect(ui.listVajt,SIGNAL(clicked(const QModelIndex &)),this,SLOT(ajtItem2TreeIni()));

	connect(ui.raBtnEmpty,SIGNAL(toggled(bool)),this,SLOT(radioConAjtGropAble()));// <选中nodata后不可用>
	connect(ui.raBtnEmpty_2,SIGNAL(toggled(bool)),this,SLOT(radioConAjtGropAble()));// <选中nodata后不可用>
	connect(ui.lauBtnIn,SIGNAL(clicked()),this,SLOT(move2TextlauToItem()));
	connect(ui.mskBtnIn,SIGNAL(clicked()),this,SLOT(move2TextmskToItem()));
	connect(ui.divBtnIn,SIGNAL(clicked()),this,SLOT(move2TextdivToItem()));
	connect(ui.conBtnIn,SIGNAL(clicked()),this,SLOT(move2TextconToItem()));
	connect(ui.ajtBtnIn,SIGNAL(clicked()),this,SLOT(move2TextajtToItem()));
	connect(ui.lauBtnOut,SIGNAL(clicked()),this,SLOT(lauItem2Tree()));
	connect(ui.mskBtnOut,SIGNAL(clicked()),this,SLOT(mskItem2Tree()));
	connect(ui.divBtnOut,SIGNAL(clicked()),this,SLOT(divItem2Tree()));
	connect(ui.conBtnOut,SIGNAL(clicked()),this,SLOT(conItem2Tree()));
	connect(ui.ajtBtnOut,SIGNAL(clicked()),this,SLOT(ajtItem2Tree()));
}

MarkDataForModel::~MarkDataForModel()
{
	this->winClose();
}



/// <summary>
/// <所有输入按钮都不能用或能用>
/// </summary>
void MarkDataForModel::allInputBtnEnable( bool _btnendable )
{
	ui.lauBtnIn->setEnabled(_btnendable);
	ui.mskBtnIn->setEnabled(_btnendable);
	ui.divBtnIn->setEnabled(_btnendable);
	ui.conBtnIn->setEnabled(_btnendable);
	ui.ajtBtnIn->setEnabled(_btnendable);
	ui.raBtnInput->setChecked(true);
	ui.raBtnInput_2->setChecked(true);

	if (!_fileLauList->rowCount()<1)
	{
		ui.lauBtnIn->setEnabled(false);
	}
	if (!_fileMskList->rowCount()<1)
	{
		ui.mskBtnIn->setEnabled(false);
	}
}
/// <summary>
/// <选Nodata,Con与Adj不能用>
/// </summary>
void MarkDataForModel::radioConAjtGropAble()
{
	if (ui.raBtnEmpty->isChecked())  
	{  
		if (_fileConList->rowCount()!=0)
		{
			// <保护区数据>
			for (int kk=0;kk<_fileConList->rowCount();kk++)
			{
				for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
				{
					if (0 == QString::compare(this->_fileConList->item(kk,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
					{
						ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
						this->_fileConList->removeRow(kk);
						for (int jj=0;jj<beMarkedNbrs.size();jj++)
						{
							if (beMarkedNbrs[jj]==ii)
							{
								beMarkedNbrs.removeAt(jj);
							}
						}
						continue;
					}
				}
			}
		}
		ui.grupBoxCon->setEnabled(false);
	}   
	if (ui.raBtnEmpty_2->isChecked())  
	{  
		if (_fileAjtList->rowCount()!=0)
		{
			// <保护区数据>
			for (int kk=0;kk<_fileAjtList->rowCount();kk++)
			{
				for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
				{
					if (0 == QString::compare(this->_fileAjtList->item(kk,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
					{
						ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
						this->_fileAjtList->removeRow(kk);
						for (int jj=0;jj<beMarkedNbrs.size();jj++)
						{
							if (beMarkedNbrs[jj]==ii)
							{
								beMarkedNbrs.removeAt(jj);
							}
						}
						continue;
					}
				}
			}
		}
		ui.grupBoxAjt->setEnabled(false);
	} 
	judgeListViewNull();
}
/// <summary>
/// <判断右边文本是否为空>
/// </summary>
void MarkDataForModel::judgeListViewNull()
{
	// <土地利用文本输入输出按钮控制>
	if (_fileLauList->rowCount()==0)
	{
		ui.lauBtnOut->setEnabled(false);
	}
	// <掩模文本输入输出按钮控制>
	if (_fileMskList->rowCount()==0)
	{
		ui.mskBtnOut->setEnabled(false);
	}
	// <驱动力数据文本输入输出按钮控制>
	if (_fileDivList->rowCount()==0)
	{
		ui.divBtnOut->setEnabled(false);
	}
	// <保护区数据文本输入输出按钮控制>
	if (_fileConList->rowCount()==0)
	{
		ui.conBtnOut->setEnabled(false);
	}
	// <区域调整数据文本输入输出按钮控制>
	if (_fileAjtList->rowCount()==0)
	{
		ui.ajtBtnOut->setEnabled(false);
	}
	// <当左边没有数据时>
	if (beMarkedNbrs.size()==ui.pro_mrkTree->FileListModel()->rowCount())
	{
		this->allInputBtnEnable(false);
	}
	// <next 可用条件，暂时先注释掉，调试方便>
// 	if (_fileLauList->rowCount()<1||_fileMskList->rowCount()<1||_fileDivList->rowCount()<1)
// 	{
// 		ui.nextButton->setEnabled(false);
// 	}
// 	else
// 	{
// 		ui.nextButton->setEnabled(true);
// 	}
}
/// <summary>
/// <判断是否选中已经选过的数据>
/// </summary>
bool MarkDataForModel::judgeIfSelect(int _selectSrl)
{
	for (int ii=0;ii<beMarkedNbrs.size();ii++)
	{
		if (_selectSrl==beMarkedNbrs[ii])
		{
			QMessageBox::about(NULL,"Title","The image has been marked, please choose other images!");
			return false;
		}
	}
	return true;
}
/// <summary>
/// <左右移动数据按钮初始化>
/// </summary>
void MarkDataForModel::move2TextInitial()
{
	allInputBtnEnable(true);
	judgeListViewNull();
}
/// <summary>
/// <标记土地利用数据>
/// </summary>
void MarkDataForModel::move2TextlauToItem()
{
	// <土地利用数据>
	index=ui.pro_mrkTree->currentIndex();
	if (ui.pro_mrkTree->FileListModel()->item(index.row(),0)->rowCount()!=1)
	{
		QMessageBox::about(NULL,"Title","land use image must has only 1 band!");
		return;
	}
	bool _selecteditem=judgeIfSelect(index.row());
	if (_selecteditem==true)
	{
		lauSrlnum=index.row();
		beMarkedNbrs.append(lauSrlnum);

		_fileNode=new QStandardItem(ui.pro_mrkTree->FileListModel()->item(lauSrlnum,0)->text());
		_fileLauList->appendRow(_fileNode);
		_fileLauList->setItem(_fileLauList->rowCount()-1, _fileNode);

		ui.pro_mrkTree->FileListModel()->item(lauSrlnum,0)->setForeground(QBrush(QColor(255, 255, 255)));
		ui.pro_mrkTree->FileListModel()->item(lauSrlnum,0)->setSelectable(false);
	}
	this->allInputBtnEnable(false);// <不知道要不要注意这两条的顺序>
	this->judgeListViewNull();
}
/// <summary>
/// <标记掩模数据>
/// </summary>
void MarkDataForModel::move2TextmskToItem()
{
	// <掩模数据>
	index=ui.pro_mrkTree->currentIndex();
	if (ui.pro_mrkTree->FileListModel()->item(index.row(),0)->rowCount()!=1)
	{
		QMessageBox::about(NULL,"Title","mask image must has only 1 band!");
		return;
	}
	bool _selecteditem=judgeIfSelect(index.row());
	if (_selecteditem==true)
	{
		mskSrlnum=index.row();
		beMarkedNbrs.append(mskSrlnum);

		_fileNode=new QStandardItem(ui.pro_mrkTree->FileListModel()->item(mskSrlnum,0)->text());
		_fileMskList->appendRow(_fileNode);
		_fileMskList->setItem(_fileMskList->rowCount()-1, _fileNode);

		ui.pro_mrkTree->FileListModel()->item(mskSrlnum,0)->setForeground(QBrush(QColor(255, 255, 255)));
		ui.pro_mrkTree->FileListModel()->item(mskSrlnum,0)->setSelectable(false);
	}
	this->allInputBtnEnable(false);
	this->judgeListViewNull();
}
/// <summary>
/// <标记驱动力数据>
/// </summary>
void MarkDataForModel::move2TextdivToItem()
{
	// <驱动力数据>
	index=ui.pro_mrkTree->currentIndex();
	bool _selecteditem=judgeIfSelect(index.row());
	if (_selecteditem==true)
	{
		beMarkedNbrs.append(index.row());
		divSrlnum.append(index.row());
		int _whichnum=divSrlnum[divSrlnum.size()-1];

		_fileNode=new QStandardItem(ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->text());
		_fileDivList->appendRow(_fileNode);
		_fileDivList->setItem(_fileDivList->rowCount()-1, _fileNode);

		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setForeground(QBrush(QColor(255, 255, 255)));
		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setSelectable(false);
	}
	this->allInputBtnEnable(false);
	this->judgeListViewNull();
}
/// <summary>
/// <标记保护区数据>
/// </summary>
void MarkDataForModel::move2TextconToItem()
{
	// <驱动力数据>
	index=ui.pro_mrkTree->currentIndex();
	bool _selecteditem=judgeIfSelect(index.row());
	if (_selecteditem==true)
	{
		beMarkedNbrs.append(index.row());
		conSrlnum.append(index.row());
		int _whichnum=conSrlnum[conSrlnum.size()-1];

		_fileNode=new QStandardItem(ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->text());
		_fileConList->appendRow(_fileNode);
		_fileConList->setItem(_fileConList->rowCount()-1, _fileNode);

		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setForeground(QBrush(QColor(255, 255, 255)));
		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setSelectable(false);
	}
	this->allInputBtnEnable(false);
	this->judgeListViewNull();
}
/// <summary>
/// <标记概率调整区数据>
/// </summary>
void MarkDataForModel::move2TextajtToItem()
{
	// <驱动力数据>
	index=ui.pro_mrkTree->currentIndex();
	bool _selecteditem=judgeIfSelect(index.row());
	if (_selecteditem==true)
	{
		beMarkedNbrs.append(index.row());
		ajtSrlnum.append(index.row());
		int _whichnum=ajtSrlnum[ajtSrlnum.size()-1];

		_fileNode=new QStandardItem(ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->text());
		_fileAjtList->appendRow(_fileNode);
		_fileAjtList->setItem(_fileAjtList->rowCount()-1, _fileNode);

		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setForeground(QBrush(QColor(255, 255, 255)));
		ui.pro_mrkTree->FileListModel()->item(_whichnum,0)->setSelectable(false);
	}
	this->allInputBtnEnable(false);
	this->judgeListViewNull();
}
/// <summary>
/// <初始化逆标记按钮lau>
/// </summary>
void MarkDataForModel::lauItem2TreeIni()
{
	ui.lauBtnOut->setEnabled(true);
}
/// <summary>
/// <初始化逆标记按钮msk>
/// </summary>
void MarkDataForModel::mskItem2TreeIni()
{
	ui.mskBtnOut->setEnabled(true);
}
/// <summary>
/// <初始化逆标记按钮div>
/// </summary>
void MarkDataForModel::divItem2TreeIni()
{
	ui.divBtnOut->setEnabled(true);
}
/// <summary>
/// <初始化逆标记按钮con>
/// </summary>
void MarkDataForModel::conItem2TreeIni()
{
	ui.conBtnOut->setEnabled(true);
}
/// <summary>
/// <初始化逆标记按钮ajt>
/// </summary>
void MarkDataForModel::ajtItem2TreeIni()
{
	ui.ajtBtnOut->setEnabled(true);
}
/// <summary>
/// <逆标记土地利用数据>
/// </summary>
void MarkDataForModel::lauItem2Tree()
{
	// <土地利用数据>
	index=ui.listVlau->currentIndex();
	int _lau=index.row();
	for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
	{
		if (0 == QString::compare(this->_fileLauList->item(_lau,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
		{
			ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
			this->_fileLauList->removeRow(_lau);
			for (int jj=0;jj<beMarkedNbrs.size();jj++)
			{
				if (beMarkedNbrs[jj]==ii)
				{
					beMarkedNbrs.removeAt(jj);
				}
			}
			break;
		}
	}
	judgeListViewNull();
}
/// <summary>
/// <逆标记掩模数据>
/// </summary>
void MarkDataForModel::mskItem2Tree()
{
	// <掩模数据>
	index=ui.listVmsk->currentIndex();
	int _msk=index.row();
	for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
	{
		if (0 == QString::compare(this->_fileMskList->item(_msk,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
		{
			ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
			this->_fileMskList->removeRow(_msk);
			for (int jj=0;jj<beMarkedNbrs.size();jj++)
			{
				if (beMarkedNbrs[jj]==ii)
				{
					beMarkedNbrs.removeAt(jj);
				}
			}
			break;
		}
	}
	judgeListViewNull();
}
/// <summary>
/// <逆标记驱动力数据>
/// </summary>
void MarkDataForModel::divItem2Tree()
{
	// <驱动力数据>
	index=ui.listVdiv->currentIndex();
	int _div=index.row();
	for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
	{
		if (0 == QString::compare(this->_fileDivList->item(_div,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
		{
			ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
			this->_fileDivList->removeRow(_div);
			for (int jj=0;jj<beMarkedNbrs.size();jj++)
			{
				if (beMarkedNbrs[jj]==ii)
				{
					beMarkedNbrs.removeAt(jj);
				}
			}
			break;
		}
	}
	judgeListViewNull();
}
/// <summary>
/// <逆标记保护区数据>
/// </summary>
void MarkDataForModel::conItem2Tree()
{
	// <保护区数据>
	index=ui.listVcon->currentIndex();
	int _con=index.row();
	for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
	{
		if (0 == QString::compare(this->_fileConList->item(_con,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
		{
			ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
			this->_fileConList->removeRow(_con);
			for (int jj=0;jj<beMarkedNbrs.size();jj++)
			{
				if (beMarkedNbrs[jj]==ii)
				{
					beMarkedNbrs.removeAt(jj);
				}
			}
			break;
		}
	}
	judgeListViewNull();
}
/// <summary>
/// <逆标记概率调整区数据>
/// </summary>
void MarkDataForModel::ajtItem2Tree()
{
	// <概率调整区数据>
	index=ui.listVajt->currentIndex();
	int _ajt=index.row();
	for (int ii=0;ii<ui.pro_mrkTree->FileListModel()->rowCount();ii++)
	{
		if (0 == QString::compare(this->_fileAjtList->item(_ajt,0)->text(),ui.pro_mrkTree->FileListModel()->item(ii,0)->text(), Qt::CaseInsensitive))
		{
			ui.pro_mrkTree->FileListModel()->item(ii,0)->setForeground(QBrush(QColor(0, 0, 0)));
			this->_fileAjtList->removeRow(_ajt);
			for (int jj=0;jj<beMarkedNbrs.size();jj++)
			{
				if (beMarkedNbrs[jj]==ii)
				{
					beMarkedNbrs.removeAt(jj);
				}
			}
			break;
		}
	}
	judgeListViewNull();
}
/// <summary>
/// <关闭窗口>
/// </summary>
void MarkDataForModel::winClose()
{
	this->close();
}
/// <summary>
/// <返回标记数据>
/// </summary>
void MarkDataForModel::whenfinishclicked()
{
	emit sandallmarkdata(lauSrlnum,mskSrlnum,divSrlnum,conSrlnum,ajtSrlnum);
	this->winClose();
}






















