#include "showproperty.h"
#include "geodpcasys.h"
#include <QHBoxLayout>


ShowProperty::ShowProperty(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ShowProperty::ShowProperty(QList<TiffDataRead*> _inUi_poDataset, int _serialnum) // <重载构造函数>	
{
	ui.setupUi(this);
	this->setWindowTitle("Attribute");
	this->setAttribute(Qt::WA_DeleteOnClose);

	QIcon qicon(":/new/prefix1/属性.png");
	this->setWindowIcon(qicon);
	/// <summary>
	/// <初始化图像元数据模型>
	/// </summary>
	imgMetaModel = new QStandardItemModel(this);
	imgMetaModel->setColumnCount(2);
	imgMetaModel->setHorizontalHeaderLabels(QStringList()<<"Project"<<"Information");
	ui.pro_tableView->setModel(imgMetaModel);
	ui.pro_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.pro_tableView->verticalHeader()->setResizeMode(QHeaderView::Stretch); 
	this->getImgProperty(_inUi_poDataset,_serialnum);
}

ShowProperty::~ShowProperty()
{

}

bool ShowProperty::getImgProperty( QList<TiffDataRead*> _inUi_poDataset, int _serialnum )
{
	if (_inUi_poDataset[_serialnum]->poDataset() == NULL )
	{
		QMessageBox::about(NULL, "About", "Something wrong with image property!");
		return false;
	}
	int row = 0; // <用来记录数据模型的行号>
	// <图像的格式信息>
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Description" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem(_inUi_poDataset[_serialnum]->poDataset()->GetDriver()->GetDescription() ) );
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Meta Information" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem(_inUi_poDataset[_serialnum]->poDataset()->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME) ) ) ;
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Data Type" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( GDALGetDataTypeName( ( _inUi_poDataset[_serialnum]->poDataset()->GetRasterBand( 1)->GetRasterDataType() ) ) ) );

	// <图像的大小和波段个数>
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "X Size" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number( _inUi_poDataset[_serialnum]->poDataset()->GetRasterXSize() ) ) );
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Y Size" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number(_inUi_poDataset[_serialnum]->poDataset()->GetRasterYSize() ) ) );
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Band Count" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number(_inUi_poDataset[_serialnum]->poDataset()->GetRasterCount() ) ) );

	// <图像的投影信息>
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Projection" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem(_inUi_poDataset[_serialnum]->poDataset()->GetProjectionRef() ) );

	// <图像的坐标和分辨率信息>
	double adfGeoTransform[6];
	QString origin = "";
	QString pixelSize = "";
	if( _inUi_poDataset[_serialnum]->poDataset()->GetGeoTransform(adfGeoTransform ) == CE_None )
	{
		origin = QString::number(adfGeoTransform[0] ) + ", " + QString::number( adfGeoTransform[3] );
		pixelSize = QString::number(adfGeoTransform[1] ) + ", " + QString::number( adfGeoTransform[5] );
	}
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Origin" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( origin ) );
	imgMetaModel->setItem( row, 0, new QStandardItem( tr( "Pixel Size" ) ) );
	imgMetaModel->setItem( row++, 1, new QStandardItem( pixelSize ) );

	return true;
}

