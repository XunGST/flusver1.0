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


PixCal::PixCal(GeoSimulator* _gsl )
{
	m_gsl=_gsl;
	m_ds=NULL;
}

PixCal::PixCal( DynaSimulation* _ds,bool pattern)
{
	m_gsl=NULL;
	m_ds=_ds;
}

PixCal::~PixCal()
{
	
}

// <巧妙的排序>
struct Test
{
	int mb1;
	int mb2;
};
bool sortbymb1(const Test &v1,const Test &v2)
{
	return v1.mb1<v2.mb1;
};
void myPushback(vector<Test> &vecTest,const int &m1,const int &m2)
{
	Test test;
	test.mb1=m1;
	test.mb2=m2;
	vecTest.push_back(test);
}
// <公用线程>
void PixCal::pixcalculate()
{
	if (m_gsl!=NULL)
	{
		bool bRlt;

		GDALDataType mgDataType=m_gsl->lau_poDataset.at(0)->datatype();

		int mnPerPixSize=m_gsl->lau_poDataset.at(0)->perPixelSize();

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
	}
	else
	{
		bool bRlt;

		GDALDataType mgDataType=m_ds->lau_poDataset2.at(0)->datatype();

		int mnPerPixSize=m_ds->lau_poDataset2.at(0)->perPixelSize();

		switch(mgDataType)
		{
		case GDT_Byte:
			mnPerPixSize = sizeof(unsigned char);
			bRlt = readData2<unsigned char>();
			break;
		case GDT_UInt16:
			mnPerPixSize = sizeof(unsigned short);
			bRlt = readData2<unsigned short>();
			break;
		case GDT_Int16:
			mnPerPixSize = sizeof(short);
			bRlt = readData2<short>();
			break;
		case GDT_UInt32:
			mnPerPixSize = sizeof(unsigned int);
			bRlt = readData2<unsigned int>();
			break;
		case GDT_Int32:
			mnPerPixSize = sizeof(int);
			bRlt = readData2<int>();
			break;
		case GDT_Float32:
			mnPerPixSize = sizeof(float);
			bRlt = readData2<float>();
			break;
		case GDT_Float64:
			mnPerPixSize = sizeof(double);
			bRlt = readData2<double>();
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
	}
}


/////////////////////为概率计算模拟重载////////////////////////////////
template<class TT> bool PixCal::readData()
{

	m_gsl->rgbLanduseType.clear();

	int _height=m_gsl->lau_poDataset.at(0)->rows();

	int _width=m_gsl->lau_poDataset.at(0)->cols();

	TT _temp=*(TT*)(m_gsl->lau_poDataset.at(0)->imgData()+0*sizeof(TT));

	m_gsl->rgbLanduseType.append(_temp);

	m_gsl->staCount.append(0);

	int _size;

	int _countEq;

	TT temp;

	long ii;

	double __tmp;

	m_gsl->_landuse=new unsigned char[_width*_height];

	for (ii=0;ii<_height*_width;ii++)
	{
		_size=m_gsl->rgbLanduseType.size();

		temp=*(TT*)(m_gsl->lau_poDataset.at(0)->imgData()+ii*sizeof(TT));

		if ((temp>0&&temp<=255)&&temp!=m_gsl->lau_poDataset[0]->invalidValue())
		{
			m_gsl->_landuse[ii]=(unsigned char)temp;
		}	
		else
		{
			m_gsl->_landuse[ii]=(unsigned char)0;
		}

		if (_size>15)
		{
			m_gsl->rgbLanduseType.clear();
			return false;
		}

		_countEq=0;

		for (int kk=0;kk<_size;kk++)
		{
			if (temp!=m_gsl->rgbLanduseType[kk])
			{
				_countEq++;
			}
			else
			{
				m_gsl->staCount[kk]++;
			}
			if (_countEq==_size)
			{
				m_gsl->rgbLanduseType.append(temp);
				m_gsl->staCount.append(1);
			}
		} 
		
	}

	vector<Test> save_arr;
	for (int kk=0;kk<_size;kk++)
	{
		myPushback(save_arr,m_gsl->rgbLanduseType.at(kk),m_gsl->staCount.at(kk));
	}
	sort(save_arr.begin(),save_arr.end(),sortbymb1);

	for (int kk=0;kk<_size;kk++)
	{
		m_gsl->staCount[kk]=save_arr[kk].mb2;
	}

	save_arr.clear();

	qSort(m_gsl->rgbLanduseType.begin(), m_gsl->rgbLanduseType.end());

	return true;
}

/////////////////////为动态模拟重载////////////////////////////////
template<class TT> bool PixCal::readData2()
{

	m_ds->rgbLanduseType2.clear();

	int _height=m_ds->lau_poDataset2.at(0)->rows();

	int _width=m_ds->lau_poDataset2.at(0)->cols();

	TT _temp=*(TT*)(m_ds->lau_poDataset2.at(0)->imgData()+0*sizeof(TT));

	m_ds->rgbLanduseType2.append(_temp);

	m_ds->staCount2.append(1);

	int _size;

	int _countEq;

	TT temp;

	long ii;

	double __tmp;

	//m_ds->_landuse2=new unsigned char[_width*_height]; 《不在这开辟空间》

	for (ii=0;ii<_height*_width;ii++)
	{
		_size=m_ds->rgbLanduseType2.size();

		temp=*(TT*)(m_ds->lau_poDataset2.at(0)->imgData()+ii*sizeof(TT));

		if ((temp>0&&temp<=255)&&temp!=m_ds->lau_poDataset2[0]->invalidValue())
		{
			m_ds->_landuse2[ii]=(unsigned char)temp;
		}	
		else
		{
			m_ds->_landuse2[ii]=(unsigned char)0;
		}

		if (_size>15)
		{
			m_ds->rgbLanduseType2.clear();
			return false;
		}

		_countEq=0;

		for (int kk=0;kk<_size;kk++)
		{
			if (temp!=m_ds->rgbLanduseType2[kk])
			{
				_countEq++;
			}
			else
			{
				m_ds->staCount2[kk]++;
			}
			if (_countEq==_size)
			{
				m_ds->rgbLanduseType2.append(temp);
				m_ds->staCount2.append(1);
			}
		} 

	}

	vector<Test> save_arr;
	for (int kk=0;kk<_size;kk++)
	{
		myPushback(save_arr,m_ds->rgbLanduseType2.at(kk),m_ds->staCount2.at(kk));
	}
	sort(save_arr.begin(),save_arr.end(),sortbymb1);

	for (int kk=0;kk<_size;kk++)
	{
		m_ds->staCount2[kk]=save_arr[kk].mb2;
	}

	save_arr.clear();

	qSort(m_ds->rgbLanduseType2.begin(), m_ds->rgbLanduseType2.end());

	return true;
}


