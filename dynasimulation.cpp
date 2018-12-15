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
#include <QDebug>

using namespace std;

DynaSimulation::DynaSimulation(QObject* parent)
{
	ui.setupUi(this);

	this->setWindowTitle("Self Adaptive Inertia and Competition Mechanism CA");
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

	QDir *temp = new QDir;
	bool exist = temp->exists("./FilesGenerate");
	if(exist)
	{

	}
	else
	{
		bool ok = temp->mkdir("./FilesGenerate");
	}

	xlength=0;

	isMultiYears=false;

	ui.laubtnColor2->setEnabled(false);
	ui.finishGeoButton->setEnabled(false);
	ui.laulineEdit2->setReadOnly(true);
	ui.inProbabilitylineEdit->setReadOnly(true);
	ui.saveSimlineEdit->setReadOnly(true);
	ui.norstradioButton->setChecked(true);
	ui.rstlabel->setEnabled(false);
	ui.rstlineEdit->setReadOnly(true);
	ui.rstlineEdit->setEnabled(false);
	ui.rstbtn->setEnabled(false);
	ui.probabilitygroupBox->setEnabled(false);
	ui.resultgroupBox->setEnabled(false);
	ui.restrictgroupBox->setEnabled(false);
	ui.simgroupbox->setEnabled(false);
	ui.finishGeoButton->setEnabled(false);
	ui.itelineEdit->setText(tr("300"));
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
	ui.tabWidget->addTab(futuretableWidget,tr("Land Use Demand"));

	switchcost=new QTableWidget(this);
	ui.tabWidget->addTab(switchcost,tr("Cost Matrix"));

	mqIntenofneigh=new QTableWidget(this);
	ui.tabWidget->addTab(mqIntenofneigh,tr("Weight of Neighborhood"));


	connect(ui.laubtn2,SIGNAL(clicked()),this,SLOT(openLauFile2()));
	connect(ui.laubtnColor2,SIGNAL(clicked()),this,SLOT(setLauColor2()));
	connect(ui.inProbabilityBtn,SIGNAL(clicked()),this,SLOT(openProbFile2()));
	connect(ui.saveSimBtn,SIGNAL(clicked()),this,SLOT(saveSimScenario()));
	connect(ui.rstbtn,SIGNAL(clicked()),this,SLOT(openRestFile()));
	connect(ui.norstradioButton,SIGNAL(toggled(bool)),this,SLOT(modeSelect()));
	connect(ui.finishGeoButton,SIGNAL(clicked()),this,SLOT(inisimmulate()));
	connect(ui.btnRun,SIGNAL(clicked()),this,SLOT(startsimulate()));
	connect(ui.btnStop,SIGNAL(clicked()),this,SLOT(stopmodel()));
	connect(ui.btnFit, SIGNAL(clicked()), ui.pro_dynagraphicsView,SLOT(ZoomFit()));

}

DynaSimulation::~DynaSimulation()
{

}

void DynaSimulation::readDemandFile(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug()<<"Read File Error!";
		return;
	}

	int num=0;

	QList<int> countDemand;

	file.readLine();

	while (!file.atEnd()) 
	{
		QString line = file.readLine();
		QStringList strlist=line.split(",");
		if (strlist.length()!=(countAvaliable.length()+1))
		{
			qDebug()<<"Number of Land Use Type is not Match";
			return;
		}
		else
		{
			for (int ii=0;ii<strlist.length();ii++)
			{
				countDemand.push_back(strlist[ii].toInt());
			}
			num++;
		}
	}


	futuretableWidget->setRowCount(1+num); 
	futuretableWidget->setColumnCount(countAvaliable.size()); 
	QStringList h_header;
	QStringList v_header;
	for (int i=0;i<countAvaliable.size();i++)
	{
		h_header<<NameAvaliable.at(i);

		QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(countAvaliable[i]));
		_tmp0->setTextAlignment(Qt::AlignCenter);
		_tmp0->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); 
		futuretableWidget->setItem(0,i,_tmp0); 

	}

	v_header<<tr("Initial Pixel Number");
	for(int ii=0;ii<num;ii++)
	{
		QString str=QString::number(countDemand[ii*(countAvaliable.size()+1)]);

		v_header.push_back(str);

		for (int jj=0;jj<countAvaliable.size();jj++)
		{
			QTableWidgetItem* _tmpX=new QTableWidgetItem(QString::number(countDemand[ii*(countAvaliable.size()+1)+jj+1]));
			_tmpX->setTextAlignment(Qt::AlignCenter);
			_tmpX->setFlags(_tmpX->flags() & (~Qt::ItemIsEditable)); 
			futuretableWidget->setItem(1+ii,jj,_tmpX);
		}
	}
	futuretableWidget->setHorizontalHeaderLabels(h_header);
	futuretableWidget->setVerticalHeaderLabels(v_header);

	file.copy(filename, "./FilesGenerate/logLandDemand.tmp");

	file.close();

}

void DynaSimulation::checkSettings()
{
	if ((ui.laulineEdit2->text().trimmed()!="")&&(ui.inProbabilitylineEdit->text().trimmed()!="")&&(ui.saveSimlineEdit->text().trimmed()!="")&&(ui.laubtnColor2->isEnabled()==false))
	{
		ui.finishGeoButton->setEnabled(true);
	}
	else
	{
		ui.finishGeoButton->setEnabled(false);
	}
}

void DynaSimulation::openLauFile2()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		ui.laulineEdit2->setText(fileName);
		ui.laubtnColor2->setEnabled(true);
	}
	else
	{
		qDebug()<<"Read Image Error!";
	}

	QFile logfile("./FilesGenerate/logFileSimulation.log");
	if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug()<<"Load File 'logFileSimulation.log' Error!";
		return;
	}

	QTextStream out(&logfile);
	out << "[Path of land use data]\n" << ui.laulineEdit2->text() << "\n";

	logfile.close();

	checkSettings();

}

void DynaSimulation::setLauColor2()
{
	/// <线程>

	PixCal* dynapcl=new PixCal(this,"./FilesGenerate/logFileSimulation.log",1);// false no sense

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
		QMessageBox::warning(this,"Error","Thread Error");
		return;
	}

	ColorPlate* dynacple=new ColorPlate(this);/// <可以重载>

	dynacple->show();

	connect(dynacple,SIGNAL(sendWinClose()),this,SLOT(input_Future_Land_Area()));

	ui.laubtnColor2->setEnabled(false);

	checkSettings();

}

void DynaSimulation::openRestFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		ui.rstlineEdit->setText(fileName);
	}
	else
	{
		qDebug()<<"Read Image Error!";
	}

	checkSettings();
}

void DynaSimulation::openProbFile2()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		ui.inProbabilitylineEdit->setText(fileName);
	}
	else
	{
		qDebug()<<"Read Image Error!";
	}
}

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
	else
	{
		qDebug()<<"Write Image Path Error!";
	}

	checkSettings();
}


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
		ui.rstlineEdit->setEnabled(true);
		ui.rstbtn->setEnabled(true);
	}
}

void DynaSimulation::input_Future_Land_Area()
{

	ui.probabilitygroupBox->setEnabled(true);
	ui.resultgroupBox->setEnabled(true);
	ui.restrictgroupBox->setEnabled(true);
	ui.simgroupbox->setEnabled(true);
	ui.finishGeoButton->setEnabled(true);

	QFile file("./FilesGenerate/config_color.log");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug()<<"Load 'config_color.log' Error!";
		return;
	}
	file.readLine();
	while (!file.atEnd()) 
	{
		QString str = file.readLine();
		QStringList strList = str.split(",", QString::SkipEmptyParts);
		countAvaliable.push_back(strList[1].toInt());
		NameAvaliable.push_back(strList[2]);

		QColor qc(strList[3].toInt(),strList[4].toInt(),strList[5].toInt());

		rgbAvaliable.append(qc);
	}
	file.close();

	futuretableWidget->setRowCount(2); 
	futuretableWidget->setColumnCount(countAvaliable.size()); 
	QStringList h_header;
	QStringList v_header;
	for (int i=0;i<countAvaliable.size();i++)
	{
		h_header<<NameAvaliable.at(i);

		QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(countAvaliable[i]));
		_tmp0->setTextAlignment(Qt::AlignCenter);
		_tmp0->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); 
		futuretableWidget->setItem(0,i,_tmp0); 
			
		QTableWidgetItem* _tmp1=new QTableWidgetItem();
		_tmp1->setTextAlignment(Qt::AlignCenter);
		futuretableWidget->setItem(1,i,_tmp1); 
	}
	v_header<<tr("Initial Pixel Number")<<tr("Future Pixel Number");
	futuretableWidget->setHorizontalHeaderLabels(h_header);
	futuretableWidget->setVerticalHeaderLabels(v_header);

	switchcost->setRowCount(countAvaliable.size()); 
	switchcost->setColumnCount(countAvaliable.size()); 
	for (int i=0;i<countAvaliable.size();i++)
	{
		for (int j=0;j<countAvaliable.size();j++)
		{
			QTableWidgetItem* _tmp2=new QTableWidgetItem(QString::number(1));
			_tmp2->setTextAlignment(Qt::AlignCenter);
			if (i==j)
			{
				_tmp2->setFlags(_tmp2->flags() & (~Qt::ItemIsEditable)); 
			}
			switchcost->setItem(i,j,_tmp2);
		}
	}
	switchcost->setHorizontalHeaderLabels(h_header);
	switchcost->setVerticalHeaderLabels(h_header);
	//switchcost->verticalHeader()->setResizeMode(QHeaderView::Stretch);

	mqIntenofneigh->setRowCount(1); 
	mqIntenofneigh->setColumnCount(countAvaliable.size()); 
	QStringList h_header_1;
	QStringList v_header_1;
	for (int i=0;i<countAvaliable.size();i++)
	{
		h_header_1<<NameAvaliable.at(i);

		QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(1));
		_tmp0->setTextAlignment(Qt::AlignCenter);
		mqIntenofneigh->setItem(0,i,_tmp0); 
	}
	v_header_1<<tr("Weight of neighborhood");
	mqIntenofneigh->setHorizontalHeaderLabels(h_header_1);
	mqIntenofneigh->setVerticalHeaderLabels(v_header_1);
//	}

	this->loadconfig();

	this->ui.laubtnColor2->setEnabled(false);
}

void DynaSimulation::inisimmulate()
{
	//==========================================--------------------------------------------------
	readImageData();
	for(int i=0;i<imglist.size();i++)
	{
		if ((imglist[i]->rows()!=imglist[0]->rows())||(imglist[i]->cols()!=imglist[0]->cols()))
		{
			QMessageBox::warning(this,"Warning ",tr\
				("The row and column number(row=%1,col=%2) of a input image(%3) is inconsistent with the land use data(row=%4,col=%5), please ensure that the row and column numbers are the same or the software will not run.")\
				.arg(imglist[i]->rows()).arg(imglist[i]->cols()).arg(imglist[i]->getFileName()).arg(imglist[0]->rows()).arg(imglist[0]->cols()));
			return;
		}
	}
	imglist[0]->close();
	imglist[1]->close();
	if (imglist.size()==3)
	{
		imglist[2]->close();
	}


	QFile logfile("./FilesGenerate/logFileSimulation.log");
	if (!logfile.open( QIODevice::Append | QIODevice::ReadWrite ))
	{
		qDebug()<<"Load 'logFileSimulation.log' Error!";
		return;
	}

	QTextStream out(&logfile);

	out << "[Path of probability data]\n" << ui.inProbabilitylineEdit->text() << "\n";
	out << "[Path of simulation result]\n" << ui.saveSimlineEdit->text() << "\n";
	if (ui.rstradioButton->isChecked()==true)
	{
		out << "[Path of restricted area]\n" << ui.rstlineEdit->text() << "\n";
	}
	else
	{
		out << "[Path of restricted area]\n" << tr("No restrict data") << "\n";
	}
	logfile.close();


	QVBoxLayout *layout = new QVBoxLayout; 
	for (int ii=0;ii<countAvaliable.size();ii++)
	{
		QLabel* _label=new QLabel(NameAvaliable[ii]);
		_label->setFrameStyle(QFrame::Panel | QFrame::Raised);  

		_label->setAutoFillBackground(true);
		QPalette palette;
		palette.setColor(QPalette::Background,rgbAvaliable[ii]);
		_label->setPalette(palette);
		_label->setLineWidth(2); 
		layout->addWidget(_label); 
	}
	ui.frame_lengend->setLayout(layout);

	ui.btnRun->setEnabled(true);
	ui.btnStop->setEnabled(true);

	ui.finishGeoButton->setEnabled(false);
}

void DynaSimulation::startsimulate()
{
	if (isMultiYears==false)
	{
		remove("./FilesGenerate/logLandDemand.tmp");
	}

	ui.btnRun->setEnabled(false);

	this->appendconfig();

	slp=new SimulationProcess(this);

	SimuThread* std=new SimuThread(this,slp);

	slp->moveToThread(std);// <这句似乎不加也可以>

	std->start();

	connect(this,SIGNAL(sendfinishedcode(int)),slp,SLOT(acceptFinisheCode(int)), Qt::DirectConnection);

	if (countAvaliable.size()!=0&&std->isRunning())
	{
		QEventLoop loop;
		connect(std, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something Wrong in Simulation Thread!");
		return;
	}

	QMessageBox::information(this,"Message","The simulation is finished!");


	this->getParameter(tr("The simulation is finished!"));

	this->write2file();

	this->ui.btnRun->setEnabled(false);

}

void DynaSimulation::appendconfig()
{
	QFile file("./FilesGenerate/config_mp.log");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug()<<"Load 'config_mp.log' Error!";
		return;
	}

	if(file.open(QFile::WriteOnly|QIODevice::Text));
	{
		QTextStream out(&file);

		out << "[Number of types]" << "\n";

		out<<QString::number(countAvaliable.size())<<"\n";

		out << "[Future Pixels]" << "\n";

		for (int ii=0;ii<countAvaliable.size();ii++)
		{
			if (isMultiYears==true)
			{
				out<<0<<"\n";
			}
			else
			{
				out<<futuretableWidget->item(1,ii)->text().trimmed()<<"\n";
			}
		}

		out<<tr("[Cost Matrix]")<<"\n";

		for (int ii=0;ii<countAvaliable.size();ii++)
		{
			QString str;
			for (int jj=0;jj<countAvaliable.size();jj++)
			{
				str+=switchcost->item(ii,jj)->text().trimmed()+",";
			}
			out<<str.left(str.length() - 1)<<"\n";
		}

		out << "[Intensity of neighborhood]" << "\n";

		for (int ii=0;ii<countAvaliable.size();ii++)
		{
			out<<mqIntenofneigh->item(0,ii)->text().trimmed()<<"\n";
		}

		out << "[Maximum Number Of Iterations]" << "\n";

		out<<ui.itelineEdit->text().trimmed()<<"\n";

		out << "[Size of neighborhood]" << "\n";

		out<<ui.CAneigh->text().trimmed()<<"\n";

		out << "[Accelerated factor]" << "\n";

		out<<ui.deglineEdit->text().trimmed()<<"\n";

	}
	file.close();
}


void DynaSimulation::loadconfig()
{
	QFile file("./FilesGenerate/config_mp.log");
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
				for (int ii=0;ii<countAvaliable.size();ii++)
				{
					str=in.readLine();
					futuretableWidget->item(1,ii)->setText(str);
					oneyearFuture.push_back(str.toInt());
				}
			}
			if (str==tr("[Cost Matrix]"))
			{
				for (int ii=0;ii<countAvaliable.size();ii++)
				{
					str=in.readLine();
					strlist=str.split(",");
					for (int jj=0;jj<countAvaliable.size();jj++)
					{
						switchcost->item(ii,jj)->setText(strlist[jj]);
					}
				}
			}
			if (str==tr("[Intensity of neighborhood]"))
			{
				for (int ii=0;ii<countAvaliable.size();ii++)
				{
					str=in.readLine();
					mqIntenofneigh->item(0,ii)->setText(str);
				}
			}

			if (str==tr("[Maximum Number Of Iterations]" ))
			{
				str=in.readLine();
				ui.itelineEdit->setText(str);
			}
			if (str==tr("[Size of neighborhood]" ))
			{
				str=in.readLine();
				ui.CAneigh->setValue(str.toInt());
			}
			if (str==tr("[Accelerated factor]" ))
			{
				str=in.readLine();
				ui.deglineEdit->setText(str);
			}
			if (str==tr("[Number of types]"))
			{
				
			}
		}
	}
}

void DynaSimulation::stopmodel()
{
	slp->stoploop();
}

void DynaSimulation::getParameter( QString _str )
{
	ui.textSimEdit->append(_str);

	QStringList list=_str.split(",");
	if (list.size()>2&&(list[1]!=NameAvaliable[0]))
	{
		int _interval=countAvaliable.size();
		/// <产生长度>
		xlength=xlength++;
		xval<<xlength;
		int all=0;
		for (int ii=0;ii<_interval;ii++)
		{
			yval<<list[ii+1].toDouble();
			all+=countAvaliable[ii];
		}
		for (int ii=0;ii<_interval;ii++)
		{
			for (int jj=0;jj<xlength;jj++)
			{
				extract_yval<<yval[jj*_interval+ii];
			}
			ui.qwtPlot->setAxisScale(ui.qwtPlot->xBottom, 0, ui.itelineEdit->text().toInt());
			ui.qwtPlot->setAxisScale(ui.qwtPlot->yLeft, 0,  all);
			QwtPlotCurve *curve = new QwtPlotCurve(NameAvaliable[ii]);
			curve->setPen(QPen(rgbAvaliable[ii],2));
			curve->setStyle(QwtPlotCurve::Lines);
			curve->setCurveAttribute(QwtPlotCurve::Fitted,true);
			curve->attach(ui.qwtPlot);
			curve->setSamples(xval, extract_yval);
			extract_yval.clear();
		}
		ui.qwtPlot->replot();

	}
}

void DynaSimulation::getColsandRowsandshowDynamicOnUi( int __cols,int __rows ,int _k )
{
	double _scale;

	if (_k==0)
	{
		currentheight=ui.pro_dynagraphicsView->height();

		_scale=ui.pro_dynagraphicsView->height()*1.0/__rows;

		ui.pro_dynagraphicsView->dynamicShow(slp->uRGB(),slp->uRGBshow(),__rows,__cols,_scale);
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

		countfinished=ui.pro_dynagraphicsView->isfinished();

		ui.pro_dynagraphicsView->dynamicShow(slp->uRGB(),slp->uRGBshow(),__rows,__cols,_scale);

		sendfinishedcode(countfinished); 
	}
	

}

void DynaSimulation::write2file()
{
	QStringList list = ui.textSimEdit->toPlainText().split("\n");
	QFile file("./FilesGenerate/output.log");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}
	QTextStream out(&file);
	foreach(QString _str,list)
		out<<_str<<"\n";
	file.close();
}

void DynaSimulation::restoreOneYears()
{
	futuretableWidget->setRowCount(2);
	futuretableWidget->setColumnCount(countAvaliable.size()); 
	QStringList h_header;
	QStringList v_header;
	for (int i=0;i<countAvaliable.size();i++)
	{
		h_header<<NameAvaliable.at(i);

		QTableWidgetItem* _tmp0=new QTableWidgetItem(QString::number(countAvaliable[i]));
		_tmp0->setTextAlignment(Qt::AlignCenter);
		_tmp0->setFlags(_tmp0->flags() & (~Qt::ItemIsEditable)); 
		futuretableWidget->setItem(0,i,_tmp0); 

		QTableWidgetItem* _tmp1=new QTableWidgetItem();
		_tmp1->setTextAlignment(Qt::AlignCenter);
		futuretableWidget->setItem(1,i,_tmp1); 
	}
	v_header<<tr("Initial Pixel Number")<<tr("Future Pixel Number");
	futuretableWidget->setHorizontalHeaderLabels(h_header);
	futuretableWidget->setVerticalHeaderLabels(v_header);

	loadconfig();

	remove("./FilesGenerate/logLandDemand.tmp");

}



void DynaSimulation::readImageData()
{
	if (imglist.size()!=0)
	{
		for (int i=0;i<imglist.size();i++)
		{
			imglist[i]->close();
			
		}
	}

	imglist.clear();

	TiffDataRead* pread = new TiffDataRead;
	imglist.append(pread);
	string laup=ui.laulineEdit2->text().trimmed().toStdString();
	imglist[0]->loadInfo(laup.c_str());

	TiffDataRead* pread1 = new TiffDataRead;
	imglist.append(pread1);
	string prop=ui.inProbabilitylineEdit->text().trimmed().toStdString();
	imglist[1]->loadInfo(prop.c_str());
	
	if (ui.rstradioButton->isChecked()==true)
	{
		TiffDataRead* pread2 = new TiffDataRead;
		imglist.append(pread2);
		string rstp=ui.rstlineEdit->text().trimmed().toStdString();
		imglist[2]->loadInfo(rstp.c_str());
	}
}