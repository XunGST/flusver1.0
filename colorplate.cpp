#include "geosimulator.h"
#include "dynasimulation.h"
#include "colorplate.h"
#include "TiffDataRead.h"
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <iostream>
#include <QSignalMapper>
#include <QColorDialog>
using namespace std;

////////////////////////////为动态模拟重载//////////////////////////////////////
ColorPlate::ColorPlate(QWidget *parent,DynaSimulation* _dsl)
{
	ui.setupUi(this); 

	/// <随机数产生随机种子>
	time_t t = time(NULL); 
	srand(t);

	QIcon neibicon(":/new/prefix1/邻域.png");
	this->setWindowIcon(neibicon);


	m_dsl=_dsl;

	ui.ColorTableWidget->setRowCount(m_dsl->rgbLanduseType2.size()); // <设置行数> 
	ui.ColorTableWidget->setColumnCount(5); // <设置列数为5> 
	ui.ColorTableWidget->horizontalHeader()->setClickable(false); // <设置表头不可点击（默认点击后进行排序）>
	QStringList header;
	header<<tr("Type Number")<<tr("In Valid")<<tr("Type Name")<<tr("Color Select")<<tr("Color");
	ui.ColorTableWidget->setHorizontalHeaderLabels(header);
	ui.ColorTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);// <设置充满表宽度>
	ui.ColorTableWidget->resizeColumnToContents (0);
	QFont font = ui.ColorTableWidget->horizontalHeader()->font(); // <加粗>
	font.setBold(true);
	ui.ColorTableWidget->horizontalHeader()->setFont(font);
	this->showCountlist();
	this->loadconfigfile();

	connect(ui.sureBtn,SIGNAL(clicked()),this,SLOT(finished()));
	connect(ui.cancelBtn,SIGNAL(clicked()),this,SLOT(closeWin()));
}
//
// <重载>
//
////////////////////////////概率计算重载//////////////////////////////////////
ColorPlate::ColorPlate( QWidget *parent,GeoSimulator* _gsr,bool patten )
{
	ui.setupUi(this); 

    //ui.loadconfigbtn->setVisible(false);

	QIcon probicon(":/new/prefix1/概率.png");
	this->setWindowIcon(probicon);

	m_gslr=_gsr;

	ui.colorlabel->setText("Please select 1 or 0 'NoData Value'.");
	ui.ColorTableWidget->setRowCount(m_gslr->rgbLanduseType.size()); // <设置行数> 
	ui.ColorTableWidget->setColumnCount(4); // <设置列数为4> 
	ui.ColorTableWidget->horizontalHeader()->setClickable(false); // <设置表头不可点击（默认点击后进行排序）>
	QStringList header;
	header<<tr("Type Number")<<tr("In Valid")<<tr("NoData Label")<<tr("Pixel statistics");
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
ColorPlate::~ColorPlate()
{

}


////////////////////////////概率计算//////////////////////////////////////
/// <summary>
/// <加载信息列表>
/// </summary>
bool ColorPlate::probabilityShowCountlist()
{
	QSignalMapper* comboMapper = new QSignalMapper(this);
	if (m_gslr->rgbLanduseType.size()!=0)
	{
		for (int ii=0;ii<m_gslr->rgbLanduseType.size();ii++)
		{
			QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(m_gslr->rgbLanduseType[ii]));
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

			QTableWidgetItem* _tmp3=new QTableWidgetItem(QString::number(m_gslr->staCount[ii]));
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
void ColorPlate::resetInvalidValue( QString position )
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
		for (int ii=0;ii<m_gslr->rgbLanduseType.size();ii++)
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
		for (int ii=0;ii<m_gslr->rgbLanduseType.size();ii++)
		{
			L_comBox[ii]->setEnabled(true);
		}
	}
}
/// <summary>
/// <设置完成>
/// </summary>
void ColorPlate::refinished()
{

	int _size=ui.ColorTableWidget->rowCount();

	for(int ii=0;ii<_size;ii++)
	{
		QString str=ui.ColorTableWidget->item(ii,2)->text();
		/// <剔除nodatavalue的相关参数>
		if (0==QString::compare(str,"NoData Value", Qt::CaseInsensitive))
		{
			m_gslr->nodatavalue=m_gslr->rgbLanduseType.at(ii);// only 1 for most
			m_gslr->nodataexit=true;
			m_gslr->rgbLanduseType.removeAt(ii);
		}
		else
		{
			m_gslr->nodataexit=false;
		}
		/// <剔除nodatavalue的相关参数>
	}
	this->closeWin();
}

////////////////////////////为动态模拟重载//////////////////////////////////////
/// <summary>
/// <加载列表>
/// </summary>
bool ColorPlate::showCountlist()
{

	QSignalMapper* comboMapper = new QSignalMapper(this);
	QSignalMapper* btnMapper = new QSignalMapper(this);

	if (m_dsl->rgbLanduseType2.size()!=0)
	{
		for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
		{
			QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(m_dsl->rgbLanduseType2[ii]));
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

			QTableWidgetItem* _tmp2=new QTableWidgetItem(tr(""));
			_tmp2->setTextAlignment(Qt::AlignCenter);
			ui.ColorTableWidget->setItem(ii,2,_tmp2); 

			QPushButton *btn = new QPushButton(this); 
			btn->setText(tr("Set Color"));
			ui.ColorTableWidget->setCellWidget(ii,3,btn); 
			L_btn.append(btn);
			connect(L_btn[ii], SIGNAL(clicked()), btnMapper, SLOT(map()));
			btnMapper->setMapping(L_btn[ii], QString("%1-%2").arg(ii).arg(1));

			QTableWidgetItem* _tmp4=new QTableWidgetItem();
			_tmp4->setFlags(_tmp4->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
			ui.ColorTableWidget->setItem(ii,4,_tmp4); 


			double rdmData=(double)rand()/(double)RAND_MAX;
			int _r=255*rdmData;
			double rdmData1=(double)rand()/(double)RAND_MAX;
			int _g=255*rdmData1;
			double rdmData2=(double)rand()/(double)RAND_MAX;
			int _b=255*rdmData2;
			_tmp4->setBackgroundColor(QColor(_r,_g,_b));


			//_tmp4->setBackgroundColor(QColor(255,255,255));
		}
		connect(comboMapper, SIGNAL(mapped(const QString &)),
			this, SLOT(setInvalidValue(const QString &))); 
		connect(btnMapper, SIGNAL(mapped(const QString &)),
			this, SLOT(setColor(const QString &))); 
	}
	else
	{
		QMessageBox::warning(this,"Error","Too many values, please choose another image!");
		return false;
	}
	return true;
}
/// <summary>
/// <设定nodata>
/// </summary>
void ColorPlate::setInvalidValue( QString position)
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
		L_btn[row]->setEnabled(false);

		// <仅支持一个nodata>
		for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
		{
			if (ii!=row)
			{
				L_comBox[ii]->setEnabled(false);
			}
		}
		
		QTableWidgetItem* _tmp4=new QTableWidgetItem(tr(""));
		_tmp4->setFlags(_tmp4->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
		ui.ColorTableWidget->setItem(row,4,_tmp4);  
		_tmp4->setBackgroundColor(QColor(255,255,255));

	}
	else
	{
		QTableWidgetItem* _tmp2=new QTableWidgetItem(tr(""));
		_tmp2->setTextAlignment(Qt::AlignCenter);
		ui.ColorTableWidget->setItem(row,2,_tmp2); 
		L_btn[row]->setEnabled(true);

		// <仅支持一个nodata>
		for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
		{
			L_comBox[ii]->setEnabled(true);
		}

		QTableWidgetItem* _tmp4=new QTableWidgetItem(tr(""));
		_tmp4->setFlags(_tmp4->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
		ui.ColorTableWidget->setItem(row,4,_tmp4);  
		_tmp4->setBackgroundColor(QColor(255,255,255));
	}

}
/// <summary>
/// <设定地类显示颜色>
/// </summary>
void ColorPlate::setColor( QString position )
{
	QStringList row_col = position .split("-");
	int row = row_col [0].toInt();
	int col = row_col [1].toInt();

	QColor color = QColorDialog::getColor(Qt::white, this);



	QTableWidgetItem* _tmp4=new QTableWidgetItem(tr(""));
	_tmp4->setFlags(_tmp4->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
	ui.ColorTableWidget->setItem(row,4,_tmp4);  
	_tmp4->setBackgroundColor(color);
}
/// <summary>
/// <设定完成>
/// </summary>
void ColorPlate::finished()
{

	int _size=ui.ColorTableWidget->rowCount();

	for(int ii=0;ii<_size;ii++)
	{
		QColor clr=ui.ColorTableWidget->item(ii,4)->backgroundColor();
		QString str=ui.ColorTableWidget->item(ii,2)->text();

		/// <剔除nodatavalue的相关参数>
		if (str.isEmpty()==false&&(0!=QString::compare(str,"NoData Value", Qt::CaseInsensitive)))
		{
			m_dsl->lauTypeName2.append(ui.ColorTableWidget->item(ii,2)->text());
		}
		if (str.isEmpty()==true&&(0!=QString::compare(str,"NoData Value", Qt::CaseInsensitive)))
		{
			m_dsl->lauTypeName2.append(tr("Landuse")+QString::number(ii));
		}

		if (0!=QString::compare(str,"NoData Value", Qt::CaseInsensitive))
		{
			m_dsl->rgbType2.append(clr);
		}

		if (0==QString::compare(str,"NoData Value", Qt::CaseInsensitive))
		{
			m_dsl->rgbLanduseType2.removeAt(ii);
			m_dsl->staCount2.removeAt(ii);
		}
		/// <剔除nodatavalue的相关参数>
	}
	m_dsl->isfinished=true;
	m_dsl->input_Future_Land_Area();
	this->buildconfigfile();
	this->closeWin();
}

/// <summary>
/// <建立配置文件>
/// </summary>
void ColorPlate::buildconfigfile()
{
	QFile file("config.txt");
	if(file.open(QFile::WriteOnly | QIODevice::Text));
	{
		QTextStream out(&file);
		out << "[Index, Land Use Type, R, G, B]" << "\n";

		for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
		{
			out<<m_dsl->rgbLanduseType2[ii]<<",";
			out<<m_dsl->lauTypeName2[ii]<<",";
			out<<m_dsl->rgbType2[ii].red()<<",";
			out<<m_dsl->rgbType2[ii].green()<<",";
			out<<m_dsl->rgbType2[ii].blue()<<"\n";
		}
	}
	file.close();
}

/// <summary>
/// <自动读取配置文件>
/// </summary>
void ColorPlate::loadconfigfile()
{
	QFile file("config.txt");
	file.open(QFile::ReadOnly | QIODevice::Text);
	if (file.exists()==true)
	{
		QTextStream in(&file);
		QString line;
		QStringList linelist;
		line = in.readLine();// <跳过第一行>
		while(!in.atEnd()) {
			line = in.readLine();
			linelist=line.split(",");
			for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
			{
				if (m_dsl->rgbLanduseType2[ii]==linelist[0].toInt())
				{
					ui.ColorTableWidget->item(ii,2)->setText(linelist[1]);
					QColor color(linelist[2].toInt(),linelist[3].toInt(),linelist[4].toInt());
					ui.ColorTableWidget->item(ii,4)->setBackgroundColor(color);
				}
			}
		}

		for (int ii=0;ii<m_dsl->rgbLanduseType2.size();ii++)
		{
			QString str=ui.ColorTableWidget->item(ii,2)->text();
			if (str.isEmpty()==true)
			{
				L_comBox[ii]->setCurrentIndex(1);
				break;
			}
		}
	}
	file.close();
}
////////////////////////////公用关闭窗口//////////////////////////////////////
/// <summary>
/// <完成后关闭窗口>
/// </summary>
void ColorPlate::closeWin()
{
	this->close();
}










