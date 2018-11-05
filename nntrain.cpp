#include "nntrain.h"
#include "geosimulator.h"
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include "geodpcasys.h"
#include <QMessageBox>
#include <iostream>
#include "src/statistics.h"
#include "src/dataanalysis.h"
#include "src/alglibmisc.h"
#include "src/LinAlg.h"
#include <time.h>
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace alglib;


NNtrain::NNtrain(GeoSimulator* _gsl )
{
	m_gsl=_gsl;
	nData=m_gsl->div_poDataset.size();
	nWidth=m_gsl->div_poDataset.at(0)->cols();
	nHeight=m_gsl->div_poDataset.at(0)->rows();
	mskcon=false;

#ifdef _DEMO_TEMP
	if(nWidth>1000||nHeight>800||nData>8)
	{
		QMessageBox::about(NULL, "DEMO", "DEMO can not support so big data.");
		m_gsl->close();
	}
#endif

	// 
	perofrp=m_gsl->ui.PerSpinBox->value();
	perofrp=perofrp*0.001;
	// 
	f_allBandData=NULL;
	d_allBandData=NULL;
	m_PointCoodinateX=NULL;
	m_PointCoodinateY=NULL;
	m_gsl->p0=NULL;
	m_gsl->dp0=NULL;
}

string putCount2File(double* saveCount,int _length)
{
	string s1;
	string inval(",");
	stringstream out1;
	for (int i=0;i<_length;i++)
	{
		out1<<setiosflags(ios::fixed)<<setprecision(8)<<saveCount[i]<<inval;
		s1=out1.str();
	}	
	out1.clear();	
	return s1;
}

NNtrain::~NNtrain()
{

}

void NNtrain::trainprocess()
{

	if (m_gsl->lau_poDataset.at(0)->imgData()!=NULL)
	{
		m_gsl->lau_poDataset.at(0)->deleteImgData();
	}
	
	bandCount=0;

	for (int ii=0;ii<nData;ii++)
	{
		bandCount+=m_gsl->div_poDataset.at(ii)->bandnum();
	}

	if (bandCount>0)
	{
		if (m_gsl->ui.radioSingle->isChecked()==true)
		{
			f_allBandData=new float[nWidth*nHeight*bandCount];
		}
		else
		{
			d_allBandData=new double[nWidth*nHeight*bandCount];
		}
			
		currentPos=0;
		//get data

		double* minmax1=new double[2];

		bool bRlt = false;
		for (int ii=0;ii<m_gsl->div_poDataset.size();ii++)
		{
			m_gsl->div_poDataset.at(ii)->loadData();
			currentBand=m_gsl->div_poDataset.at(ii)->bandnum();

			for (int kk=0;kk<currentBand;kk++)
			{
				m_gsl->div_poDataset.at(ii)->poDataset()->GetRasterBand(1+kk)->ComputeRasterMinMax(1,minmax1);
				minmaxsave<<minmax1[0]<<minmax1[1]<<m_gsl->div_poDataset.at(ii)->poDataset()->GetRasterBand(1+kk)->GetNoDataValue();
			}

			switch(m_gsl->div_poDataset.at(ii)->datatype())
			{
				case GDT_Byte:
					bRlt = dataCopyConvert<unsigned char>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_UInt16:
					bRlt = dataCopyConvert<unsigned short>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_Int16:
					bRlt = dataCopyConvert<short>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_UInt32:
					bRlt = dataCopyConvert<unsigned int>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_Int32:
					bRlt = dataCopyConvert<int>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_Float32:
					bRlt = dataCopyConvert<float>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				case GDT_Float64:
					bRlt = dataCopyConvert<double>(m_gsl->div_poDataset.at(ii)->imgData());
					break;
				default:
					cout<<"CGDALRead::loadFrom : unknown data type!"<<endl;

			}
			currentPos+=nWidth*nHeight*currentBand;
			m_gsl->div_poDataset.at(ii)->close();
		}

		delete[] minmax1;


		if (f_allBandData!=NULL||d_allBandData!=NULL)
		{
			nnTrain();
			if (m_gsl->ui.radioSingle->isChecked()==true)
			{
				if (f_allBandData!=NULL)
				{
					delete[] f_allBandData;
					f_allBandData=NULL;
				}
			}
			else
			{
				if (d_allBandData!=NULL)
				{
					delete[] d_allBandData;
					d_allBandData=NULL;
				}
			}
		}
	}
	if (m_gsl->_landuse!=NULL)
	{
		delete[] m_gsl->_landuse;
		m_gsl->_landuse=NULL;
	}
	/// <保存影像>
	if (m_gsl->ui.savePlineEdit->text().size()>0&&m_gsl->lau_poDataset.size()>0&&m_gsl->div_poDataset.size()>0&&(m_gsl->dp0!=NULL||m_gsl->p0!=NULL))
	{
		if (m_gsl->ui.radioSingle->isChecked()==true)
		{
			int i,j,k;
			/// <生成tiff文件>
			TiffDataWrite pwrite;
			bool brlt = pwrite.init(m_gsl->ui.savePlineEdit->text().toStdString().c_str(), m_gsl->lau_poDataset.at(0)->rows(), m_gsl->lau_poDataset.at(0)->cols(), m_gsl->rgbLanduseType.size(), \
				m_gsl->lau_poDataset.at(0)->geotransform(), m_gsl->lau_poDataset.at(0)->projectionRef(), GDT_Float32, -1);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				return ;
			}
			float _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (m_gsl->p0!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							_val = m_gsl->p0[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j];
							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] m_gsl->p0;
				m_gsl->p0=NULL;
			}
		}
		else
		{
			int i,j,k;
			/// <生成tiff文件>
			TiffDataWrite pwrite;
			bool brlt = pwrite.init(m_gsl->ui.savePlineEdit->text().toStdString().c_str(), m_gsl->lau_poDataset.at(0)->rows(), m_gsl->lau_poDataset.at(0)->cols(), m_gsl->rgbLanduseType.size(), \
				m_gsl->lau_poDataset.at(0)->geotransform(), m_gsl->lau_poDataset.at(0)->projectionRef(), GDT_Float64, -1);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				return ;
			}
			double _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (m_gsl->dp0!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							_val = m_gsl->dp0[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j];
							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] m_gsl->dp0;
				m_gsl->dp0=NULL;
			}
		}
	}
	m_gsl->lau_poDataset.at(0)->close();
}

template<class TT> bool NNtrain::dataCopyConvert(unsigned char* buffer)
{
	TT _temp;
	if (m_gsl->ui.radioSingle->isChecked()==true)
	{
		float data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			for (int ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*sizeof(TT));
				data_temp=(float)_temp;
				f_allBandData[currentPos+ii]=data_temp;
			}
		}
	}
	else
	{
		double data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			for (int ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*sizeof(TT));
				data_temp=(double)_temp;
				d_allBandData[currentPos+ii]=data_temp;
			}
		}
	}
	return true;
}


void NNtrain::nnTrain()
{
	if (m_gsl->ui.divNorrabtnyes->isChecked()==true)
	{
		QString status = QString("Normalizing data, please wait...");
		sendParameter(status);
		normalizationdata();
	}
	QString status_s = QString("Set randomPoint, please wait...");
	sendParameter(status_s);

	if (m_gsl->ui.raAve->isChecked()==true)
	{
		QString status_c = QString("Select uniform sampling...");
		sendParameter(status_c);
		setRandomPoint(false);
	}
	else
	{
		QString status_c = QString("Select Sampling in Proportion...");
		sendParameter(status_c);
		setRandomPoint(true);
	}

	//-------------------alglib array-----------------------
	real_2d_array trainarr;
	int jj;
	double _filter;// <新的改动>

	trainarr.setlength(numof,(bandCount+1));
	if (m_gsl->ui.radioSingle->isChecked()==true)
	{
		for (int i=0;i<bandCount;i++)
		{
			jj=0;
			for (int j=0;j<numof;j++)
			{
				double _filter=f_allBandData[i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j]];
				if (_filter<0||_filter>1)
				{
					trainarr[jj][i]=0;
				}
				else
				{
					trainarr[jj][i]=_filter;
				}
				jj++;
			}
		}
	}
	else
	{
		for (int i=0;i<bandCount;i++)
		{
			jj=0;
			for (int j=0;j<numof;j++)
			{
				_filter=d_allBandData[i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j]];
				if (_filter<0||_filter>1)
				{
					trainarr[jj][i]=0;
				}
				else
				{
					trainarr[jj][i]=_filter;
				}
				jj++;
			}
		}
	}
	jj=0;
	for (int j=0;j<numof;j++)
	{
		trainarr[jj][bandCount]=(double)m_gsl->_landuse[m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j]]-1;
		jj++;
	}


	delete[] m_PointCoodinateX;
	delete[] m_PointCoodinateY;

	/// <输出写入统计文本>
	ofstream file;
	file.open( "ANNinput.txt", ios::out );
	string line;

	for (int ii=0;ii<m_gsl->div_poDataset.size();ii++)
	{
		string _str=m_gsl->divMetaTable->item(ii,0)->text().toStdString();
		line=line+_str+", ";
	}
	file << line+"class" << endl;

	double* sa=new double[bandCount+1];

	for (int ii=0;ii<numof;ii++)
	{
		for (int jj=0;jj<(bandCount+1);jj++)
		{
			sa[jj]=trainarr[ii][jj];
		}
		line=putCount2File(sa,(bandCount+1));
		file <<line.substr(0,line.length()-1)<< endl;
	}
	file.close();

	//----------------------------------------build NN-------------------------------------------
	int hidLayCount=m_gsl->ui.HidSpinBox->value();
	int runCount=m_gsl->ui.TTspinBox->value();

	QString status = QString("Start training, please wait...");
	sendParameter(status);

	//initial net
	mlptrainer trn;
	multilayerperceptron network;
	mlpreport rep;


	mlpcreatetrainercls(bandCount, m_gsl->rgbLanduseType.size(), trn);
	mlpcreatec1(bandCount,hidLayCount, m_gsl->rgbLanduseType.size(), network);
	mlpsetdataset(trn, trainarr, numof);
	double TimeStart=GetTickCount();
	mlptrainnetwork(trn, network, runCount, rep);
	double TimeEnd=GetTickCount();
	double TimeUsed=(TimeEnd-TimeStart)/1000;

	QString status0 = QString("All training time:  %1 s").arg(TimeUsed);
	sendParameter(status0);

	QString status1 = QString("Output NN parameters: ");
	sendParameter(status1);

	QString status2 = QString("relclserror = %1").arg(rep.relclserror);
	sendParameter(status2);

	QString status3 = QString("avgce = %1").arg(rep.avgce);
	sendParameter(status3);

	QString status4 = QString("rmserror =  %1").arg(rep.rmserror);
	sendParameter(status4);

	QString status5 = QString("avgerror =   %1").arg(rep.avgerror);
	sendParameter(status5);
 
	QString status6 = QString("avgrelerror = %1").arg(rep.avgrelerror);
	sendParameter(status6);

	QString status7 = QString("ngrad =  %1").arg(rep.ngrad);
	sendParameter(status7);

	QString status8 = QString("nhess =  %1").arg(rep.nhess);
	sendParameter(status8);

	QString status9 = QString("ncholesky =  %1").arg(rep.ncholesky);
	sendParameter(status9);


	trainarr.setlength(0,0);

	QString status_F = QString("Waiting for prediction...");
	sendParameter(status_F);
	real_1d_array prearr;
	prearr.setlength(bandCount);
	real_1d_array dst;

	m_gsl->nodataexit=true;
	if (m_gsl->ui.radioSingle->isChecked()==true)
	{
		int _a=m_gsl->rgbLanduseType.size();
		m_gsl->p0=new float[nWidth*nHeight*_a];
		for (int i=0;i<nHeight;i++)
		{
			for (int j=0;j<nWidth;j++)
			{
				if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[i*nWidth+j]&&m_gsl->_landuse[i*nWidth+j]!=0))
				{
					for (int k=0;k<bandCount;k++)
					{
						prearr[k]=f_allBandData[k*nHeight*nWidth+i*nWidth+j];
					}
					mlpprocess(network, prearr, dst);
					for(int ii=0;ii<dst.length();ii++)
					{
						m_gsl->p0[nWidth*nHeight*ii+i*nWidth+j]=(float)dst[ii];
					}
				}
				else
				{
					for(int ii=0;ii<m_gsl->rgbLanduseType.size();ii++)
					{
						m_gsl->p0[nWidth*nHeight*ii+i*nWidth+j]=-1;
					}
				}
			}
		}
	}
	else
	{
		m_gsl->dp0=new double[nWidth*nHeight*m_gsl->rgbLanduseType.size()];
		int mskdata;
		for (int i=0;i<nHeight;i++)
		{
			for (int j=0;j<nWidth;j++)
			{
				if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[i*nWidth+j]&&m_gsl->_landuse[i*nWidth+j]!=0))
				{
					for (int k=0;k<bandCount;k++)
					{
						prearr[k]=d_allBandData[k*nHeight*nWidth+i*nWidth+j];
					}
					mlpprocess(network, prearr, dst);
					for(int ii=0;ii<dst.length();ii++)
					{
						m_gsl->dp0[nWidth*nHeight*ii+i*nWidth+j]=(double)dst[ii];
					}
				}
				else
				{
					for(int ii=0;ii<m_gsl->rgbLanduseType.size();ii++)
					{
						m_gsl->dp0[nWidth*nHeight*ii+i*nWidth+j]=-1;
					}
				}
			}
		}
	}
}

bool NNtrain::normalizationdata()
{
	int i, j, k=0;
	double temp;
	double max1;
	double min1;
	double nodata;
	int mskdata;

	if (m_gsl->ui.radioSingle->isChecked()==true)
	{
		for (k=0; k<bandCount; k++)
		{
			min1=minmaxsave[k*3+0];
			max1=minmaxsave[k*3+1];
			nodata=minmaxsave[k*3+2];

			for (i=0; i<nWidth; i++)
			{
				for (j=0; j<nHeight; j++)
				{	
					if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[j*nWidth+i]&&m_gsl->_landuse[j*nWidth+i]!=0))
					{
						temp=(f_allBandData[k*nWidth*nHeight+j*nWidth+i]-min1)/(max1-min1);
						if (f_allBandData[k*nWidth*nHeight+j*nWidth+i]==nodata)
						{
							temp=0;
						}
						f_allBandData[k*nWidth*nHeight+j*nWidth+i]=temp;
					}
					else
					{
						f_allBandData[k*nWidth*nHeight+j*nWidth+i]=-1;
					}
				}
			}
		}
	}
	else
	{
		for (k=0; k<bandCount; k++)
		{
			min1=minmaxsave[k*3+0];
			max1=minmaxsave[k*3+1];
			nodata=minmaxsave[k*3+2];

			for (i=0; i<nWidth; i++)
			{
				for (j=0; j<nHeight; j++)
				{	
					if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[j*nWidth+i]&&m_gsl->_landuse[j*nWidth+i]!=0))
					{
						temp=(d_allBandData[k*nWidth*nHeight+j*nWidth+i]-min1)/(max1-min1);
						if (d_allBandData[k*nWidth*nHeight+j*nWidth+i]==nodata)
						{
							temp=0;
						}
						d_allBandData[k*nWidth*nHeight+j*nWidth+i]=temp;

					}
					else
					{
						d_allBandData[k*nWidth*nHeight+j*nWidth+i]=-1;
					}
				}
			}
		}
	}
	return true;
}

bool NNtrain::setRandomPoint(bool _stra)
{
	numof=nHeight*nWidth*perofrp;
	qSort(m_gsl->staCount.begin(), m_gsl->staCount.end());
	if (numof/m_gsl->staCount.size()>m_gsl->staCount[0]&&_stra==false)
	{
		QString status = QString("The sampling points is too much! 1%").arg(numof);
		QMessageBox::warning(this,"Error",status);
		return false;
	}
	m_PointCoodinateX=new int[numof];
	m_PointCoodinateY=new int[numof];
	double raPointX,raPointY;
	bool label=false;
	int i=0,j=0,xc,yc,al_num=0,bl_num=0;
	int mskdata;
	QList<int> l_num;
	for (int ii=0;ii<m_gsl->rgbLanduseType.size();ii++)
	{
		l_num.append(0);
	}
	

	if (_stra==true)
	{
		while (i<numof)
		{
			xc=randomFunction(nHeight);
			yc=randomFunction(nWidth);
			label=true;
			if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[xc*nWidth+yc]&&m_gsl->_landuse[xc*nWidth+yc]!=0))
			{
				for (j=0;j<al_num;j++) 
				{
					if (m_PointCoodinateX[j]==xc&&m_PointCoodinateY[j]==yc)
					{
						label=false;
						break;
					}
				}
				if (label==true&&m_gsl->_landuse[xc*nWidth+yc]>0)
				{
					m_PointCoodinateX[i]=xc;
					m_PointCoodinateY[i]=yc;
					al_num=i;
					i++;
				}
			}
		}

		for (int i=0;i<numof;i++)
		{
			for (int _ii=0;_ii<m_gsl->rgbLanduseType.size();_ii++)
			{
				if ((int)m_gsl->_landuse[xc*nWidth+yc]==m_gsl->rgbLanduseType.at(_ii))
				{
					l_num[_ii]+=1;
				}
			}
		}
	}
	else
	{
		int num=ceil((double)(numof/m_gsl->rgbLanduseType.size()));
		int _temp,_label;
		while (i<numof)
		{
			xc=randomFunction(nHeight);
			yc=randomFunction(nWidth);
			label=true;

			if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[xc*nWidth+yc]&&m_gsl->_landuse[xc*nWidth+yc]!=0))
			{
				for (j=0;j<al_num;j++) 
				{
					if (m_PointCoodinateX[j]==xc&&m_PointCoodinateY[j]==yc)
					{
						label=false;
						break;
					}
				}
				if (label==true)
				{
					_temp=(int)m_gsl->_landuse[xc*nWidth+yc];
					for (int jj=0;jj<m_gsl->rgbLanduseType.size();jj++)
					{
						_label=m_gsl->rgbLanduseType.at(jj);
						if (_temp==_label&&l_num[jj]<=num)
						{
							m_PointCoodinateX[i]=xc;
							m_PointCoodinateY[i]=yc;
							al_num=i;
							i++;
							l_num[jj]++;
							break;
						}
					}
				}
			}
		}
	}

	return true;
}

int NNtrain::randomFunction( int _lon )
{
	double dourp=(double)rand()/(double)RAND_MAX;
	int inrp=floor((_lon-1)*dourp);
	return inrp;
}

