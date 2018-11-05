#include "predictionform.h"
#include "markovplayer.h"
#include "predictionthread.h"
#include "QDateTime"
#include "QDir"
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

predictionform::predictionform(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowTitle(tr("Markov Chain"));
	this->setAttribute(Qt::WA_DeleteOnClose);
	QIcon prebicon(":/new/prefix1/预测.png");
	this->setWindowIcon(prebicon);

	ui.cbxPredictYear->setEnabled(false);

	ui.btnStart->setEnabled(false);

	//构造文件路径
	QDir *temp = new QDir;
	bool exist = temp->exists("./FilesGenerate");
	if(!exist)
	{
		bool ok = temp->mkdir("./FilesGenerate");
	}


	connect(ui.btnOpenStart,SIGNAL(clicked()),this,SLOT(openStartFile())); 
	connect(ui.btnOpenEnd,SIGNAL(clicked()),this,SLOT(openEndFile())); 
	connect(ui.letStartYear,SIGNAL(editingFinished()),this,SLOT(decideEnable())); 
	connect(ui.letEndYear,SIGNAL(editingFinished()),this,SLOT(decideEnable())); 
	connect(ui.btnStart,SIGNAL(clicked()),this,SLOT(runMakovChain())); 

}

predictionform::~predictionform()
{
	clearAllStart();
	clearAllEnd();
}


void predictionform::openStartFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		clearAllStart();
		startLoadImage(&fileName);
		if (vStartpoDataset.at(0)->bandnum()!=1)
		{
			this->clearAllStart(); 
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.letStart->setText(fileName);
	}
}

void predictionform::openEndFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		clearAllEnd();
		endLoadImage(&fileName);
		if (vStartpoDataset.at(0)->bandnum()!=1)
		{
			this->clearAllEnd(); 
			QMessageBox::warning(this,"Error","Land use data has only one band!");
			return;
		}
		ui.letEnd->setText(fileName);
	}
}

bool predictionform::startLoadImage(QString* _fileName)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	vStartpoDataset.append(pread);

	// <提取数据>
	if (!vStartpoDataset[0]->loadFrom(_fileName->toStdString().c_str()))
	{
		//cout<<"load error!"<<endl;
	}
	else
	{
		//cout<<"load success!"<<endl;
	}

	return true;
}

void predictionform::clearAllStart()
{
	if (vStartpoDataset.size()!=0)
	{
		if (vStartpoDataset[0]!=NULL)
		{
			vStartpoDataset[0]->close();
		}
	}
	vStartpoDataset.clear();
}

bool predictionform::endLoadImage(QString* _fileName)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	// <存入>
	vEndpoDataset.append(pread);

	// <提取数据>
	if (!vEndpoDataset[0]->loadFrom(_fileName->toStdString().c_str()))
	{
		//cout<<"load error!"<<endl;
	}
	else
	{
		//cout<<"load success!"<<endl;
	}

	return true;
}

void predictionform::clearAllEnd()
{
	if (vEndpoDataset.size()!=0)
	{
		if (vEndpoDataset[0]!=NULL)
		{
			vEndpoDataset[0]->close();
		}
	}
	vEndpoDataset.clear();
}

int get_num(int x)
{
	int i=0;
	while(x!=0)
	{
		x=static_cast<int>(x/10);
		i++;
	}
	return i;
}

void predictionform::decideEnable()
{
	if (ui.letStart->text().isEmpty()!=true&&ui.letEnd->text().isEmpty()!=true)
	{
		if (ui.letStartYear->text().isEmpty()!=true&&ui.letEndYear->text().isEmpty()!=true)
		{
			startyear=ui.letStartYear->text().toInt();
			endyear=ui.letEndYear->text().toInt();
		
			if (get_num(startyear)>4||get_num(endyear)>4)
			{
				ui.btnStart->setEnabled(false);
				ui.cbxPredictYear->setEnabled(false);
				ui.cbxPredictYear->clear();
				return;
			}
			else
			{
				if (startyear>=endyear)
				{
					ui.btnStart->setEnabled(false);
					ui.cbxPredictYear->setEnabled(false);
					ui.cbxPredictYear->clear();
					return;
				}
				else
				{
					ui.btnStart->setEnabled(true);
					ui.cbxPredictYear->setEnabled(true);
					generatefutureyear();
				}
			}
		}
	}
}


void predictionform::generatefutureyear()
{

	ui.cbxPredictYear->clear();

	int midyear=endyear-startyear;

	int numyear=5000/midyear;

	for (int i=0;i<numyear;i++)
	{
		ui.cbxPredictYear->addItem(QWidget::tr("%1").arg(endyear+ (i + 1)* midyear));
	}
	
}

void predictionform::runMakovChain()
{
	int predictyear=ui.cbxPredictYear->itemText(ui.cbxPredictYear->currentIndex()).toInt();

	mkp=new markovPlayer(vStartpoDataset,vEndpoDataset,startyear,endyear,predictyear);

	predictionthread* prd=new predictionthread(this,mkp);
	
	mkp->moveToThread(prd);

	prd->start();

	if (prd->isRunning())
	{
		QEventLoop loop;
		connect(prd, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in kappa thread!");
		return;
	}

	QMessageBox::information(this,"Message","Complete the calculation!");

	read2textedit();
}

void predictionform::read2textedit()
{
	QFile file("./FilesGenerate/MakovChain.csv");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug()<<"New Kappa.csv Error!";
		return;
	}
	ui.textEdit->clear();
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		ui.textEdit->append(line);
	}
	file.close();
}


