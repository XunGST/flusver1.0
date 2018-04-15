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
	// 
	perofrp=m_gsl->ui.PerSpinBox->value();
	perofrp=perofrp*0.001;
	// 
	f_allBandData=NULL;
	d_allBandData=NULL;
	us_allBandData=NULL;
	m_PointCoodinateX=NULL;
	m_PointCoodinateY=NULL;
	m_gsl->p0=NULL;
	m_gsl->dp0=NULL;
	m_gsl->usSp0=NULL;
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

// 	/// <输出写入统计文本>
// 	ofstream file;
// 	file.open( "../statistic.txt", ios::out );
// 	string line;
// 
// 	double* sa=new double[bandCount+1];
// 
// 	for (int ii=0;ii<numof;ii++)
// 	{
// 		for (int jj=0;jj<(bandCount+1);jj++)
// 		{
// 			sa[jj]=trainarr[ii][jj];
// 		}
// 		line=putCount2File(sa,(bandCount+1));
// 		file << line << endl;
// 	}
// 	file.close();
}

NNtrain::~NNtrain()
{
	// string putCount2File(double* saveCount,int _length)
	// {
	// 	string s1;
	// 	string inval(" ");
	// 	stringstream out1;
	// 	for (int i=0;i<_length;i++)
	// 	{
	// 		out1<<setiosflags(ios::fixed)<<setprecision(8)<<saveCount[i]<<inval;
	// 		s1=out1.str();
	// 	}	
	// 	out1.clear();	
	// 	return s1;
	// 
	// // 	/// <输出写入统计文本>
	// // 	ofstream file;
	// // 	file.open( "../statistic.txt", ios::out );
	// // 	string line;
	// // 
	// // 	double* sa=new double[bandCount+1];
	// // 
	// // 	for (int ii=0;ii<numof;ii++)
	// // 	{
	// // 		for (int jj=0;jj<(bandCount+1);jj++)
	// // 		{
	// // 			sa[jj]=trainarr[ii][jj];
	// // 		}
	// // 		line=putCount2File(sa,(bandCount+1));
	// // 		file << line << endl;
	// // 	}
	// // 	file.close();
	// }
		//delete[] sa;

// 	/// <输出写入统计文本>
// 	char a[4]="abs";
// 	ofstream outfile("d://b//b.txt");
// 	outfile<<a;
// 	outfile<<msProjectionRef;
// 	outfile.flush();
// 	outfile.close();
// 	/// <输出写入统计文本>
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
		if (m_gsl->ui.radioDouble->isChecked()==true)
		{
			d_allBandData=new double[nWidth*nHeight*bandCount];
		}
		if (m_gsl->ui.radioMemory->isChecked()==true)
		{
			us_allBandData=new unsigned short[nWidth*nHeight*bandCount];
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

				// 防止GDAL的自带函数出错
				float nodatavalue_factor=m_gsl->div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
				double nodatavalue_factord=m_gsl->div_poDataset[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
				if (minmax1[0]==nodatavalue_factor||minmax1[0]==nodatavalue_factord)
				{
					minmax1[0]=0;
				}
				// 防止GDAL的自带函数出错

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


		if (f_allBandData!=NULL||d_allBandData!=NULL||us_allBandData!=NULL)
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
			if (m_gsl->ui.radioDouble->isChecked()==true)
			{
				if (d_allBandData!=NULL)
				{
					delete[] d_allBandData;
					d_allBandData=NULL;
				}
			}
			if (m_gsl->ui.radioMemory->isChecked()==true)
			{
				if (us_allBandData!=NULL)
				{
					delete[] us_allBandData;
					us_allBandData=NULL;
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
	if (m_gsl->ui.savePlineEdit->text().size()>0&&m_gsl->lau_poDataset.size()>0&&m_gsl->div_poDataset.size()>0&&(m_gsl->dp0!=NULL||m_gsl->p0!=NULL||m_gsl->usSp0!=NULL))
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
				sendParameter(tr("保存路径错误： ")+m_gsl->ui.savePlineEdit->text().toStdString().c_str());
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
		if (m_gsl->ui.radioDouble->isChecked()==true)
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
		if (m_gsl->ui.radioMemory->isChecked()==true)
		{
			int i,j,k;
			/// <生成tiff文件>
			TiffDataWrite pwrite;
			bool brlt = pwrite.init(m_gsl->ui.savePlineEdit->text().toStdString().c_str(), m_gsl->lau_poDataset.at(0)->rows(), m_gsl->lau_poDataset.at(0)->cols(), m_gsl->rgbLanduseType.size(), \
				m_gsl->lau_poDataset.at(0)->geotransform(), m_gsl->lau_poDataset.at(0)->projectionRef(), GDT_UInt16, 0);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				return ;
			}
			unsigned short _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (m_gsl->usSp0!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							_val = m_gsl->usSp0[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j];
							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] m_gsl->usSp0;
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
	if (m_gsl->ui.radioDouble->isChecked()==true)
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
	if (m_gsl->ui.radioMemory->isChecked()==true)
	{
		unsigned short data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			for (int ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*sizeof(TT));
				data_temp=(unsigned short)(_temp*65534+1);
				us_allBandData[currentPos+ii]=data_temp;
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
	QString status_s = QString("Set random points, please wait...");
	sendParameter(status_s);

	if (m_gsl->ui.raAve->isChecked()==true)
	{
		QString status_c = QString("Select uniform sampling...");
		sendParameter(status_c);
		setRandomPoint(false);
	}
	else
	{
		QString status_c = QString("Select sampling in proportion...");
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
	if (m_gsl->ui.radioDouble->isChecked()==true)
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

	if (m_gsl->ui.radioMemory->isChecked()==true)
	{
		for (int i=0;i<bandCount;i++)
		{
			jj=0;
			for (int j=0;j<numof;j++)
			{
				_filter=us_allBandData[i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j]];

				trainarr[jj][i]=(((double)_filter)-1)/65534.0;

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
	file.open( "NetworkInput.txt", ios::out );
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


	//7 features,3 classes
	mlpcreatetrainercls(bandCount, m_gsl->rgbLanduseType.size(), trn);
	//7 features, 3 classes, and 10 hidden
	mlpcreatec1(bandCount,hidLayCount, m_gsl->rgbLanduseType.size(), network);
	//num5 training data
	mlpsetdataset(trn, trainarr, numof);
	//training 5 times
	double TimeStart=GetTickCount();
	mlptrainnetwork(trn, network, runCount, rep);
	double TimeEnd=GetTickCount();
	double TimeUsed=(TimeEnd-TimeStart)/1000;
	//output
	QString status0 = QString("All training time:  %1 s").arg(TimeUsed);
	sendParameter(status0);
// 	//training parameters
	QString status1 = QString("Output NN parameters: ");
	sendParameter(status1);
// 	
	QString status2 = QString("relclserror = %1").arg(rep.relclserror);
	sendParameter(status2);
// // 	
	QString status3 = QString("avgce = %1").arg(rep.avgce);
	sendParameter(status3);
// // 
	QString status4 = QString("rmserror =  %1").arg(rep.rmserror);
	sendParameter(status4);
// // 
	QString status5 = QString("avgerror =   %1").arg(rep.avgerror);
	sendParameter(status5);
// 
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
	//predict classes
	real_1d_array prearr;
	prearr.setlength(bandCount);
	//output array
	real_1d_array dst;
	//3 bands picture
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
	if (m_gsl->ui.radioDouble->isChecked()==true)
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
	if (m_gsl->ui.radioMemory->isChecked()==true)
	{
		m_gsl->usSp0=new unsigned short[nWidth*nHeight*m_gsl->rgbLanduseType.size()];
		int mskdata;
		for (int i=0;i<nHeight;i++)
		{
			for (int j=0;j<nWidth;j++)
			{
				if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[i*nWidth+j]&&m_gsl->_landuse[i*nWidth+j]!=0))
				{
					for (int k=0;k<bandCount;k++)
					{
						prearr[k]=((double)us_allBandData[k*nHeight*nWidth+i*nWidth+j]-1)/65534.0;
					}
					mlpprocess(network, prearr, dst);
					for(int ii=0;ii<dst.length();ii++)
					{
						m_gsl->usSp0[nWidth*nHeight*ii+i*nWidth+j]=(unsigned short)(dst[ii]*65534+1);
					}
				}
				else
				{
					for(int ii=0;ii<m_gsl->rgbLanduseType.size();ii++)
					{
						m_gsl->usSp0[nWidth*nHeight*ii+i*nWidth+j]=0;
					}
				}
			}
		}
	}
}
/// <>
/// <存归一化图层>
/// <>
void NNtrain::saveNormalizedData()
{


	float* tempNormData=new float[nWidth*nHeight];
	for (int kk=0;kk<bandCount;kk++)
	{
		for (int ii=0;ii<nWidth*nHeight;ii++)
		{
			tempNormData[ii]=f_allBandData[kk*nWidth*nHeight+ii];
		}
		
		QString fname;

		fname=QString::number(kk);

		fname=fname+".tif";

		//写入过程
		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		char **papszMetadata = poDriver->GetMetadata();
		GDALDataset* poDataset2=poDriver->Create(fname.toStdString().c_str(),nWidth,nHeight,1,GDT_Float32,papszMetadata);

		//写入投影和坐标系
 		poDataset2->SetGeoTransform(m_gsl->lau_poDataset.at(0)->geotransform());
 		poDataset2->SetProjection(m_gsl->lau_poDataset.at(0)->projectionRef());
		poDataset2->GetRasterBand(1)->SetNoDataValue(-1);

		//写入数据
		CPLErr err = poDataset2->RasterIO(GF_Write, 0, 0,nWidth,nHeight,tempNormData,nWidth,nHeight,GDT_Float32, 1, 0, 0, 0, 0);
		GDALClose(poDataset2);

		if (err==CE_None)
		{
			cout<<"write data succed!"<<endl;

		}
	}
	delete[] tempNormData;
	tempNormData=NULL;

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
							temp=0;//in case of strange value
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

	if (m_gsl->ui.radioDouble->isChecked()==true)
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
							temp=0;//in case of strange value
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

	if (m_gsl->ui.radioMemory->isChecked()==true)
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
						temp=(us_allBandData[k*nWidth*nHeight+j*nWidth+i]-min1)/(max1-min1);
						if (us_allBandData[k*nWidth*nHeight+j*nWidth+i]==nodata)
						{
							temp=0;//in case of strange value
						}
						us_allBandData[k*nWidth*nHeight+j*nWidth+i]=(unsigned short)(temp*65534+1);

					}
					else
					{
						us_allBandData[k*nWidth*nHeight+j*nWidth+i]=0;
					}
				}
			}
		}
	}

	//saveNormalizedData();

	return true;
}


// <巧妙的打乱>
struct Coor
{
	int nRow;
	int nCol;
};

void coorPushback(vector<Coor> &vecCoor,const int &_row,const int &_col)
{
	Coor coor;
	coor.nRow=_row;
	coor.nCol=_col;
	vecCoor.push_back(coor);
}


bool NNtrain::setRandomPoint(bool _stra)// <采样不到这么多怎么办？>
{

	srand((unsigned)time(NULL));

	//numof=nHeight*nWidth*perofrp;

	numof=0;

	for (int ii=0;ii<m_gsl->staCount.size();ii++)
	{
		numof+=m_gsl->staCount[ii];
	}

	qSort(m_gsl->staCount.begin(), m_gsl->staCount.end());

	numof=numof*perofrp;

	if (numof/m_gsl->staCount.size()>m_gsl->staCount[0]&&_stra==false)
	{
		QString status = QString("The sampling points is too much! %1").arg(numof);
		//QMessageBox::warning(this,"Error",status);
		sendParameter(status);
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
				for (j=0;j<al_num;j++) /// <判断是否有相同坐标点生成>
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
		/// <统计采样点数>
		for (int i=0;i<numof;i++)
		{
			/// <switch方法>
			for (int _ii=0;_ii<m_gsl->rgbLanduseType.size();_ii++)
			{
				if ((int)m_gsl->_landuse[xc*nWidth+yc]==m_gsl->rgbLanduseType.at(_ii))
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

		int nType=m_gsl->rgbLanduseType.size();

		int numofeach=ceil((double)(numof*1.0/nType));

		int _label;

		int nCurrent=0;

		for (int kk=0;kk<nType;kk++)
		{
			for (int ii=0;ii<nHeight;ii++)
			{
				for (int jj=0;jj<nWidth;jj++)
				{
					_label=m_gsl->rgbLanduseType.at(kk);

					if (_label==m_gsl->_landuse[ii*nWidth+jj])
					{
						coorPushback(coorSavior,ii,jj);
					}
				}
			}

			random_shuffle(coorSavior.begin(), coorSavior.end());


			for(int ii=0;ii<numofeach;ii++)
			{
				if (nCurrent<numof)
				{
					m_PointCoodinateX[nCurrent]=coorSavior[ii].nRow;
					m_PointCoodinateY[nCurrent]=coorSavior[ii].nCol;
					nCurrent++;
				}
			}

			coorSavior.clear();

		}


		// <原等量采样>
// 		int num=ceil((double)(numof/m_gsl->rgbLanduseType.size()));
// 		int _temp,_label;
// 		while (i<numof)
// 		{
// 			xc=randomFunction(nHeight);
// 			yc=randomFunction(nWidth);
// 			label=true;
// 
// 			if (m_gsl->nodataexit==false||(m_gsl->nodatavalue!=m_gsl->_landuse[xc*nWidth+yc]&&m_gsl->_landuse[xc*nWidth+yc]!=0))
// 			{
// 				for (j=0;j<al_num;j++) /// <判断是否有相同坐标点生成>
// 				{
// 					if (m_PointCoodinateX[j]==xc&&m_PointCoodinateY[j]==yc)
// 					{
// 						label=false;
// 						break;
// 					}
// 				}
// 				if (label==true)
// 				{
// 					_temp=(int)m_gsl->_landuse[xc*nWidth+yc];
// 					for (int jj=0;jj<m_gsl->rgbLanduseType.size();jj++)
// 					{
// 						_label=m_gsl->rgbLanduseType.at(jj);
// 						if (_temp==_label&&l_num[jj]<=num)
// 						{
// 							m_PointCoodinateX[i]=xc;
// 							m_PointCoodinateY[i]=yc;
// 							al_num=i;
// 							i++;
// 							l_num[jj]++;
// 							break;
// 						}
// 					}
// 				}
// 			}
// 		}
	}

	return true;
}

int NNtrain::randomFunction( int _lon )
{

	double dourp=(double)rand()/(double)RAND_MAX;

	int inrp=floor((_lon-1)*dourp);
	return inrp;
}

