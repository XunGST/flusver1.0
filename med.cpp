#include "med.h"
#include "threadmed.h"
#include "morpherodla.h"
#include "QDateTime"
#include "QDir"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

med::med(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//构造文件路径
	QDir *temp = new QDir;
	bool exist = temp->exists("./FilesGenerate");
	if(!exist)
	{
		bool ok = temp->mkdir("./FilesGenerate");
	}

	this->setWindowTitle("Morphological Erosion and Dilation (MED)");
	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(ui.btnOpenImg,SIGNAL(clicked()),this,SLOT(openInputImage())); 
	connect(ui.btnSaveImg,SIGNAL(clicked()),this,SLOT(saveOutputImage())); 
	connect(ui.btnRun,SIGNAL(clicked()),this,SLOT(runDelineation())); 
	ui.lineWinSize->setText(QString::number(3));
}

med::~med()
{

}

void med::openInputImage()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick one image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		ui.lineInput->setText(fileName);
	}
	else
	{
		qDebug()<<"Read Image Path Error!";
	}
}

void med::saveOutputImage()
{
	QString fileName = QFileDialog::getSaveFileName(
		this,
		tr( "Input a image name to save..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif)" ) );
	if (!fileName.isNull())
	{
		ui.lineOutput->setText(fileName);
	}
	else
	{
		qDebug()<<"Write Image Path Error!";
	}
}

void med::runDelineation()
{
	QString qsInputPath=ui.lineInput->text();
	QString qsOutputPath=ui.lineOutput->text();
	QString qsWindowSize=ui.lineWinSize->text();
	int iNumWin=qsWindowSize.toInt();
	if (iNumWin%2==0)
	{
		iNumWin+=1; // 偶数变奇数
	}
	QFile file("./FilesGenerate/ParameterFile.tmp");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	out<<qsInputPath<<endl;
	out<<qsOutputPath<<endl;
	out<<QString::number(iNumWin)<<endl;
	file.close();

	mMed=new MorphEroDla(this);

	threadmed* tMed=new threadmed(this,mMed);

	mMed->moveToThread(tMed);

	tMed->start();

	if (tMed->isRunning())
	{
		QEventLoop loop;
		connect(tMed, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
	}
	else
	{
		QMessageBox::warning(this,"Error","Something wrong in UGB calculation!");
		return;
	}

	QMessageBox::information(this,"Message",tr("Complete the calculation!\nWindow Size: %1").arg(iNumWin));


}
