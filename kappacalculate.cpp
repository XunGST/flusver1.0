#include "kappacalculate.h"
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include "kappacalculator.h"
#include "kappathread.h"
#include "fomcalculator.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>


KappaCalculate::KappaCalculate(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowTitle(tr("Precision Validation"));
	this->setAttribute(Qt::WA_DeleteOnClose);
	QIcon probicon(":/new/prefix1/卡帕系数.png");
	this->setWindowIcon(probicon);

	ui.spinBox_samplingRate->setValue(1);

	ui.lineEdit_samplingAmout->setText("100");

	ui.btnStart_FoM->setEnabled(false);

	ui.rbtnFoM->setChecked(true);

	ui.rbtnFoM->setVisible(false);
	ui.rbtnKappa->setVisible(false);

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

	// FoM

	connect(ui.InputTurthBtn,SIGNAL(clicked()),this,SLOT(openTurthFile())); // Kappa

	connect(ui.InputCompBtn,SIGNAL(clicked()),this,SLOT(openCompdFile())); // Kappa

	connect(ui.startBtn,SIGNAL(clicked()),this,SLOT(startcalculate())); // Kappa

	connect(ui.radioButton_samplingAmount,SIGNAL(toggled(bool)),this,SLOT(switchEnable())); // Kappa

	ui.radioButton_samplingRate->setChecked(true); // Kappa

	ui.lineEdit_samplingAmout->setEnabled(false); // Kappa

	//------------------------------------------------------------------------------------------------------
	connect(ui.InputBaseyearBtn_FoM,SIGNAL(clicked()),this,SLOT(readfileforFoM())); // FoM
	connect(ui.InputTurthBtn_FoM,SIGNAL(clicked()),this,SLOT(readfileforFoM())); // FoM
	connect(ui.InputCompBtn_FoM,SIGNAL(clicked()),this,SLOT(readfileforFoM())); // FoM

	connect(ui.btnStart_FoM,SIGNAL(clicked()),this,SLOT(startcalculateFoM())); // Kappa

	//ui.tabWidget->removeTab(1);// 封印Fom

}

KappaCalculate::~KappaCalculate()
{
	clearFoMfile();
	clearAllTurth();
	clearAllCompd();
}



///
/// <土地利用数据>
///
bool KappaCalculate::turthLoadImage( QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	vTurth_poDataset.append(pread);

	// <提取数据>
	if (!vTurth_poDataset[0]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	dNodataValue_Turth=vTurth_poDataset[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();

	return true;
}
/// <summary>
/// <打开>
/// </summary>
void KappaCalculate::openTurthFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		clearAllTurth();
		turthLoadImage(&fileName);
		if (vTurth_poDataset.at(0)->bandnum()!=1)
		{
			this->clearAllTurth(); 
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.input_path_truth->setText(fileName);
	}
}
/// <summary>
/// <清除>
/// </summary>
void KappaCalculate::clearAllTurth()
{
	if (vTurth_poDataset.size()!=0)
	{
		if (vTurth_poDataset[0]!=NULL)
		{
			vTurth_poDataset[0]->close();
		}
	}
	vTurth_poDataset.clear();
}



///
/// <对比土地利用数据>
///
bool KappaCalculate::compdLoadImage( QString* _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	vCompd_poDataset.append(pread);

	// <提取数据>
	if (!vCompd_poDataset[0]->loadFrom(_fileName->toStdString().c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	dNodataValue_Compd=vCompd_poDataset[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();

	return true;
}
/// <summary>
/// <打开>
/// </summary>
void KappaCalculate::openCompdFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		clearAllCompd();
		compdLoadImage(&fileName);
		if (vTurth_poDataset.at(0)->bandnum()!=1)
		{
			this->clearAllTurth(); 
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.input_path_compared->setText(fileName);
	}
}

/// <summary>
/// <清除>
/// </summary>
void KappaCalculate::clearAllCompd()
{
	if (vCompd_poDataset.size()!=0)
	{
		if (vCompd_poDataset[0]!=NULL)
		{
			vCompd_poDataset[0]->close();
		}
	}
	vCompd_poDataset.clear();
}

void KappaCalculate::read2textedit()
{
	QFile file("./FilesGenerate/Kappa.csv");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug()<<"New Kappa.csv Error!";
		return;
	}
	ui.textAccuracy->clear();
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		ui.textAccuracy->append(line);
	}
	file.close();
}


void KappaCalculate::startcalculate()
{
	mdsamplingrate=ui.spinBox_samplingRate->value()*0.01;
	misamplingamount=ui.lineEdit_samplingAmout->text().toInt();
	if (ui.radioButton_samplingRate->isChecked()==true)
	{
		mdsamplingrate=ui.spinBox_samplingRate->value()*0.01;
		misamplingamount=-1;
	}
	else
	{
		mdsamplingrate=-1;
		misamplingamount=ui.lineEdit_samplingAmout->text().toInt();
	}

	kcr=new KappaCalculator( vTurth_poDataset,vCompd_poDataset,mdsamplingrate,misamplingamount);
	KappaThread* ktrd=new KappaThread(kcr);
	kcr->moveToThread(ktrd);// <这句似乎不加也可以>
	ktrd->start();

	if (ktrd->isRunning())
	{
		QEventLoop loop;
		connect(ktrd, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in kappa thread!");
		return;
	}

	QMessageBox::information(this,"Message","Complete the calculation!");

	read2textedit();

// 	clearAllTurth();
// 	clearAllCompd();

}

void KappaCalculate::switchEnable()
{
	if (ui.radioButton_samplingRate->isChecked()==true)
	{
		ui.spinBox_samplingRate->setEnabled(true);
		ui.lineEdit_samplingAmout->setEnabled(false);
	}
	else
	{
		ui.spinBox_samplingRate->setEnabled(false);
		ui.lineEdit_samplingAmout->setEnabled(true);
	}
}

/// <summary>
/// <FoM文件[0]为startmap,[1]为truth map,[2]为sim result>
/// </summary>
bool ascendingSort(const fomReader &a,const fomReader &b) 
{ 
	return a.serialnumber<b.serialnumber; 
} 

void KappaCalculate::openfileforFoM(QString* _fileName,int snum)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	// <新建IO类>
	fomReader preader; 
	preader._pread= new TiffDataRead;
	preader.serialnumber=snum;

	string str=_fileName->toStdString();

	preader._path=str;


	// <存入>
	sFoM_Reader.push_back(preader);

	int numofdata=sFoM_Reader.size();

	// <提取数据>
	if (!sFoM_Reader[numofdata-1]._pread->loadInfo(preader._path.c_str()))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	if (numofdata==3)
	{
		ui.btnStart_FoM->setEnabled(true);

		std::sort(sFoM_Reader.begin(),sFoM_Reader.end(),ascendingSort);

		dNodataValue_Start=sFoM_Reader[0]._pread->poDataset()->GetRasterBand(1)->GetNoDataValue();

	}
	
}

void KappaCalculate::readfileforFoM()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		int serialnum=0;

		QObject* lSender=sender();
		if(lSender==0)
			return ;
		if(lSender->objectName()=="InputBaseyearBtn_FoM")
		{
			ui.input_path_baseyear->setText(fileName);
			serialnum=1;
			if (sFoM_Reader.size()==3)
			{
				sFoM_Reader[serialnum-1]._pread->close();
				vector<fomReader>::iterator it = sFoM_Reader.begin()+serialnum-1;
				sFoM_Reader.erase(it);
			}
			
		}
		else if(lSender->objectName()=="InputTurthBtn_FoM")
		{
			ui.input_path_truth_2->setText(fileName);
			serialnum=2;
			if (sFoM_Reader.size()==3)
			{
				sFoM_Reader[serialnum-1]._pread->close();
				vector<fomReader>::iterator it = sFoM_Reader.begin()+serialnum-1;
				sFoM_Reader.erase(it);
			}
		}
		else
		{
			ui.input_path_compared_2->setText(fileName);
			serialnum=3;
			if (sFoM_Reader.size()==3)
			{
				sFoM_Reader[serialnum-1]._pread->close();
				vector<fomReader>::iterator it = sFoM_Reader.begin()+serialnum-1;
				sFoM_Reader.erase(it);
			}
		}

		openfileforFoM(&fileName,serialnum);
	}
}

/// <summary>
/// <清除>
/// </summary>
void KappaCalculate::clearFoMfile()
{
	if (sFoM_Reader.size()!=0)
	{
		for (int ii=0;ii<sFoM_Reader.size();ii++)
		{
			if (sFoM_Reader[ii]._pread!=NULL)
			{
				sFoM_Reader[ii]._pread->close();
			}
		}
	}
	vCompd_poDataset.clear();
}

void KappaCalculate::startcalculateFoM()
{

	if (ui.rbtnFoM->isChecked()==true)
	{
		mode=1;
	}
	else
	{
		mode=2;
	}

	fcr=new FoMcalculator(this,sFoM_Reader,mode);
 	KappaThread* ftrd=new KappaThread(fcr);
 	fcr->moveToThread(ftrd);// <这句似乎不加也可以>
 	ftrd->start();

	if (ftrd->isRunning())
	{
		QEventLoop loop;
		connect(ftrd, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in FoM thread!");
		return;
	}

	QMessageBox::information(this,"Message","Complete the calculation!");

	//clearFoMfile();

	read2texteditFoM();
}


void KappaCalculate::read2texteditFoM()
{

	QString str;
	if (ui.rbtnFoM->isChecked()==true)
	{
		str="./FilesGenerate/FoM.csv";
	}
	else
	{
		str="./FilesGenerate/Kappa_changepart.csv";
	}
	QFile file(str);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString str1;
		str1="New "+str+" error!";
		qDebug()<<str1;
		return;
	}
	ui.textAccuracy_FoM->clear();
	while (!file.atEnd()) 
	{
		QByteArray line = file.readLine();
		ui.textAccuracy_FoM->append(line);
	}
	file.close();
	//ui.btnStart_FoM->setEnabled(false);
}










