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
#include "geodpcasys.h"
#include "colorplate.h"
#include "nntrain.h"
#include "anntrainthread.h"
#include "qmath.h"

using namespace std;

GeoSimulator::GeoSimulator( GeoDpCAsys* _gdp )
{
	ui.setupUi(this);

#ifdef _DEMO_TEMP
	this->setWindowTitle("Distribution Probability Module (DEMO)");
#else
	this->setWindowTitle("Distribution Probability Module");
#endif

	
	this->setAttribute(Qt::WA_DeleteOnClose);
	m_gdp=_gdp;
	_landuse=NULL;
	p0=NULL;
	dp0=NULL;
	QIcon probicon(":/new/prefix1/概率.png");
	this->setWindowIcon(probicon);
	/// <记录编号始化>
	lauSerialNum=0;
	lauNumImage=0;
	divSerialNum=0;
	divNumImage=0;
	nodataexit=false;
	           

	ui.laubtnColor->setEnabled(false);
	ui.divbtnSubtract->setEnabled(false);
	ui.ANNtrainBtn->setEnabled(false);

	ui.divNorrabtnyes->setChecked(true);
	ui.radioSingle->setChecked(true);
	ui.raAve->setChecked(true);

	ui.PerSpinBox->setSingleStep(1);  
	ui.PerSpinBox->setRange(1,50);          
	ui.PerSpinBox->setSuffix(" %*0.1");    
	ui.PerSpinBox->resize(200,40);       
	ui.PerSpinBox->setValue(5);          

	hideLayer();       

	ui.TTspinBox->setRange(1,5);
	ui.TTspinBox->setValue(1);  

	/// <驱动力数据列表初始化>
	divMetaTable=new QStandardItemModel(this);
	divMetaTable->setColumnCount(8);
	divMetaTable->setHorizontalHeaderLabels(QStringList()<<"Image Name"<<"Data Type"<<"Rows"<<"Cols"<<"Bands"<<"No Data"<<"Max"<<"Min");
	ui.divtableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
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


bool GeoSimulator::lauLoadImage(QString* _fileName )
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	lauSerialNum=lauNumImage;

	TiffDataRead* pread = new TiffDataRead;

	lau_poDataset.append(pread);

	if (!lau_poDataset[lauSerialNum]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	lauNumImage=lau_poDataset.size();
	nodatavalue=lau_poDataset[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	return true;
}

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

void GeoSimulator::openLauFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
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
	}
}

void GeoSimulator::setLauColor()
{

	PixCal* pcl=new PixCal(this);

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
		QMessageBox::warning(this,"Error","Too many land use types!");
		return;
	}

	ColorPlate* cple=new ColorPlate(this,this,false);/// <可以重载>
	cple->show();
}

bool GeoSimulator::divLoadImage(QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	divSerialNum=divNumImage;

	TiffDataRead* pread = new TiffDataRead;

	div_poDataset.append(pread);

	if (!div_poDataset[divSerialNum]->loadInfo(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	divNumImage=div_poDataset.size();
	return true;
}

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
		hideLayer();
	}
}

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

void GeoSimulator::substractDivFileInitial()
{
	ui.divbtnSubtract->setEnabled(true);
}

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

void GeoSimulator::trainAndSaveAsTif()
{

	ui.ANNtrainBtn->setEnabled(false);

	NNtrain* ntn=new NNtrain(this);

	AnnTrainThread* ntd=new AnnTrainThread(this,ntn);

	connect(ntn,SIGNAL(sendParameter(QString)),this,SLOT(getParameter(QString)));

	ntn->moveToThread(ntd);

	ntd->start();

	if (rgbLanduseType.size()!=0&&ntd->isRunning())
	{
		QEventLoop loop;
		connect(ntd, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in training!");
		return;
	}

	this->closeGeoSimulator();

    QMessageBox::information(this,"Message","Finished training!");

	this->getParameter(tr("Finished training!"));

}

void GeoSimulator::hideLayer()
{
	int bands=0;
	for (int ii=0;ii<div_poDataset.size();ii++)
	{
		bands += div_poDataset.at(ii)->bandnum();
	}
	ui.HidSpinBox->setSingleStep(1); 
	int maxhidla=sqrt((double)(bands+rgbLanduseType.size()))+10;
	ui.HidSpinBox->setRange(1,maxhidla*2);         
	ui.HidSpinBox->setValue(maxhidla);          
}

void GeoSimulator::getParameter( QString _str )
{
	ui.probOutText->append(_str);
}

void GeoSimulator::closeGeoSimulator()
{
	this->lauClearall();
	this->divClearall();
	if (_landuse!=NULL)
	{
		delete[] _landuse;
		_landuse=NULL;
	}
	if (p0!=NULL)
	{
		delete[] p0;
		p0=NULL;
	}
	if (dp0!=NULL)
	{
		delete[] dp0;
		dp0=NULL;
	}
}



