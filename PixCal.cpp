#include "PixCal.h"
#include "TiffDataRead.h"
#include "geosimulator.h"
#include "dynasimulation.h"
#include <iostream>
#include <QtGui>
#include <QtCore>
#include <algorithm>
#include <vector>
using namespace std;



PixCal::PixCal(QObject *parent,QString logFilePath,int numLine)
{

	mdNumline=numLine;
	QFile file(logFilePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	//QTextStream in(&file);
	while (!file.atEnd()) 
	{
		QString line = file.readLine();
		mvLineList.push_back(line);
	}
	file.close();

	pixcalculate();

}

PixCal::~PixCal()
{
	pread->close();
	mvLineList.clear();
}

// <巧妙的排序>
struct DoubleList
{
	int mb1;
	int mb2;
};
bool sortbymb1(const DoubleList &v1,const DoubleList &v2)
{
	return v1.mb1<v2.mb1;
};
void myPushback(vector<DoubleList> &vecdList,const int &m1,const int &m2)
{
	DoubleList dlist;
	dlist.mb1=m1;
	dlist.mb2=m2;
	vecdList.push_back(dlist);
}

///
/// <土地利用数据>
///
bool PixCal::lauLoadImage2( QString _fileName )
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");


	string stdfilenamestr=_fileName.trimmed().toStdString(); // 不分开居然读不进来，醉了


	// <新建IO类>
	pread = new TiffDataRead;


	// <提取数据>
	if (!pread->loadFrom(stdfilenamestr.c_str()))
	{
		cout<<"load error!"<<endl;
		return false;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	return true;
}



// <公用线程>
void PixCal::pixcalculate()
{
	if (mvLineList.size()!=0)
	{

		bool bRlt=lauLoadImage2(mvLineList[mdNumline]);
		
		GDALDataType mgDataType=pread->datatype();

		int mnPerPixSize=pread->perPixelSize();

		switch(mgDataType)
		{
		case GDT_Byte:
			mnPerPixSize = sizeof(unsigned char);
			bRlt = readData<unsigned char>();
			break;
		case GDT_UInt16:
			mnPerPixSize = sizeof(unsigned short);
			bRlt = readData<unsigned short>();
			break;
		case GDT_Int16:
			mnPerPixSize = sizeof(short);
			bRlt = readData<short>();
			break;
		case GDT_UInt32:
			mnPerPixSize = sizeof(unsigned int);
			bRlt = readData<unsigned int>();
			break;
		case GDT_Int32:
			mnPerPixSize = sizeof(int);
			bRlt = readData<int>();
			break;
		case GDT_Float32:
			mnPerPixSize = sizeof(float);
			bRlt = readData<float>();
			break;
		case GDT_Float64:
			mnPerPixSize = sizeof(double);
			bRlt = readData<double>();
			break;
		default:
			cout<<"CGDALRead::loadFrom : unknown data type!"<<endl;
			return ;
		}

		if (bRlt == false)
		{
			cout<<"CGDALRead::loadFrom : read data error!"<<endl;
			return ;
		}

		pread->close();

	}
}


/////////////////////为概率计算模拟重载////////////////////////////////
template<class TT> bool PixCal::readData()
{
	QList<int> rgbLanduseType;
	QList<int> stastisticCount;

	TT _temp=*(TT*)(pread->imgData()+0*sizeof(TT));

	rgbLanduseType.append(_temp);

	stastisticCount.append(0);

	int miHeight=pread->rows();

	int miWidth=pread->cols();
	
	int _size;

	int _countEq;

	TT temp;

	long ii;

	double __tmp;

	for (ii=0;ii<miHeight*miWidth;ii++)
	{
		_size=rgbLanduseType.size();

		temp=*(TT*)(pread->imgData()+ii*sizeof(TT));

		_countEq=0;

		for (int kk=0;kk<_size;kk++)
		{
			if (temp!=rgbLanduseType[kk])
			{
				_countEq++;
			}
			else
			{
				stastisticCount[kk]++;
			}
			if (_countEq==_size)
			{
				rgbLanduseType.append(temp);
				stastisticCount.append(1);
			}
		} 
		
	}

	vector<DoubleList> dList;

	for (int kk=0;kk<_size;kk++)
	{
		myPushback(dList,rgbLanduseType.at(kk),stastisticCount.at(kk));
	}
	sort(dList.begin(),dList.end(),sortbymb1);

	for (int kk=0;kk<_size;kk++)
	{
		rgbLanduseType[kk]=dList[kk].mb1;
		stastisticCount[kk]=dList[kk].mb2;
	}

	dList.clear();

	// <新建tmp文件保存导入路径>
	QFile tmpfile("./FilesGenerate/file.tmp");
	if (!tmpfile.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&tmpfile);
	for (int ii=0;ii<rgbLanduseType.size();ii++)
	{
		out<<QString::number(rgbLanduseType.at(ii))<<endl;
		out<<QString::number(stastisticCount.at(ii))<<endl;
	}

	tmpfile.close();

	return true;
}

