#include "geosimulator.h"
#include "mapshowviewer.h"
#include "showproperty.h"
#include "geodpcasys.h"
#include <iostream>
#include <QtGui/QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include "PixCal.h"
#include "pixcalthread.h"
#include "TrainPrepare.h"
#include "geodpcasys.h"
#include "colorplate.h"
#include "nntrain.h"
#include "anntrainthread.h"
#include "qmath.h"

using namespace std;

GeoSimulator::GeoSimulator( QObject* parent)
{
	ui.setupUi(this);
	this->setWindowTitle("ANN-based Probability-of-occurrence Estimation");
	this->setAttribute(Qt::WA_DeleteOnClose);
	QIcon probicon(":/new/prefix1/概率.png");
	this->setWindowIcon(probicon);


	//构造文件路径
	QDir *temp = new QDir;
	bool exist = temp->exists("./FilesGenerate");
	if(exist)
	{

	}
	else
	{
		bool ok = temp->mkdir("./FilesGenerate");
	}


	/// <记录编号始化>
	lauSerialNum=0;
	lauNumImage=0;
	divSerialNum=0;
	divNumImage=0;

	ui.laubtnColor->setEnabled(false);
	ui.divbtnSubtract->setEnabled(false);
	ui.ANNtrainBtn->setEnabled(false);
	ui.divgroupBox->setEnabled(false);
	ui.trangroupBox->setEnabled(false);
	ui.SavegroupBox->setEnabled(false);

	ui.divNorrabtnyes->setChecked(true);
	ui.radioSingle->setChecked(true);
	ui.raAve->setChecked(true);

	ui.PerSpinBox->setSingleStep(1);  
	ui.PerSpinBox->setRange(1,100);          // <设置变化范围>  
	//ui.PerSpinBox->setSuffix(" * 0.001");    // <设置输出显示前缀>  
	ui.PerSpinBox->resize(200,40);        // <设置大小>  
	ui.PerSpinBox->setValue(1);          // <设置初始值>  

	hideLayer();       // <设置初始值> 

	ui.TTspinBox->setRange(1,5);
	ui.TTspinBox->setValue(1);  

	// 封印这三个功能
	ui.TTspinBox->setVisible(false);
	ui.TTlabel->setVisible(false);
	ui.radioMemory->setVisible(false);

	/// <驱动力数据列表初始化>
	divMetaTable=new QStandardItemModel(this);
	divMetaTable->setColumnCount(8);
	divMetaTable->setHorizontalHeaderLabels(QStringList()<<"Image Name"<<"Data Type"<<"Rows"<<"Cols"<<"Bands"<<"NoData Value"<<"Max"<<"Min");
	ui.divtableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);// <设置充满表宽度>
	ui.divtableView->setModel(divMetaTable);
	ui.divtableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.divtableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(ui.laubtn,SIGNAL(clicked()),this,SLOT(openLauFile()));
	connect(ui.laubtnColor,SIGNAL(clicked()),this,SLOT(setLauColor()));

	connect(ui.divbtn,SIGNAL(clicked()),this,SLOT(selectDivFiles()));
	connect(ui.divbtnAdd,SIGNAL(clicked()),this,SLOT(addOneDivFile()));
	connect(ui.divtableView,SIGNAL(clicked(const QModelIndex &)),this,SLOT(substractDivFileInitial()));
	connect(ui.divbtnSubtract,SIGNAL(clicked()),this,SLOT(substractDivFile()));

	/// <输入输出选择按钮>
	connect(ui.savePBtn,SIGNAL(clicked()),this,SLOT(saveProb_PreData()));

	/// <保存神经网络概率>
	connect(ui.ANNtrainBtn,SIGNAL(clicked()),this,SLOT(trainAndSaveAsTif()));

}

GeoSimulator::~GeoSimulator()
{
	this->closeGeoSimulator();
}

/// ************************<土地利用数据>************************** ///
/// <summary>
/// <加载驱土地利用影像文件>
/// </summary>
bool GeoSimulator::lauLoadImage(QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	lauSerialNum=lauNumImage;

	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	lau_poDataset.append(pread);

	// <提取数据>
	if (!lau_poDataset[lauSerialNum]->loadInfo(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	lauNumImage=lau_poDataset.size();// <用于记录总波段数>

	return true;
}
/// <summary>
/// <重新加载土地利用影像文件>
/// </summary>
void GeoSimulator::lauClearall()
{
	if (lau_poDataset.size()!=0)
	{
		if (lau_poDataset[0]!=NULL)
		{
			lau_poDataset[0]->close();
			delete lau_poDataset[0];
		}
	}
	lau_poDataset.clear();
	lauNumImage=0;
	lauSerialNum=0;
}
/// <summary>
/// <选择土地利用影像>
/// </summary>
void GeoSimulator::openLauFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		// 处理界面
		lauClearall();
		lauLoadImage(&fileName);
		if (lau_poDataset.at(0)->bandnum()!=1)
		{
			this->lauClearall();
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.laulineEdit->setText(fileName);
		ui.laubtnColor->setEnabled(true);

		// <新建log文件保存导入路径>
		
		QFile logfile("./FilesGenerate/logFileTrain.log");

		if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			qDebug()<<"Load File 'logFileTrain.log' Error!";
			return;
		}

		QTextStream out(&logfile);

		out << tr("[Path of land use data]").trimmed() <<"\n"<< ui.laulineEdit->text()<<"\n";

		logfile.close();
	}

}
/// <summary>
/// <选择土地利用类型颜色>
/// </summary>
void GeoSimulator::setLauColor()
{
	/// <线程>

	PixCal* pcl=new PixCal(this,"./FilesGenerate/logFileTrain.log",1);// false no sense

	PixCalThread* ptd=new PixCalThread(this,pcl);

	pcl->moveToThread(ptd);
	
	ptd->start();


	if (ptd->isRunning())
	{
	 	QEventLoop loop;
	 	connect(ptd, SIGNAL(finished()), &loop, SLOT(quit()));
	 	loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Thread Error!");
		return;
	}

	TrainPrepare* tpe=new TrainPrepare(this);

	connect(tpe,SIGNAL(sendWinClose()),this,SLOT(activatedModule()));

	tpe->show();

	ui.laubtnColor->setEnabled(false);

	ui.laubtn->setEnabled(false);
}


/// ************************<驱动力文件部分>************************** ///
/// <summary>
/// <加载驱动力影像文件>
/// </summary>
bool GeoSimulator::divLoadImage(QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	divSerialNum=divNumImage;

	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	div_poDataset.append(pread);

	// <提取数据>
	if (!div_poDataset[divSerialNum]->loadInfo(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	divNumImage=div_poDataset.size();// <用于记录总波段数>

	return true;
}
/// <summary>
/// <重新加载驱动力文件>
/// </summary>
void GeoSimulator::divClearall()
{
	divMetaTable->clear();
	divMetaTable->setHorizontalHeaderLabels(QStringList()<<"Image Name"<<"Data Type"<<"Rows"<<"Cols"<<"Bands"<<"No Data"<<"Max"<<"Min");
	for (int ii=0;ii<div_poDataset.size();ii++)
	{
		if (div_poDataset.size()!=0)
		{
			if (div_poDataset[ii]!=NULL)
			{
				div_poDataset[ii]->close();
				delete div_poDataset[ii];
			}
		}
	}
	div_poDataset.clear();
	divSerialNum=0;
	divNumImage=0;
}
/// <summary>
/// <选择驱动力文件>
/// </summary>
void GeoSimulator::selectDivFiles()
{
	QStringList _fileNames = QFileDialog::getOpenFileNames(
		this,
		tr( "Pick one or more image files to open..." ),
		QDir::currentPath(),                                              // 
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)") 
		);


	if (!_fileNames.isEmpty())
	{

		divClearall();

		QStringList::Iterator it = _fileNames.begin();

		QString itQstr;

		while(it != _fileNames.end())
		{
			itQstr=it[0];

			divLoadImage(&itQstr);

			++it;
		}
		getBasicDivInfo();
		hideLayer();
	}

}
/// <summary>
/// <选择驱动力文件>
/// </summary>
void GeoSimulator::getBasicDivInfo()
{
	double* minmax1=new double[2];
	double max1;
	double min1;
	for (int ii=0;ii<div_poDataset.count();ii++)
	{
		QFileInfo fileInfo(div_poDataset[ii]->getFileName());
		divMetaTable->setItem(ii,0,new QStandardItem(fileInfo.fileName()));
		divMetaTable->setItem(ii,1,new QStandardItem(GDALGetDataTypeName(div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetRasterDataType())));
		divMetaTable->setItem(ii,2,new QStandardItem(QString::number( div_poDataset[ii]->poDataset()->GetRasterYSize())));
		divMetaTable->setItem(ii,3,new QStandardItem(QString::number( div_poDataset[ii]->poDataset()->GetRasterXSize())));
		divMetaTable->setItem(ii,4,new QStandardItem(QString::number( div_poDataset[ii]->poDataset()->GetRasterCount())));
		divMetaTable->setItem(ii,5,new QStandardItem(QString::number( div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue())));
		if (div_poDataset[ii]->poDataset()->GetRasterCount()==1)
		{
			div_poDataset[ii]->poDataset()->GetRasterBand(1)->ComputeRasterMinMax(1,minmax1);
			min1=minmax1[0];

			// 防止GDAL的自带函数出错
			float nodatavalue_factor=div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
			double nodatavalue_factord=div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
			if (min1==nodatavalue_factor||min1==nodatavalue_factord)
			{
				min1=0;
			}
			// 防止GDAL的自带函数出错

			max1=minmax1[1];
			divMetaTable->setItem(ii,6,new QStandardItem(QString::number(max1)));
			divMetaTable->setItem(ii,7,new QStandardItem(QString::number(min1)));
		}
		else
		{
			divMetaTable->setItem(ii,6,new QStandardItem(tr("-")));
			divMetaTable->setItem(ii,7,new QStandardItem(tr("-")));
		}
		

		divMetaTable->item(ii, 0)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 1)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 2)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 3)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 4)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 5)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 6)->setTextAlignment(Qt::AlignCenter);
		divMetaTable->item(ii, 7)->setTextAlignment(Qt::AlignCenter);
	}
	delete[] minmax1;
}
/// <summary>
/// <添加驱动力文件>
/// </summary>
void GeoSimulator::addOneDivFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick a image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		this->divLoadImage(&fileName);

		getBasicDivInfo();
		/// <也变化>
		hideLayer();
	}
}
/// <summary>
/// <减去驱动力文件>
/// </summary>
void GeoSimulator::substractDivFile()
{
	QModelIndex divindex=ui.divtableView->currentIndex();
	int divSerialNum=divindex.row();

	divMetaTable->removeRow(divSerialNum);
	div_poDataset[divSerialNum]->close();
	div_poDataset.removeAt(divSerialNum);
	divNumImage=divMetaTable->rowCount();
	divSerialNum=divNumImage;
	ui.divbtnSubtract->setEnabled(false);
	getBasicDivInfo();

	hideLayer();

}
/// <summary>
/// <减去驱动力文件按钮初始化>
/// </summary>
void GeoSimulator::substractDivFileInitial()
{
	ui.divbtnSubtract->setEnabled(true);
}
/// <summary>
/// <保存数据路径>
/// </summary>
void GeoSimulator::saveProb_PreData()
{
	QString _savefilename = QFileDialog::getSaveFileName(
		this,
		tr( "Input a image name to save..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif)" ) );
	if (!_savefilename.isNull())
	{
		ui.savePlineEdit->setText(_savefilename);
		ui.ANNtrainBtn->setEnabled(true);
	}
}
/// <summary>
/// <保存数据>
/// </summary>
void GeoSimulator::trainAndSaveAsTif()
{

	for(int i=0;i<div_poDataset.size();i++)
	{
		if ((div_poDataset[i]->rows()!=lau_poDataset[0]->rows())||(div_poDataset[i]->cols()!=lau_poDataset[0]->cols()))
		{
			QMessageBox::warning(this,"Warning ",tr\
			("The row and column number(row=%1,col=%2) of a driving factor(%3) is inconsistent with the land use data(row=%4,col=%5), please ensure that the row and column numbers are the same or the software will not run.")\
			.arg(div_poDataset[i]->rows()).arg(div_poDataset[i]->cols()).arg(div_poDataset[i]->getFileName()).arg(lau_poDataset[0]->rows()).arg(lau_poDataset[0]->cols()));
			return;
		}
	}

	writeNetworkParameter();

	ui.ANNtrainBtn->setEnabled(false);
	/// <线程>
	NNtrain* ntn=new NNtrain(this);

	AnnTrainThread* ntd=new AnnTrainThread(this,ntn);// <抛线程>

	connect(ntn,SIGNAL(sendParameter(QString)),this,SLOT(getParameter(QString)));

	ntn->moveToThread(ntd);// <这句似乎不加也可以>

	ntd->start();

	if (ntd->isRunning())
	{
		QEventLoop loop;
		connect(ntd, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something Wrong in Training!");
		return;
	}

	this->getParameter(tr("The training is finished!"));

	QMessageBox::information(this,"Message","The training is finished!");

	this->write2file();

	this->closeGeoSimulator();
}
/// <summary>
/// <隐藏层数计算>
/// </summary>
void GeoSimulator::hideLayer()
{
	int bands=0;
	for (int ii=0;ii<div_poDataset.size();ii++)
	{
		bands += div_poDataset.at(ii)->bandnum();
	}
	ui.HidSpinBox->setSingleStep(1); 
	int maxhidla=sqrt((double)(bands))+10;
	ui.HidSpinBox->setRange(1,maxhidla*4);       // <设置变化范围>   
	ui.HidSpinBox->setValue(maxhidla);          // <设置初始值> 
}
/// <summary>
/// <接收显示参数>
/// </summary>
void GeoSimulator::getParameter( QString _str )
{
	ui.probOutText->append(_str);
}

/// <summary>
/// <写入文件>
/// </summary> 
void GeoSimulator::write2file()
{
	QStringList list = ui.probOutText->toPlainText().split("\n");

	QFile file("./FilesGenerate/ANNoutput.log");
	if (!file.open(QIODevice::ReadWrite))
		return;
	QTextStream out(&file);
	foreach(QString _str,list)
		out<<_str<<"\n";
	file.close();
}

/// <summary>
/// <清空>
/// </summary>
void GeoSimulator::closeGeoSimulator()
{
	this->lauClearall();
	this->divClearall();
}
/// <summary>
/// <完成操作激活组件>
/// </summary>
void GeoSimulator::activatedModule()
{
	ui.divgroupBox->setEnabled(true);
	ui.trangroupBox->setEnabled(true);
	ui.SavegroupBox->setEnabled(true);
}
/// <summary>
/// <写入路径，参数>
/// </summary>
void GeoSimulator::writeNetworkParameter()
{

	QFile logfile("./FilesGenerate/logFileTrain.log");

	if (!logfile.open(QIODevice::Append| QIODevice::ReadWrite))
	{
		qDebug()<<"Load File 'logFileTrain.log' Error!";
		return;
	}

	QTextStream in(&logfile);

	in << tr("[Path of saving data]").trimmed() <<"\n";

	in<<ui.savePlineEdit->text()<<"\n";

	in << tr("[Number of driving data]").trimmed() <<"\n";

	in<< QString::number(div_poDataset.size())<<"\n";

	in << tr("[Path of driving data]").trimmed() <<"\n";

	for (int ii=0;ii<div_poDataset.size();ii++)
	{
		QString str=QString::fromStdString(div_poDataset[ii]->getFileName());
		in<<str.replace("\\","\/",Qt::CaseInsensitive)<<"\n";
	}

	in << tr("[Data type]").trimmed() <<"\n";

	if (ui.radioSingle->isChecked()==true)
	{
		in << tr("Float").trimmed() <<"\n";
	}
	else if(ui.radioDouble->isChecked()==true)
	{
		in << tr("Double").trimmed() <<"\n";
	}
	else
	{
		in << tr("Unsigned short").trimmed() <<"\n";
	}

	in << tr("[Normalization type]").trimmed() <<"\n";

	if (ui.divNorrabtnyes->isChecked()==true)
	{
		in << tr("Normalization").trimmed() <<"\n";
	}
	else
	{
		in << tr("Not normalized").trimmed() <<"\n";
	}

	in << tr("[Sample type]").trimmed() <<"\n";

	if (ui.raAve->isChecked()==true)
	{
		in << tr("Uniform Sampling").trimmed() <<"\n";
	}
	else
	{
		in << tr("Sampling in Proportion").trimmed() <<"\n";
	}


	in << tr("[Percentage of Random Points]").trimmed() <<"\n";

	in << QString::number(ui.PerSpinBox->value())<<"\n";

	in << tr("[Hidden layer]").trimmed() <<"\n";

	in << QString::number(ui.HidSpinBox->value())<<"\n";


	logfile.close();

	for (int ii=0;ii<div_poDataset.size();ii++)
	{
		div_poDataset[ii]->close();
	}

	lau_poDataset[0]->close();

}

