#include "geosimulator.h"
#include "dynasimulation.h"
#include "TrainPrepare.h"
#include "TiffDataRead.h"
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <iostream>
#include <QSignalMapper>
#include <QColorDialog>
using namespace std;


////////////////////////////概率计算重载//////////////////////////////////////
TrainPrepare::TrainPrepare( QWidget *parent)
{
	ui.setupUi(this); 

	QIcon neibicon(":/new/prefix1/邻域.png");
	this->setWindowIcon(neibicon);
	this->setWindowTitle(tr("Set NoData Value"));

	QFile tmpfile("./FilesGenerate/file.tmp");
	if (!tmpfile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&tmpfile);
	while (!in.atEnd()) {
		mlRgbLanduseType.append(in.readLine().toInt());
		mlStastisticCount.append(in.readLine().toInt());
	}

	tmpfile.close();

	remove("./FilesGenerate/file.tmp");

	ui.ColorTableWidget->setRowCount(mlRgbLanduseType.size()); // <设置行数> 
	ui.ColorTableWidget->setColumnCount(4); // <设置列数为4> 
	ui.ColorTableWidget->horizontalHeader()->setClickable(false); // <设置表头不可点击（默认点击后进行排序）>
	QStringList header;
	header<<tr("Land Use Code")<<tr("NoData Option")<<tr("NoData Label")<<tr("Pixel Statistics");
	ui.ColorTableWidget->setHorizontalHeaderLabels(header);
	ui.ColorTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);// <设置充满表宽度>
	ui.ColorTableWidget->resizeColumnToContents (0);
	QFont font = ui.ColorTableWidget->horizontalHeader()->font(); // <加粗>
	font.setBold(true);
	ui.ColorTableWidget->horizontalHeader()->setFont(font);
	this->probabilityShowCountlist();

	connect(ui.sureBtn,SIGNAL(clicked()),this,SLOT(refinished()));
	connect(ui.cancelBtn,SIGNAL(clicked()),this,SLOT(closeWin()));
}


TrainPrepare::~TrainPrepare()
{
	this->closeWin();
}


////////////////////////////概率计算//////////////////////////////////////
/// <summary>
/// <加载信息列表>
/// </summary>
bool TrainPrepare::probabilityShowCountlist()
{
	QSignalMapper* comboMapper = new QSignalMapper(this);
	if (mlRgbLanduseType.size()!=0)
	{
		for (int ii=0;ii<mlRgbLanduseType.size();ii++)
		{
			QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(mlRgbLanduseType[ii]));
			_tmp0->setTextAlignment(Qt::AlignCenter);
			_tmp0->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
			ui.ColorTableWidget->setItem(ii,0,_tmp0); 

			QComboBox* comBox= new QComboBox(this); 
			comBox->addItem(QWidget::tr("Valid Data")); 
			comBox->addItem(QWidget::tr("NoData Value")); 
			comBox->setCurrentIndex(0);
			L_comBox.append(comBox);
			ui.ColorTableWidget->setCellWidget(ii,1,L_comBox[ii]);  
			connect(L_comBox[ii], SIGNAL(currentIndexChanged (int)), comboMapper, SLOT(map()));
			comboMapper->setMapping(L_comBox[ii], QString("%1-%2").arg(ii).arg(1));

			QTableWidgetItem* _tmp2=new QTableWidgetItem(tr("Valid Data"));
			_tmp2->setFlags(_tmp2->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
			_tmp2->setTextAlignment(Qt::AlignCenter);
			ui.ColorTableWidget->setItem(ii,2,_tmp2); 

			QTableWidgetItem* _tmp3=new QTableWidgetItem(QString::number(mlStastisticCount[ii]));
			_tmp3->setTextAlignment(Qt::AlignCenter);
			_tmp3->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
			ui.ColorTableWidget->setItem(ii,3,_tmp3); 
		}
		connect(comboMapper, SIGNAL(mapped(const QString &)),
			this, SLOT(resetInvalidValue(const QString &)));  
	}
	else
	{
		QMessageBox::warning(this,"Error","Too many values, please choose another image!");
		return false;
	}
	return true;
}
/// <summary>
/// <手动设定nodata>
/// </summary>
void TrainPrepare::resetInvalidValue( QString position )
{
	QStringList row_col = position .split("-");
	int row = row_col [0].toInt();
	int col = row_col [1].toInt();

	int cindex=L_comBox[row]->currentIndex();

	if (cindex==1)
	{
		QTableWidgetItem* _tmp2=new QTableWidgetItem(tr("NoData Value"));
		_tmp2->setFlags(_tmp2->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
		_tmp2->setTextAlignment(Qt::AlignCenter);
		_tmp2->setBackgroundColor(QColor(100,100,100));
		_tmp2->setTextColor(QColor(255,0,0));
		ui.ColorTableWidget->setItem(row,2,_tmp2);

		// <仅支持一个nodata>
		for (int ii=0;ii<mlRgbLanduseType.size();ii++)
		{
			if (ii!=row)
			{
				L_comBox[ii]->setEnabled(false);
			}
		}
	}
	else
	{
		QTableWidgetItem* _tmp2=new QTableWidgetItem(tr("Valid Data"));
		_tmp2->setTextAlignment(Qt::AlignCenter);
		ui.ColorTableWidget->setItem(row,2,_tmp2); 

		// <仅支持一个nodata>
		for (int ii=0;ii<mlRgbLanduseType.size();ii++)
		{
			L_comBox[ii]->setEnabled(true);
		}
	}
}
/// <summary>
/// <设置完成>
/// </summary>
void TrainPrepare::refinished()
{
	// <新建log文件保存导入路径>
	QFile logfile("./FilesGenerate/logFileTrain.log");

	if (!logfile.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug()<<"Load File 'logFileTrain.log' Error!";
		return;
	}

	QTextStream in(&logfile);

	in << tr("[NoData Value]").trimmed() <<"\n";

	int _size=ui.ColorTableWidget->rowCount();

	for(int ii=0;ii<_size;ii++)
	{
		QString str=ui.ColorTableWidget->item(ii,2)->text();
		/// <剔除nodatavalue的相关参数>
		if (0==QString::compare(str,"NoData Value", Qt::CaseInsensitive))
		{
			in<<QString::number(mlRgbLanduseType.at(ii))<<endl;
			mlRgbLanduseType.removeAt(ii);
			mlStastisticCount.removeAt(ii);
		}
	}

	if (mlRgbLanduseType.size()==_size)
	{
		in<<tr("No NoData Value").trimmed()<<endl;
	}

	logfile.close();

	buildconfigfile();

	sendWinClose();

	this->closeWin();
}


/// <summary>
/// <建立配置文件>
/// </summary>
void TrainPrepare::buildconfigfile()
{
	QFile file("./FilesGenerate/config_nodata.tmp");
	if(file.open(QFile::WriteOnly | QIODevice::Text));
	{
		QTextStream out(&file);
		out << "[Index, Count]" << "\n";

		for (int ii=0;ii<mlRgbLanduseType.size();ii++)
		{
			out<<mlRgbLanduseType[ii]<<",";
			out<<mlStastisticCount[ii]<<"\n";
		}
	}
	file.close();
}
/// <summary>
/// <完成后关闭窗口>
/// </summary>
void TrainPrepare::closeWin()
{
	this->close();
}










