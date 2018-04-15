#include "dynasimulation.h"
#include "mapshowviewer.h"
#include "showproperty.h"
#include "geodpcasys.h"
#include "TiffDataRead.h"
#include <iostream>
#include <QtGui/QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include "PixCal.h"
#include "pixcalthread.h"
#include "geodpcasys.h"
#include "colorplate.h"
#include "nntrain.h"
#include "anntrainthread.h"
#include "simulationprocess.h"
#include "simuthread.h"
#include "qmath.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_item.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#include "qwt_legend.h"  
#include "qwt_legend_label.h"
#include "qwt_plot_panner.h"
#include <QLabel>

using namespace std;

DynaSimulation::DynaSimulation(GeoDpCAsys* _gdp)
{
	ui.setupUi(this);

	this->setWindowTitle("Dynamic Simulation Module");
	this->setAttribute(Qt::WA_DeleteOnClose);

	QIcon neibicon(":/new/prefix1/邻域.png");
	this->setWindowIcon(neibicon);

	QIcon qicons(":/new/prefix1/开始.png");
	ui.btnRun->setIcon(qicons);
	ui.btnRun->setEnabled(false);

	QIcon qiconf(":/new/prefix1/还原.png");
	ui.btnFit->setIcon(qiconf);

	QIcon qiconstop(":/new/prefix1/停止.png");
	ui.btnStop->setIcon(qiconstop);
	ui.btnStop->setEnabled(false);


	_landuse2=NULL;
	u_rgbshow=NULL;
	u_rgb=NULL;

	m_gdp2=_gdp;

	isfinished=false;
	xlength=0;


	ui.laubtnColor2->setEnabled(false);
	ui.laulineEdit2->setReadOnly(true);
	ui.inProbabilitylineEdit->setReadOnly(true);
	ui.saveSimlineEdit->setReadOnly(true);
	ui.norstradioButton->setChecked(true);
	//ui.radioConvention->setChecked(true);
	ui.rstlabel->setEnabled(false);
	ui.rstlineEdit->setReadOnly(true);
	ui.rstlineEdit->setEnabled(false);
	ui.rstbtn->setEnabled(false);
	ui.itelineEdit->setText(tr("80"));
	ui.deglineEdit->setText(tr("0.1"));
	ui.CAneigh->setMinimum(1);
	ui.CAneigh->setSingleStep(2);
	ui.CAneigh->setValue(3);

	ui.tabWidget->removeTab(1);
	ui.tabWidget->removeTab(0);

	ui.qwtPlot->setAxisTitle(QwtPlot::xBottom,"Iterations");
	ui.qwtPlot->setAxisTitle(QwtPlot::yLeft,"Number");
	int num = 10000;
	ui.qwtPlot->setAxisScale(ui.qwtPlot->xBottom, 0, ui.itelineEdit->text().toInt());
	ui.qwtPlot->setAxisScale(ui.qwtPlot->yLeft, 0,  num);
	//ui.qwtPlot->insertLegend(new QwtLegend(), QwtPlot::LeftLegend );
	(void) new QwtPlotPanner(ui.qwtPlot->canvas());


	futuretableWidget=new QTableWidget(this);
	ui.tabWidget->addTab(futuretableWidget,tr("Future Land Area"));

	restricttableWidget=new QTableWidget(this);
	ui.tabWidget->addTab(restricttableWidget,tr("Restricted Matrix"));

	switchcost=new QTableWidget(this);
	ui.tabWidget->addTab(switchcost,tr("Cost Matrix"));

	mqIntenofneigh=new QTableWidget(this);
	ui.tabWidget->addTab(mqIntenofneigh,tr("Intensity of neigh"));


	connect(ui.laubtn2,SIGNAL(clicked()),this,SLOT(openLauFile2()));
	connect(ui.laubtnColor2,SIGNAL(clicked()),this,SLOT(setLauColor2()));
	connect(ui.inProbabilityBtn,SIGNAL(clicked()),this,SLOT(openProbFile2()));
	connect(ui.saveSimBtn,SIGNAL(clicked()),this,SLOT(saveSimScenario()));
	connect(ui.rstbtn,SIGNAL(clicked()),this,SLOT(openRestFile()));

	// <模式选择>
	connect(ui.norstradioButton,SIGNAL(toggled(bool)),this,SLOT(modeSelect()));
//	connect(ui.radioConvention,SIGNAL(toggled(bool)),this,SLOT(modeSelect()));

	connect(ui.finishGeoButton,SIGNAL(clicked()),this,SLOT(inisimmulate()));
	connect(ui.btnRun,SIGNAL(clicked()),this,SLOT(startsimulate()));
	connect(ui.btnStop,SIGNAL(clicked()),this,SLOT(stopmodel()));
	connect(ui.btnFit, SIGNAL(clicked()), ui.pro_dynagraphicsView, SLOT( ZoomFit() ) );


}

DynaSimulation::~DynaSimulation()
{
	this->closeDynaSimulation();
}
///
/// <土地利用数据>
///
bool DynaSimulation::lauLoadImage2( QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	lauSerialNum2=lauNumImage2;

	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	lau_poDataset2.append(pread);

	// <提取数据>
	if (!lau_poDataset2[lauSerialNum2]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	lauNumImage2=lau_poDataset2.size();// <用于记录总波段数>
	nodatavalue2=lau_poDataset2[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	return true;
}
/// <summary>
/// <打开>
/// </summary>
void DynaSimulation::openLauFile2()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		lauClearall2();
		lauLoadImage2(&fileName);
		if (lau_poDataset2.at(0)->bandnum()!=1)
		{
			this->lauClearall2(); 
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.laulineEdit2->setText(fileName);
		ui.laubtnColor2->setEnabled(true);
	}
}
/// <summary>
/// <清除>
/// </summary>
void DynaSimulation::lauClearall2()
{
	if (lau_poDataset2.size()!=0)
	{
		if (lau_poDataset2[0]!=NULL)
		{
			lau_poDataset2[0]->close();
			delete lau_poDataset2[0];
		}
	}
	lau_poDataset2.clear();
	lauNumImage2=0;
	lauSerialNum2=0;
}
/// <summary>
/// <输入颜色>
/// </summary>
void DynaSimulation::setLauColor2()
{
	/// <线程>
	/// <开辟空间>
	int _height=lau_poDataset2.at(0)->rows();

	int _width=lau_poDataset2.at(0)->cols();

	this->_landuse2=new unsigned char[_height*_width];
	/// <开辟空间>

	PixCal* dynapcl=new PixCal(this,false);// false no sense

	PixCalThread* dynaptd=new PixCalThread(this,dynapcl);

	dynapcl->moveToThread(dynaptd);

	dynaptd->start();


	if (dynaptd->isRunning())
	{
		QEventLoop loop1;
		connect(dynaptd, SIGNAL(finished()), &loop1, SLOT(quit()));
		loop1.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Too many land use types!");
		return;
	}

	ColorPlate* dynacple=new ColorPlate(this,this);/// <可以重载>

	dynacple->show();

}
///
/// <概率数据>
///
bool DynaSimulation::probLoadImage2( QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	probSerialNum2=probNumImage2;

	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	prob_poDataset2.append(pread);

	// <提取数据>
	if (!prob_poDataset2[probSerialNum2]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	probNumImage2=prob_poDataset2.size();// <用于记录总波段数>
	return true;
}
/// <summary>
/// <清除概率数据>
/// </summary>
void DynaSimulation::probClearall2()
{
	if (prob_poDataset2.size()!=0)
	{
		if (prob_poDataset2[0]!=NULL)
		{
			prob_poDataset2[0]->close();
			delete prob_poDataset2[0];
		}
	}
	prob_poDataset2.clear();
	probNumImage2=0;
	probSerialNum2=0;
}
/// <summary>
/// <打开概率数据>
/// </summary>
void DynaSimulation::openProbFile2()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		probClearall2();
		probLoadImage2(&fileName);
		ui.inProbabilitylineEdit->setText(fileName);
	}
}
///
/// <保存>
///
void DynaSimulation::saveSimScenario()
{
	QString _savefilename = QFileDialog::getSaveFileName(
		this,
		tr( "Input a image name to save..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif)" ) );
	if (!_savefilename.isNull())
	{
		ui.saveSimlineEdit->setText(_savefilename);
		ui.finishGeoButton->setEnabled(true);
	}
}
///
/// <限制转换区域>
///
bool DynaSimulation::restLoadImage( QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	restSerialNum=restNumImage;

	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	rest_poDataset.append(pread);

	// <提取数据>
	if (!rest_poDataset[restSerialNum]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	restNumImage=rest_poDataset.size();// <用于记录总波段数>
	return true;
}
/// <summary>
/// <清除限制转换区域>
/// </summary>
void DynaSimulation::restClearall()
{
	if (rest_poDataset.size()!=0)
	{
		if (rest_poDataset[0]!=NULL)
		{
			rest_poDataset[0]->close();
			delete rest_poDataset[0];
		}
	}
	rest_poDataset.clear();
	restNumImage=0;
	restSerialNum=0;
	restexit=false;
}
/// <summary>
/// <打开限制转换区域>
/// </summary>
void DynaSimulation::openRestFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		restClearall();
		restLoadImage(&fileName);
		if (rest_poDataset.at(0)->bandnum()!=1)
		{
			this->restClearall();
			QMessageBox::warning(this,"Error","Restrict data has only one band!");
			return;
		}
		ui.rstlineEdit->setText(fileName);
		restexit=true;
	}
}
///
/// <策略选择>
///
void DynaSimulation::modeSelect()
{
	if (ui.norstradioButton->isChecked()==true)
	{
		ui.rstlabel->setEnabled(false);
		ui.rstlineEdit->clear();
		ui.rstlineEdit->setEnabled(false);
		ui.rstbtn->setEnabled(false);
	}
	else
	{
		ui.rstlabel->setEnabled(true);
		//ui.rstlineEdit->clear();
		ui.rstlineEdit->setEnabled(true);
		ui.rstbtn->setEnabled(true);
	}
// 	if (ui.radioConvention->isChecked()==true)
// 	{
// 		ui.deglabel->setEnabled(true);
// 		ui.deglabel->setText(tr("Degree of Freedom (0 - 1)"));
// 		ui.deglineEdit->setText(tr("1"));
// 		ui.deglineEdit->setEnabled(true);
// 	}
// 	else
// 	{
// 		ui.deglabel->setEnabled(false);
// 		ui.deglabel->setText(tr("Degree of Freedom (random)"));
// 		ui.deglineEdit->clear();
// 		ui.deglineEdit->setEnabled(false);
// 	}
}
///
/// <初始化输入目标数量表格>
///
void DynaSimulation::input_Future_Land_Area()
{
	if (isfinished==true)
	{
		// <第一个>
		futuretableWidget->setRowCount(2); // <设置行数> 
		futuretableWidget->setColumnCount(rgbLanduseType2.size()); // <设置列数> 
		QStringList h_header;
		QStringList v_header;
		for (int i=0;i<rgbLanduseType2.size();i++)
		{
			h_header<<lauTypeName2.at(i);

			QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(staCount2[i]));
			_tmp0->setTextAlignment(Qt::AlignCenter);
			_tmp0->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
			futuretableWidget->setItem(0,i,_tmp0); 
			
			QTableWidgetItem* _tmp1=new QTableWidgetItem();
			_tmp1->setTextAlignment(Qt::AlignCenter);
			futuretableWidget->setItem(1,i,_tmp1); 
		}
		v_header<<tr("Initial Pixel Number")<<tr("Future Pixel Number");
		futuretableWidget->setHorizontalHeaderLabels(h_header);
		futuretableWidget->setVerticalHeaderLabels(v_header);
		//ui.futuretableWidget->verticalHeader()->setResizeMode(QHeaderView::Stretch); 

		// <第二个>
		restricttableWidget->setRowCount(rgbLanduseType2.size()); // <设置行数> 
		restricttableWidget->setColumnCount(rgbLanduseType2.size()); // <设置列数> 
		for (int i=0;i<rgbLanduseType2.size();i++)
		{
			for (int j=0;j<rgbLanduseType2.size();j++)
			{
				QTableWidgetItem* _tmp1=new QTableWidgetItem(QString::number(1));
				_tmp1->setTextAlignment(Qt::AlignCenter);
				if (i==j)
				{
					_tmp1->setFlags(_tmp1->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
				}
				restricttableWidget->setItem(i,j,_tmp1);
			}
		}
		restricttableWidget->setHorizontalHeaderLabels(h_header);
		restricttableWidget->setVerticalHeaderLabels(h_header);
		//ui.restricttableWidget->verticalHeader()->setResizeMode(QHeaderView::Stretch);

		// <第三个>
		switchcost->setRowCount(rgbLanduseType2.size()); // <设置行数> 
		switchcost->setColumnCount(rgbLanduseType2.size()); // <设置列数> 
		for (int i=0;i<rgbLanduseType2.size();i++)
		{
			for (int j=0;j<rgbLanduseType2.size();j++)
			{
				QTableWidgetItem* _tmp2=new QTableWidgetItem(QString::number(1));
				_tmp2->setTextAlignment(Qt::AlignCenter);
				if (i==j)
				{
					_tmp2->setFlags(_tmp2->flags() & (~Qt::ItemIsEditable)); // <不可编辑>
				}
				switchcost->setItem(i,j,_tmp2);
			}
		}
		switchcost->setHorizontalHeaderLabels(h_header);
		switchcost->setVerticalHeaderLabels(h_header);
		//switchcost->verticalHeader()->setResizeMode(QHeaderView::Stretch);

		// <第四个>
		mqIntenofneigh->setRowCount(1); // <设置行数> 
		mqIntenofneigh->setColumnCount(rgbLanduseType2.size()); // <设置列数> 
		QStringList h_header_1;
		QStringList v_header_1;
		for (int i=0;i<rgbLanduseType2.size();i++)
		{
			h_header_1<<lauTypeName2.at(i);

			QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(1));
			_tmp0->setTextAlignment(Qt::AlignCenter);
			mqIntenofneigh->setItem(0,i,_tmp0); 
		}
		v_header_1<<tr("Intensity of neighborhood");
		mqIntenofneigh->setHorizontalHeaderLabels(h_header_1);
		mqIntenofneigh->setVerticalHeaderLabels(v_header_1);
	}

	this->loadconfig();
}
///
/// <模拟初始化>
///
void DynaSimulation::inisimmulate()
{
	QVBoxLayout *layout = new QVBoxLayout; 
	for (int ii=0;ii<rgbLanduseType2.size();ii++)
	{
		QLabel* _label=new QLabel(lauTypeName2[ii]);
		_label->setFrameStyle(QFrame::Panel | QFrame::Raised);  
		
		_label->setAutoFillBackground(true);
		QPalette palette;
		palette.setColor(QPalette::Background,rgbType2[ii]);
		_label->setPalette(palette);
		_label->setLineWidth(2); 
		layout->addWidget(_label); 
	}
	ui.frame_lengend->setLayout(layout);

	ui.btnRun->setEnabled(true);
	ui.btnStop->setEnabled(true);

	ui.finishGeoButton->setEnabled(false);
	slp=new SimulationProcess(this);

}
///
/// <模拟开始>
///
void DynaSimulation::startsimulate()
{

	ui.btnRun->setEnabled(false);

	this->appendconfig();

	SimuThread* std=new SimuThread(this,slp);

	slp->moveToThread(std);// <这句似乎不加也可以>

	std->start();

	if (rgbLanduseType2.size()!=0&&std->isRunning())
	{
		QEventLoop loop;
		connect(std, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in simulation!");
		return;
	}

	QMessageBox::information(this,"Message","Simulation has finished!");


	this->getParameter(tr("Finished Simulating!"));

	this->write2file();

	this->closeDynaSimulation();

	this->ui.btnRun->setEnabled(false);


}

/// <summary>
/// <追加参数配置>
/// <\summary>
void DynaSimulation::appendconfig()
{
	QFile file("config1.txt");

	if(file.open(QFile::WriteOnly|QIODevice::Text));
	{
		QTextStream out(&file);

		out << "[Future Pixels]" << "\n";

		for (int ii=0;ii<rgbLanduseType2.size();ii++)
		{
			out<<futuretableWidget->item(1,ii)->text().trimmed()<<"\n";
		}

		out<<tr("[Restricted Matrix]")<<"\n";

		for (int ii=0;ii<rgbLanduseType2.size();ii++)
		{
			QString str;
			for (int jj=0;jj<rgbLanduseType2.size();jj++)
			{
				str+=restricttableWidget->item(ii,jj)->text().trimmed()+",";
			}
			out<<str.left(str.length() - 1)<<"\n";
		}

		out<<tr("[Cost Matrix]")<<"\n";

		for (int ii=0;ii<rgbLanduseType2.size();ii++)
		{
			QString str;
			for (int jj=0;jj<rgbLanduseType2.size();jj++)
			{
				str+=switchcost->item(ii,jj)->text().trimmed()+",";
			}
			out<<str.left(str.length() - 1)<<"\n";
		}

		out << "[Intensity of neighborhood]" << "\n";

		for (int ii=0;ii<rgbLanduseType2.size();ii++)
		{
			out<<mqIntenofneigh->item(0,ii)->text().trimmed()<<"\n";
		}
	}
	file.close();
}
/// <summary>
/// <读取参数配置>
/// <\summary>
void DynaSimulation::loadconfig()
{
	QFile file("config1.txt");
	file.open(QFile::ReadOnly|QIODevice::Text);
	if (file.exists()==true)
	{
		QTextStream in(&file);
		QString str;
		QStringList strlist;
		while(!in.atEnd())
		{
			str=in.readLine();
			if (str==tr("[Future Pixels]"))
			{
				for (int ii=0;ii<rgbLanduseType2.size();ii++)
				{
					str=in.readLine();
					futuretableWidget->item(1,ii)->setText(str);
				}
			}
			if (str==tr("[Restricted Matrix]"))
			{
				for (int ii=0;ii<rgbLanduseType2.size();ii++)
				{
					str=in.readLine();
					strlist=str.split(",");
					for (int jj=0;jj<rgbLanduseType2.size();jj++)
					{
						restricttableWidget->item(ii,jj)->setText(strlist[jj]);
					}
				}
			}
			if (str==tr("[Cost Matrix]"))
			{
				for (int ii=0;ii<rgbLanduseType2.size();ii++)
				{
					str=in.readLine();
					strlist=str.split(",");
					for (int jj=0;jj<rgbLanduseType2.size();jj++)
					{
						switchcost->item(ii,jj)->setText(strlist[jj]);
					}
				}
			}
			if (str==tr("[Intensity of neighborhood]"))
			{
				for (int ii=0;ii<rgbLanduseType2.size();ii++)
				{
					str=in.readLine();
					mqIntenofneigh->item(0,ii)->setText(str);
				}
			}
		}
	}
}
/// <summary>
/// <停止模型>
/// <\summary>
void DynaSimulation::stopmodel()
{
	slp->stoploop();
}
/// <summary>
/// <显示进度>
/// <\summary>
void DynaSimulation::getParameter( QString _str )
{
	ui.textSimEdit->append(_str);

	QStringList list=_str.split(",");
	if (list.size()>2&&(list[1]!=lauTypeName2[0]))
	{
		int _interval=rgbLanduseType2.size();
		/// <产生长度>
		xlength=xlength++;
		xval<<xlength;
		int all=0;
		for (int ii=0;ii<_interval;ii++)
		{
			yval<<list[ii+1].toDouble();
			all+=staCount2[ii];
		}
		/// <设置坐标轴的范围>
		for (int ii=0;ii<_interval;ii++)
		{
			for (int jj=0;jj<xlength;jj++)
			{
				extract_yval<<yval[jj*_interval+ii];
			}
			/// <设置坐标轴的名称>
			ui.qwtPlot->setAxisScale(ui.qwtPlot->xBottom, 0, ui.itelineEdit->text().toInt());
			ui.qwtPlot->setAxisScale(ui.qwtPlot->yLeft, 0,  all);
			QwtPlotCurve *curve = new QwtPlotCurve(lauTypeName2[ii]);
			curve->setPen(QPen(rgbType2[ii],2));/// <设置画笔>
			curve->setStyle(QwtPlotCurve::Lines);
			curve->setCurveAttribute(QwtPlotCurve::Fitted,true);/// <是曲线更光滑> 
			curve->attach(ui.qwtPlot);
			curve->setSamples(xval, extract_yval);
			extract_yval.clear();
		}
		ui.qwtPlot->replot();

	}
}
/// <summary>
/// <清除内存>
/// <\summary>
void DynaSimulation::closeDynaSimulation()
{
	this->lauClearall2();
	this->probClearall2();
	this->restClearall();
	if (_landuse2!=NULL)
	{
		delete[] _landuse2;
	}
	if (u_rgb!=NULL)
	{
		delete[] u_rgb;
		u_rgb=NULL;
	}
	if (u_rgbshow)
	{
		delete[] u_rgbshow;
		u_rgbshow=NULL;
	}
	
}

/// <summary>
/// <参数接收槽>
/// </summary>
void DynaSimulation::getColsandRowsandshowDynamicOnUi( int __cols,int __rows ,int _k )
{
	double _scale;


	if (_k==0)
	{

		currentheight=ui.pro_dynagraphicsView->height();

		_scale=ui.pro_dynagraphicsView->height()*1.0/__rows;

	}
	else
	{
		if (ui.pro_dynagraphicsView->height()!=currentheight)
		{
			_scale=ui.pro_dynagraphicsView->height()*1.0/currentheight;
			currentheight=ui.pro_dynagraphicsView->height();
		}
		else
		{
			_scale=1;
		}
	}

	ui.pro_dynagraphicsView->dynamicShow(u_rgb,u_rgbshow,__rows,__cols,_scale);


}

/// <summary>
/// <写入文件>
/// </summary>
void DynaSimulation::write2file()
{
	QStringList list = ui.textSimEdit->toPlainText().split("\n");
	QFile file("record.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	QTextStream out(&file);
	foreach(QString _str,list)
		out<<_str<<"\n";
	file.close();
}
