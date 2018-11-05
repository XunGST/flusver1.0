#include "geodpcasys.h"
#include "showproperty.h"
#include <iostream>
#include <QtGui/QFileDialog>
#include <QApplication>
#include <QLocale>
#include <QLibraryInfo>
#include "mapshowviewer.h"
#include "dynasimulation.h"
#include "predictionform.h"
#include "med.h"

using namespace std;

GeoDpCAsys::GeoDpCAsys(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{

	ui.setupUi(this);
	this->setWindowTitle("FLUS model");

	ifPickFile=false;
	serialNum=0;
	child_serialNum=0;
	NumImage=0;

	QIcon qicon(":/new/prefix1/中大.png");
	this->setWindowIcon(qicon);

	QIcon openicon(":/new/prefix1/打开.png");
	this->ui.actionOpen->setIcon(openicon);

	QIcon clearicon(":/new/prefix1/清除.png");
	this->ui.actionClear->setIcon(clearicon);

	QIcon probicon(":/new/prefix1/概率.png");
	this->ui.actionGeo_Simulation->setIcon(probicon);

	QIcon neibicon(":/new/prefix1/邻域.png");
	this->ui.actionDynamic_Simulation->setIcon(neibicon);

	QIcon qicona(":/new/prefix1/关于.png");
	ui.actionAbout->setIcon(qicona);

	QIcon qiconZi(":/new/prefix1/zoomIn.png");
	ui.actionZoom_In->setIcon(qiconZi);

	QIcon qiconZo(":/new/prefix1/zoomOut.png");
	ui.actionZoom_Out->setIcon(qiconZo);

	QIcon qiconclose(":/new/prefix1/关闭.png");
	ui.actionExit->setIcon(qiconclose);

	QIcon qiconfe(":/new/prefix1/还原.png");
	ui.actionFull_Extent->setIcon(qiconfe);

	ui.pro_tView->setModel(ui.pro_tView->FileListModel());
	ui.pro_tView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.pro_tView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui.actionOpen,SIGNAL(triggered()),this,SLOT(pickOpenFile()));
	connect(ui.actionClear,SIGNAL(triggered()),this,SLOT(clearAll()));
	connect(ui.pro_tView,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(showMouseRightMenu(const QPoint &)));	
	connect(ui.rCBox,SIGNAL(activated(int)),this,SLOT(comboBoxChange()));
	connect(ui.gCBox,SIGNAL(activated(int)),this,SLOT(comboBoxChange()));
	connect(ui.bCBox,SIGNAL(activated(int)),this,SLOT(comboBoxChange()));
	connect(ui.actionGeo_Simulation,SIGNAL(triggered()),this,SLOT(geosimulatorstart()));
	connect(ui.actionDynamic_Simulation,SIGNAL(triggered()),this,SLOT(dynasimulationprocess()));
	connect(ui.actionAbout,SIGNAL(triggered()),this,SLOT(infomation()));

	connect( ui.actionZoom_In, SIGNAL( triggered() ), this->ui.pro_GView, SLOT( ZoomIn() ) );
	connect( ui.actionZoom_Out, SIGNAL( triggered() ), this->ui.pro_GView, SLOT( ZoomOut() ) );
	connect( ui.actionFull_Extent, SIGNAL( triggered() ), this->ui.pro_GView, SLOT( ZoomFit() ) );
	connect( ui.actionExit, SIGNAL( triggered() ), this, SLOT(close()) );
	connect( ui.actionMarkov_chain,SIGNAL( triggered() ),this,SLOT(openMarkov()));
	connect( ui.actionMorphological_method,SIGNAL( triggered() ),this,SLOT(openMED()));
}

GeoDpCAsys::~GeoDpCAsys()
{
	this->clearAll();
	this->close();
}

bool GeoDpCAsys::loadImage( const char* _fileName )
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	serialNum=NumImage;

	TiffDataRead* pread = new TiffDataRead;

	Ui_poDataset.append(pread);

	if (!Ui_poDataset[serialNum]->loadFrom(_fileName))
	{
		cout<<"load error!"<<endl;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	NumImage=Ui_poDataset.size();
	return true;
}

void GeoDpCAsys::pickOpenFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr( "Pick a image file to open..." ),
		QDir::currentPath(),
		tr( "tiff(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
	if ( !fileName.isNull() )
	{
		this->loadImage(fileName.toStdString().c_str());
		ifPickFile=true;
		ui.pro_tView->getBandList(Ui_poDataset,serialNum);
		ui.pro_tView->expandAll();
		this->showImageOrBand();
	}
}

void GeoDpCAsys::showMouseRightMenu(const QPoint &pos)
{

	QModelIndex FaChindex = ui.pro_tView->indexAt(pos); 
	if (FaChindex.parent()==QModelIndex())
	{
		this->serialNum=FaChindex.row(); 
		child_serialNum=0;  
		
		int a;
		a=Ui_poDataset.size();
		if (serialNum>0&&Ui_poDataset[serialNum]->bandnum()>=3)
		{
			ischecked.clear();
			for (int ii=0;ii<3;ii++)
			{
				ischecked.append(ii);
			}
		}
		else
		{
			ischecked.clear();
			for (int ii=0;ii<3;ii++)
			{
				ischecked.append(child_serialNum);
			}
		}
	}
	if (FaChindex.parent()!=QModelIndex())
	{
		this->child_serialNum=FaChindex.row();
		serialNum=FaChindex.parent().row();
		ischecked.clear();
		for (int ii=0;ii<3;ii++)
		{
			ischecked.append(child_serialNum);
		}
	}
	

	QString fileName = FaChindex.data().toString();

	QMenu *qMenu = NULL;
	if (qMenu)
	{
		delete qMenu;
		qMenu = NULL;
	}                                   
	qMenu = new QMenu(ui.pro_tView);

	if (fileName.isNull()==false&&FaChindex.parent()!=QModelIndex())
	{
		ifPickFile=false;
		QAction* showBandAction = new QAction(tr("show band with GRAY").arg(serialNum).arg(child_serialNum), this);
		connect(showBandAction, SIGNAL(triggered()), this, SLOT(showImageOrBand()));
		qMenu->addAction(showBandAction);

		ifPickFile=false;
		QAction* changeColorBar = new QAction(tr("Change Color Bar").arg(serialNum), this);
		connect(changeColorBar, SIGNAL(triggered()), this->ui.pro_GView, SLOT(changecolorbar()));
		qMenu->addAction(changeColorBar);
	}

	if(fileName.isNull()==false&&FaChindex.parent()==QModelIndex())
	{
		if (Ui_poDataset[serialNum]->bandnum()<3)
		{
			ifPickFile=false;
			QAction* showGRAYAction = new QAction(tr("show image with GRAY").arg(serialNum), this);
			connect(showGRAYAction, SIGNAL(triggered()), this, SLOT(showImageOrBand()));
			qMenu->addAction(showGRAYAction);

			ifPickFile=false;
			QAction* changeColorBar = new QAction(tr("Change Color Bar").arg(serialNum), this);
			connect(changeColorBar, SIGNAL(triggered()), this->ui.pro_GView, SLOT(changecolorbar()));
			qMenu->addAction(changeColorBar);
		}
		else
		{
			ifPickFile=true;
			QAction* showRGBAction = new QAction(tr("show image with RGB").arg(serialNum), this);
			connect(showRGBAction, SIGNAL(triggered()), this, SLOT(showImageOrBand()));
			qMenu->addAction(showRGBAction);

		}

		QAction* closeChoosenAction = new QAction(tr("&Close"),this);
		connect(closeChoosenAction, SIGNAL(triggered()),this, SLOT(closeChoosenData()));
		qMenu->addAction(closeChoosenAction);

		QAction* addTreeItemAction = new QAction(tr("&Add Item"), this);
		connect(addTreeItemAction, SIGNAL(triggered()), this, SLOT(pickOpenFile()));
		qMenu->addAction(addTreeItemAction);

		QAction* _imgProperty = new QAction(tr("&Image Property"),this);
		connect(_imgProperty, SIGNAL(triggered()), this, SLOT(showImgProperty()));
		qMenu->addAction(_imgProperty);
	}
	qMenu->exec(QCursor::pos()); 
}

void GeoDpCAsys::showImageOrBand()
{
	this->comboBoxAdd();
	this->setCheck();
	ui.pro_GView->chooseImageToshow(Ui_poDataset,serialNum,ischecked);
}

void GeoDpCAsys::closeChoosenData()
{
	ifPickFile=true;
	ui.rCBox->clear();
	ui.gCBox->clear();
	ui.bCBox->clear();
	ui.pro_tView->FileListModel()->removeRow(serialNum);
	Ui_poDataset[serialNum]->close();
	Ui_poDataset.removeAt(serialNum);
	NumImage=Ui_poDataset.size();
	if (serialNum-1<0)
	{
		serialNum=NumImage-1; 
	}
	else
	{
		serialNum=serialNum-1; 
	}
	if (NumImage!=0)
	{
		this->showImageOrBand();
	}
	else
	{
		ui.pro_GView->deleteForRenew(); 
		ui.bandOpBox->setTitle(tr("RGB"));
	}
}

void GeoDpCAsys::showImgProperty()
{
	imgProperty=new ShowProperty(Ui_poDataset,serialNum);
	imgProperty->show();
}

void GeoDpCAsys::comboBoxAdd()
{
	ui.rCBox->clear();
	ui.gCBox->clear();
	ui.bCBox->clear();
	if (Ui_poDataset.size()!= 0)
	{
		QFileInfo fileInfo(Ui_poDataset[serialNum]->getFileName());
		ui.bandOpBox->setTitle(tr(fileInfo.fileName().toStdString().c_str()));
		for ( int i = 0; i <Ui_poDataset[serialNum]->poDataset()->GetRasterCount(); i++ )
		{
			ui.rCBox->addItem(QWidget::tr("Band %1").arg( i + 1 ));
			ui.gCBox->addItem(QWidget::tr("Band %1").arg( i + 1 ));
			ui.bCBox->addItem(QWidget::tr("Band %1").arg( i + 1 ));
		}
		if (ifPickFile==true)
		{
			if (Ui_poDataset[serialNum]->bandnum()>=3)
			{
				ischecked.clear();
				for (int ii=0;ii<3;ii++)
				{
					ischecked.append(ii);
				}
				child_serialNum=0; 
			}
			else
			{
				ischecked.clear();
				for (int ii=0;ii<3;ii++)
				{
					ischecked.append(child_serialNum);
				}
			}
			ifPickFile=false;
		}
		this->setGroupName();
		ui.rCBox->setCurrentIndex(ischecked[0]);
		ui.gCBox->setCurrentIndex(ischecked[1]);
		ui.bCBox->setCurrentIndex(ischecked[2]);
		child_serialNum=0; 
	}
}

void GeoDpCAsys::comboBoxChange()
{
	ischecked.clear();
	ischecked.append(ui.rCBox->currentIndex());
	ischecked.append(ui.gCBox->currentIndex());
	ischecked.append(ui.bCBox->currentIndex());
	this->setCheck();
	this->setGroupName();
	ui.pro_GView->chooseImageToshow(Ui_poDataset,serialNum,ischecked);
	child_serialNum=0; 
}

void GeoDpCAsys::setCheck()
{
	for (int ii=0;ii<ui.pro_tView->FileListModel()->rowCount();ii++)
	{
		if (ii==serialNum)
		{
			ui.pro_tView->FileListModel()->item(ii,0)->setCheckState(Qt::Checked);
		}
		else
		{
			ui.pro_tView->FileListModel()->item(ii,0)->setCheckState(Qt::Unchecked);
		}
		ui.pro_tView->FileListModel()->item(ii,0)->setCheckable(false);
	}
}

void GeoDpCAsys::setGroupName()
{
	QFileInfo fileInfo(Ui_poDataset[serialNum]->getFileName());
	if (ischecked[0]==ischecked[1]&&ischecked[1]==ischecked[2])
	{
		ui.bandOpBox->setTitle(tr(fileInfo.fileName().toStdString().c_str())+tr(" -> Band %1").arg(ischecked[0]+1));
	}
	else
	{
		ui.bandOpBox->setTitle(tr(fileInfo.fileName().toStdString().c_str()));
	}
}

void GeoDpCAsys::clearAll()
{
	ifPickFile=true;
	ui.rCBox->clear();
	ui.gCBox->clear();
	ui.bCBox->clear();
	ui.pro_tView->FileListModel()->clear();
	ui.pro_tView->FileListModel()->setHorizontalHeaderLabels(QStringList()<<"Data");
	for (int ii=0;ii<Ui_poDataset.size();ii++)
	{
		if (Ui_poDataset[ii]!=NULL)
		{
			Ui_poDataset[ii]->close();
			delete Ui_poDataset[ii];
		}
	}
	ischecked.clear();
	ui.pro_GView->deleteForRenew();
	Ui_poDataset.clear();

	serialNum=0;
	child_serialNum=0;
	NumImage=0;

}

void GeoDpCAsys::geosimulatorstart()
{
	geosim=new GeoSimulator(this);
	geosim->show();
}

void GeoDpCAsys::dynasimulationprocess()
{
	dynasim=new DynaSimulation(this);
	dynasim->show();
}

void GeoDpCAsys::infomation()
{
	QMessageBox::about(this,tr("About this model"),tr(" ").arg(deadline_year).arg(deadline_month).arg(deadline_day));
}

void GeoDpCAsys::restart()
{
	qApp->quit();
	QProcess::startDetached(qApp->applicationFilePath(), QStringList());
}

void GeoDpCAsys::openMarkov()
{
	predf=new predictionform();
	predf->show();
}

void GeoDpCAsys::openMED()
{
	mMorEroDla=new med();
	mMorEroDla->show();
}




