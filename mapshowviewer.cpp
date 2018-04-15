#include "mapshowviewer.h"
#include <iostream>
#include <QtCore/QList>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QGraphicsPixmapItem>
#include <iostream>
using namespace std;

MapShowViewer::MapShowViewer(QWidget *parent)//QWidget *parent,
	:  QGraphicsView(parent)
{
	isfinished=0;
	m_scaleFactor=1.0;
	m_level=1.0;
	picScene = NULL;
	u_rgbShowArray= NULL;
	u_sketchBand= NULL;
	u_tmpArray= NULL;
	u_rgbArray= NULL;
}

MapShowViewer::~MapShowViewer()
{
	Gra_show.clear();
	if (picScene!=NULL)
	{
		picScene->clear();
		delete[] picScene;
		picScene=NULL;
	}
    if (u_rgbShowArray!=NULL)
    {
		delete[] u_rgbShowArray;
		u_rgbShowArray=NULL;
    }
}

/// <summary>
/// <RGB或Gray显示选中图像>
/// </summary>
bool MapShowViewer::chooseImageToshow( QList<TiffDataRead*> _inUi_poDataset, int _serialnum,QList<int> _ischecked)
{
	// <先析构>
	if (_inUi_poDataset.size()==0||_ischecked.size()!=3)
	{
		QMessageBox::about(NULL,"title","input wrong!");
		return false;
	}
	deleteForRenew();

	// <提取基本参数>
	_serialcode=_serialnum;
	cols = _inUi_poDataset[_serialcode]->cols();
	rows = _inUi_poDataset[_serialcode]->rows();
	bands = _inUi_poDataset[_serialcode]->bandnum();
	perPsize = _inUi_poDataset[_serialcode]->perPixelSize();
	dataLength = _inUi_poDataset[_serialcode]->datalength()/bands;
	noValue = _inUi_poDataset[_serialcode]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	_dataType = _inUi_poDataset[_serialcode]->datatype();

	getScalePara(false);// <是否原尺寸显示>

	u_rgbArray=new unsigned char[_iScaleHeight*_iScaleWidth*3];
	u_tmpArray=new unsigned char[_iScaleHeight*_iScaleWidth*perPsize];

	for (int ii=0;ii<3;ii++)
	{
		Gra_show.append(_inUi_poDataset[_serialcode]->poDataset()->GetRasterBand(_ischecked[ii]+1));
		Gra_show[ii]->RasterIO(GF_Read, 0, 0, cols, rows, u_tmpArray, _iScaleWidth,_iScaleHeight,_dataType, 0, 0 );
		styleBaseSketch(u_tmpArray,_dataType);
		for (int jj=0;jj<_iScaleHeight*_iScaleWidth;jj++)
		{
			u_rgbArray[ii*_iScaleHeight*_iScaleWidth+jj]=u_sketchBand[jj];
		}
	}

	destoryCharPointer(u_tmpArray);
	destoryCharPointer(u_sketchBand);
	// <将三个波段组合起来>
	int bytePerLine = (_iScaleWidth* 24 + 31)/8;
	u_rgbShowArray = new unsigned char[bytePerLine * _iScaleHeight * 3];
	for( int h = 0; h < _iScaleHeight; h++ )
	{
		for( int w = 0; w < _iScaleWidth; w++)
		{
			for (int ii=0;ii<3;ii++)
			{
				u_rgbShowArray[h * bytePerLine + w * 3 +ii] = u_rgbArray[ii*_iScaleHeight*_iScaleWidth + h * _iScaleWidth + w];
			}
		}
	}
	destoryCharPointer(u_rgbArray);
	// <构造图像并显示>
	QGraphicsPixmapItem *imgItem;
	imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(u_rgbShowArray, _iScaleWidth, _iScaleHeight, bytePerLine,QImage::Format_RGB888)));
	picScene = new QGraphicsScene();
	picScene->addItem(imgItem);
	this->setScene(picScene);

	return true;
}
/// <summary>
/// <不同类型数据转换成char用于显示，用两个函数配合，函数1和函数2>
/// <函数1>
/// </summary>
template<class TT> bool MapShowViewer::imgSketch2uArray(unsigned char* buffer)
{
	u_sketchBand = new unsigned char[_iScaleHeight*_iScaleWidth];
	Gra_show[0]->ComputeRasterMinMax(1,minmax);
	min=minmax[0];
	max=minmax[1];

	if( min <= noValue && noValue<= max )
	{
		min = 0;
	}
	for ( int i = 0; i <_iScaleHeight*_iScaleWidth; i++ )
	{
		TT _temp=*(TT*)(buffer+i*sizeof(TT));
		if (_temp> max||_temp==noValue)
		{
			u_sketchBand[i] = 255;
		}
		else if (_temp<= max&&_temp>= min )
		{
			u_sketchBand[i] =static_cast<uchar>( 255 - 255 * ( max - _temp) / ( max - min ));
		}
		else
		{
			u_sketchBand[i] = 0;
		}
	}
	return true;
}
/// <summary>
/// <函数2>
/// </summary>
bool MapShowViewer::styleBaseSketch(unsigned char* _buffer,GDALDataType _datatype)
{
	//get data
	bool bRlt = false;
	switch(_datatype)
	{
	case GDT_Byte:
		bRlt = imgSketch2uArray<unsigned char>(_buffer);
		break;
	case GDT_UInt16:
		bRlt = imgSketch2uArray<unsigned short>(_buffer);
		break;
	case GDT_Int16:
		bRlt = imgSketch2uArray<short>(_buffer);
		break;
	case GDT_UInt32:
		bRlt = imgSketch2uArray<unsigned int>(_buffer);
		break;
	case GDT_Int32:
		bRlt = imgSketch2uArray<int>(_buffer);
		break;
	case GDT_Float32:
		bRlt = imgSketch2uArray<float>(_buffer);
		break;
	case GDT_Float64:
		bRlt = imgSketch2uArray<double>(_buffer);
		break;
	default:
		cout<<"CGDALRead::loadFrom : unknown data type!"<<endl;
		close();
		return false;
	}

	if (bRlt == false)
	{
		QMessageBox::information(this,
			tr("Error"),
			tr("Other Data Type!"));
		close();
		return false;
	}
	return true;
}
/// <summary>
/// <删除指针>
/// </summary>
void MapShowViewer::destoryCharPointer(unsigned char* _destroied)
{
	if (_destroied!=NULL)
	{
		delete[] _destroied;
	}
	_destroied=NULL;
}
/// <summary>
/// <删除显示用的u_rgbShowArray>
/// </summary>
bool MapShowViewer::deleteForRenew()
{
// 	for (int ii=0;ii<Gra_show.size();ii++)
// 	{
// 		GDALClose(Gra_show.at(ii));
// 		Gra_show.removeAt(ii);
// 	}
	Gra_show.clear();

	for (int ii=0;ii<imgItemlist.size();ii++)
	{
		delete[] imgItemlist.at(ii);
		imgItemlist.removeAt(ii);
	}
	imgItemlist.clear();

	if (picScene!=NULL)
	{
		picScene->clear();
		delete[] picScene;
		picScene=NULL;
	}
	if (u_rgbShowArray!=NULL)
	{
		delete[] u_rgbShowArray;
		u_rgbShowArray=NULL;
	}
	return true;
}
/// <summary>
/// <获取基本信息>
/// </summary>
void  MapShowViewer::getScalePara(bool _iniornot)
{
	if (_iniornot==false)
	{
		// <压缩或扩张参数>
		double m_scaleFactor = this->height() * 1.0 /rows;
		_iScaleWidth= (int)(cols*m_scaleFactor-1);
		_iScaleHeight =(rows*m_scaleFactor-1);
	}
	else
	{
		_iScaleWidth = cols;
		_iScaleHeight = rows;
	}
}
/// <summary>
/// <动态显示函数>
/// </summary>
void MapShowViewer::dynamicShow(unsigned char* ___u_rgbArray,unsigned char* ___u_rgbShowArray,int _rows,int _cols,double _m_scaleFactor)
{

	//isfinished=false;

	m_level=this->height()*1.0/_rows*1.0; // 修改标准，默认是1

	int bytePerLine = (_cols* 24 + 31)/8;
	for( int h = 0; h < _rows; h++ )
	{
		for( int w = 0; w < _cols; w++)
		{
			for (int ii=0;ii<3;ii++)
			{
				___u_rgbShowArray[h * bytePerLine + w * 3 +ii] = ___u_rgbArray[ii*_rows*_cols + h * _cols + w];
			}
		}
	}
	// <构造图像并显示>
    
    QGraphicsPixmapItem * imgItem_child = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(___u_rgbShowArray, _cols, _rows, bytePerLine,QImage::Format_RGB888)));
	imgItemlist.append(imgItem_child);

	
	picScene = new QGraphicsScene(this);
	picScene->addItem(imgItemlist.at(imgItemlist.size()-1));
	this->scale(_m_scaleFactor,_m_scaleFactor);
	this->setScene(picScene);
	this->viewport()->repaint();

	if (imgItemlist.size()>1)
	{
		delete[] imgItemlist.at(0);
		imgItemlist.removeAt(0);
	}

	isfinished++;

}
/// <summary>
/// <颜色条（不尽人意）>
/// </summary>
void MapShowViewer::changecolorbar()
{
	int bytePerLine = (_iScaleWidth* 24 + 31)/8;
	for( int h = 0; h < _iScaleHeight; h++ )
	{
		for( int w = 0; w < _iScaleWidth; w++)
		{
			if ((u_rgbShowArray[h * bytePerLine + w * 3 +0]+u_rgbShowArray[h * bytePerLine + w * 3 +1]+u_rgbShowArray[h * bytePerLine + w * 3 +2])<255*3)
			{
				u_rgbShowArray[h * bytePerLine + w * 3 +0] = static_cast<uchar>(u_rgbShowArray[h * bytePerLine + w * 3 +0]);
				u_rgbShowArray[h * bytePerLine + w * 3 +1] =static_cast<uchar>(255-u_rgbShowArray[h * bytePerLine + w * 3 +1]);
				u_rgbShowArray[h * bytePerLine + w * 3 +2] =static_cast<uchar>(255-u_rgbShowArray[h * bytePerLine + w * 3 +2]);
			}
		}
	}
	// <构造图像并显示>
	QGraphicsPixmapItem *imgItem;
	imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(u_rgbShowArray, _iScaleWidth, _iScaleHeight, bytePerLine,QImage::Format_RGB888)));
	picScene = new QGraphicsScene();
	picScene->addItem(imgItem);
	this->setScene(picScene);
	this->viewport()->repaint();
}


// / <------------------------------------------------------算法无关操作---------------------------------------------------------->
// 
// 
///<summary>
///鼠标滚轮事件，实现图像缩放
///</summary>
///<param name="event">滚轮事件</param>
void MapShowViewer::wheelEvent( QWheelEvent *event )
{
	// 滚轮向上滑动，放大图像
	if ( event->delta() > 0 )
	{
		ZoomIn();
	}
	// 滚轮向下滑动，缩小图像
	if ( event->delta() < 0 )
	{
		ZoomOut();
	}
}

///<summary>
///鼠标按键按下事件
///</summary>
///<param name="event">鼠标事件.</param>
void MapShowViewer::mousePressEvent( QMouseEvent *event )
{
	// 滚轮键按下，平移图像
	if ( event->button() == Qt::MidButton )
	{
		this->setDragMode(QGraphicsView::ScrollHandDrag );
		this->setInteractive( false );
		lastEventCursorPos = event->pos();
	}
}

///<summary>
/// 鼠标移动事件
///</summary>
///<param name="event">鼠标事件</param>
void MapShowViewer::mouseMoveEvent( QMouseEvent *event )
{
	if ( this->dragMode() ==QGraphicsView::ScrollHandDrag )
	{
		QPoint delta = ( event->pos() -lastEventCursorPos ) / 10;
		this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + ( isRightToLeft() ? delta.x() :-delta.x() ) );
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - delta.y() );
		this->viewport()->setCursor(Qt::ClosedHandCursor );
	}

}

///<summary>
///鼠标按键释放事件
///</summary>
///<param name="event">鼠标事件</param>
void MapShowViewer::mouseReleaseEvent( QMouseEvent *event )
{
	if ( event->button() == Qt::MidButton )
	{
		this->setDragMode(QGraphicsView::NoDrag );
	}
}