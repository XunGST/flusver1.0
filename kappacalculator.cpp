#include "kappacalculator.h"
#include "TiffDataRead.h"
#include <QMessageBox>
#include <QtCore>
#include <vector>
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

KappaCalculator::KappaCalculator(QVector<TiffDataRead*> _turth,QVector<TiffDataRead*> _compd, double _rateofsampling, int _numofsample)
{
	turth=_turth;
	compd=_compd;

	mdsamplingRate=_rateofsampling;
	mnsamplingAmountAverage=_numofsample;

}

KappaCalculator::~KappaCalculator()
{
	turth[0]->close();
	compd[0]->close();
}



void KappaCalculator::calculatorKappa()
{

	if (turth.size()>1&&compd.size()>1)
	{
		turth[1]->close();
		compd[1]->close();
		turth.remove(0);
		compd.remove(0);
	}

	nodatat=turth[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	nodatac=compd[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();

	//maxmum=turth[0]->poDataset()->GetRasterBand(1)->GetMaximum();
	//minmum=turth[0]->poDataset()->GetRasterBand(1)->GetMinimum();

	double minmax[2];
	turth[0]->poDataset()->GetRasterBand(1)->ComputeRasterMinMax(false,minmax);
	minmum=minmax[0];
	maxmum=minmax[1];




	itrows=turth[0]->rows();
	itcols=turth[0]->cols();

	icrows=compd[0]->rows();
	iccols=compd[0]->cols();

	if (itrows!=icrows||iccols!=itcols||nodatat==maxmum||nodatat==minmum)
	{
		double minmax[2];
		minmum=turth[0]->poDataset()->GetRasterBand(1)->ComputeRasterMinMax(true,minmax);
		maxmum=minmax[0];
		minmum=minmax[1];

		return;
	}

	uturth=new unsigned char[itrows*itcols];
	ucompd=new unsigned char[iccols*icrows];

	size_t ii;
	for (ii=0;ii<itrows*itcols;ii++)
	{
		uturth[ii]=convert2uchar(turth,ii);
		ucompd[ii]=convert2uchar(compd,ii);
	}

	setRandomPoint();

	stasticpixel();

	if (uturth!=NULL||ucompd!=NULL)
	{
		delete[] uturth;
		uturth=NULL;
		delete[] ucompd;
		ucompd=NULL;
	}
}


void KappaCalculator::stasticpixel()
{
	int numof=0;

	if (vCountTurthExitValue.size()>0&&vCountCompdExitValue.size()>0)
	{
		vCountTurthExitValue.clear();
		vCountCompdExitValue.clear();
	}

	for (int ii=minmum;ii<=maxmum;ii++)
	{
		vCountTurthExitValue.push_back(0);
		vCountCompdExitValue.push_back(0);
	}

	bool isfindt;
	bool isfindc;

	// 统计总数量

	for (int ii=0;ii<mnNumofSam;ii++)
	{
		if (sampling_uturth[ii]!=0&&(sampling_uturth[ii]<=maxmum&&sampling_uturth[ii]>=minmum))
		{
			isfindt=false;
			isfindc=false;

			for (int jj=0;jj<vExitValue.size();jj++)
			{
				if(vExitValue[jj]==sampling_uturth[ii])
				{
					vCountTurthExitValue[jj]++;
					isfindt=true;
				}

				if(vExitValue[jj]==sampling_ucompd[ii])
				{
					vCountCompdExitValue[jj]++;
					isfindc=true;
				}

				if (isfindc==true&&isfindt==true)
				{
					break;
				}

			}
		}
	}


	for (int ii=0;ii<vCountTurthExitValue.size();ii++)
	{
		numof+=vCountTurthExitValue[ii];
	}

	int **pixelstatistic;
	pixelstatistic=new int* [vExitValue.size()];
	for (int ii=0; ii<vExitValue.size(); ii++)
	{
		pixelstatistic[ii] = new int[vExitValue.size()];
	}

	for (int ii=0;ii<vExitValue.size(); ii++)
	{
		for (int jj=0;jj<vExitValue.size(); jj++)
		{
			pixelstatistic[ii][jj]=0;
		}
	}

	for (int ii=0;ii<mnNumofSam;ii++)
	{
		if (sampling_uturth[ii]!=0)
		{
			if (sampling_uturth[ii]==sampling_ucompd[ii]||sampling_ucompd[ii]==0)
			{
				pixelstatistic[sampling_uturth[ii]-1][sampling_uturth[ii]-1]++;
			}
			else
			{
				pixelstatistic[sampling_ucompd[ii]-1][sampling_uturth[ii]-1]++;
			}
		}
	}

	delete[] sampling_uturth;
	delete[] sampling_ucompd;

	QFile file("./FilesGenerate/Kappa.csv");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	out<<"[Confusion Matrix]"<<endl;

	QString head(tr("Land use types"));
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		head+=",type"+QString::number(ii+1);
	}

	out<<head+",total"<<endl;

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			lineheader+=","+QString::number(pixelstatistic[ii][jj]);
			
		}

		lineheader+=","+QString::number(vCountCompdExitValue[ii]);

		out<<lineheader<<endl;
	}

	QString linetotal="total";

	double sum=0;
	for (int jj=0;jj<vExitValue.size();jj++)
	{
		linetotal+=","+QString::number(vCountTurthExitValue[jj]);
		sum+=vCountTurthExitValue[jj];
	}
	out<<linetotal+","+QString::number(sum,'l')<<endl;





	out<<endl<<"[Kappa Coefficient]"<<endl;
	QString lineheader="Kappa";
	double _sumDia=0;
	double _N=0;
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii==jj)
			{
				_sumDia+=pixelstatistic[ii][jj];
				_N+=vCountTurthExitValue[ii];
			}
		}
	}
	double _rowcolsum=0;
	double rowsum=0;
	double colsum=0;
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		rowsum=0;
		colsum=0;
		for (int jj=0;jj<vExitValue.size();jj++)
		{
			rowsum+=pixelstatistic[ii][jj];
			colsum+=pixelstatistic[jj][ii];
		}
		_rowcolsum+=rowsum*colsum;
	}

	double kappa;
	kappa=(_N*_sumDia-_rowcolsum)/(_N*_N-_rowcolsum);

	lineheader+=","+QString::number(kappa);
	out<<lineheader<<endl;



	out<<endl<<"[Overall Accuracy]"<<endl;
	QString header="Overall";
	double _sumConer=0;
	double _sumAll=0;
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii==jj)
			{
				_sumConer+=pixelstatistic[ii][jj];
				_sumAll+=vCountTurthExitValue[ii];
			}
		}
	}
	header+=","+QString::number((double)_sumConer/(double(_sumAll)));
	out<<header<<endl;


	out<<endl<<"[Commission Error]"<<endl;

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);
		double _sum=0;

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii!=jj)
			{
				_sum+=pixelstatistic[ii][jj];
			}
		}
		lineheader+=","+QString::number((double)_sum/(double(vCountCompdExitValue[ii])));
		out<<lineheader<<endl;
	}


	out<<endl<<"[Omission Error]"<<endl;

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);
		double _sum=0;

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii!=jj)
			{
				_sum+=pixelstatistic[jj][ii];
			}
		}
		lineheader+=","+QString::number((double)_sum/(double(vCountTurthExitValue[ii])));
		out<<lineheader<<endl;
	}


	out<<endl<<"[Producer's Accuracy]"<<endl;

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii==jj)
			{
				lineheader+=","+QString::number((double)pixelstatistic[jj][ii]/(double(vCountTurthExitValue[ii])));
			}
		}
		
		out<<lineheader<<endl;
	}

	out<<endl<<"[User's Accuracy]"<<endl;

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			if (ii==jj)
			{
				lineheader+=","+QString::number((double)pixelstatistic[jj][ii]/(double(vCountCompdExitValue[ii])));
			}
		}

		out<<lineheader<<endl;
	}


	file.close();


	//删除申请的内存  
	for(int i = 0; i < vExitValue.size(); i++)  
	{  
		delete []pixelstatistic[i];  
	}  
	delete [] pixelstatistic; 

}

template<class TT> unsigned char KappaCalculator::convert(QVector<TiffDataRead*> vImage,int ii)
{
	TT temp=*(TT*)(vImage[0]->imgData()+ii*sizeof(TT));

	if (temp==(TT)vImage[0]->poDataset()->GetRasterBand(1)->GetNoDataValue())
	{
		return (unsigned char)0;
	}

	return (unsigned char)temp;
}

unsigned char KappaCalculator::convert2uchar(QVector<TiffDataRead*> vImage,int ii)
{
	unsigned char uRestrictValue;

	switch(vImage.at(0)->datatype())
	{
	case GDT_Byte:
		uRestrictValue = convert<unsigned char>(vImage,ii);
		break;
	case GDT_UInt16:
		uRestrictValue = convert<unsigned short>(vImage,ii);
		break;
	case GDT_Int16:
		uRestrictValue = convert<short>(vImage,ii);
		break;
	case GDT_UInt32:
		uRestrictValue = convert<unsigned int>(vImage,ii);
		break;
	case GDT_Int32:
		uRestrictValue = convert<int>(vImage,ii);
		break;
	case GDT_Float32:
		uRestrictValue = convert<float>(vImage,ii);
		break;
	case GDT_Float64:
		uRestrictValue = convert<double>(vImage,ii);
		break;
	default:
		return 0;
	}

	return uRestrictValue;
}


// <巧妙的打乱>
struct Coor
{
	int nRow;
	int nCol;
};

void coorPushbackKappa(vector<Coor> &vecCoor,const int &_row,const int &_col)
{
	Coor coor;
	coor.nRow=_row;
	coor.nCol=_col;
	vecCoor.push_back(coor);
}

int randomFunction( int _lon )
{
	double dourp=(double)rand()/(double)RAND_MAX;
	int inrp=floor((_lon-1)*dourp);
	return inrp;
}

bool KappaCalculator::setRandomPoint()// <采样不到这么多怎么办？>
{
	bool _stra=false; // 采样策略

	srand((unsigned)time(NULL));

	size_t _nnumofAll=0;

	mnNumofSam=0;

	vector<int> stastisticTruth;

	if (vExitValue.size()>0)
	{
		vExitValue.clear();
	}

	for (int ii=minmum;ii<=maxmum;ii++)
	{
		vExitValue.push_back(ii);
		stastisticTruth.push_back(0);
	}

	bool isfindt;
	bool isfindc;

	// 统计总数量

	size_t ii,jj;
	for (ii=0;ii<itrows*itcols;ii++)
	{
		if (uturth[ii]!=0&&(uturth[ii]<=maxmum&&uturth[ii]>=minmum))
		{
			for (jj=0;jj<maxmum;jj++)
			{
				if (uturth[ii]==vExitValue[jj])
				{
					stastisticTruth[jj]++;
					break;
				}
			}
			_nnumofAll++;
		}
	}

	qSort(vExitValue.begin(), vExitValue.end());

	qSort(stastisticTruth.begin(), stastisticTruth.end());

	if (mdsamplingRate!=-1)
	{
		mnNumofSam=_nnumofAll*mdsamplingRate;
		_stra=true; // 比例采样
	}
	else
	{
		mnNumofSam=mnsamplingAmountAverage*vExitValue.size();
		_stra=false; // 全用均匀采样
	}
	

	if (mnNumofSam/vExitValue.size()>stastisticTruth[0]&&_stra==false)
	{
		QString status = QString("The sampling points is too much! %1").arg(mnNumofSam);

		return false;
	}

	m_PointCoodinateX=new int[mnNumofSam];
	m_PointCoodinateY=new int[mnNumofSam];

	double raPointX,raPointY;
	bool label=false;
	size_t i=0,j=0,xc,yc;
	int al_num=0,bl_num=0;
	QList<int> l_num;
	for (int ii=0;ii<vExitValue.size();ii++)
	{
		l_num.append(0);
	}



	if (_stra==true)
	{
		while (i<mnNumofSam)
		{
			xc=randomFunction(itrows);
			yc=randomFunction(itcols);
			label=true;
			if (uturth[xc*itcols+yc]<=maxmum&&uturth[xc*itcols+yc]>=minmum)
			{
				for (j=0;j<al_num;j++) /// <判断是否有相同坐标点生成>
				{
					if (m_PointCoodinateX[j]==xc&&m_PointCoodinateY[j]==yc)
					{
						label=false;
						break;
					}
				}
				if (label==true&&uturth[xc*itcols+yc]>0)
				{
					m_PointCoodinateX[i]=xc;
					m_PointCoodinateY[i]=yc;
					al_num=i;
					i++;
				}
			}
		}
		/// <统计采样点数>
		size_t i,_ii;
		for (i=0;i<mnNumofSam;i++)
		{
			/// <switch方法>
			for (_ii=0;_ii<vExitValue.size();_ii++)
			{
				if ((int)uturth[xc*itcols+yc]==vExitValue.at(_ii))
				{
					l_num[_ii]+=1;
					/// break;<保证扫描完毕，不能break>
				}
			}
		}
	}
	else
	{
		// <新等量采样>

		vector<Coor> coorSavior;

		int nType=vExitValue.size();

		size_t numofeach=ceil((double)(mnNumofSam*1.0/nType));

		int _label;

		size_t nCurrent=0;

		size_t kk,ii,jj;
		for (kk=0;kk<nType;kk++)
		{
			for (ii=0;ii<itrows;ii++)
			{
				for (jj=0;jj<itcols;jj++)
				{
					_label=vExitValue.at(kk);

					if (_label==uturth[ii*itcols+jj])
					{
						coorPushbackKappa(coorSavior,ii,jj);
					}
				}
			}

			random_shuffle(coorSavior.begin(), coorSavior.end());

			for(ii=0;ii<numofeach;ii++)
			{
				if (nCurrent<mnNumofSam)
				{
					m_PointCoodinateX[nCurrent]=coorSavior[ii].nRow;
					m_PointCoodinateY[nCurrent]=coorSavior[ii].nCol;
					nCurrent++;
				}
			}

			coorSavior.clear();

		}
	}

	sampling_uturth=new unsigned char[mnNumofSam];

	sampling_ucompd=new unsigned char[mnNumofSam];

	for (ii=0;ii<mnNumofSam;ii++)
	{
		sampling_uturth[ii]=uturth[m_PointCoodinateX[ii]*itcols+m_PointCoodinateY[ii]];
		sampling_ucompd[ii]=ucompd[m_PointCoodinateX[ii]*itcols+m_PointCoodinateY[ii]];
	}

	delete[] uturth;
	uturth=NULL;
	delete[] ucompd;
	ucompd=NULL;
	delete[] m_PointCoodinateX;
	m_PointCoodinateX=NULL;
	delete[] m_PointCoodinateY;
	m_PointCoodinateY=NULL;

	return true;
}


