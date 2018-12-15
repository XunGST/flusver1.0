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



NNtrain::NNtrain(QObject* parent)
{
	getAllParameter();

	nWidth=imgList.at(0)->cols();
	nHeight=imgList.at(0)->rows();
	nData=pathofdivingfactor.length();

	f_allBandData=NULL;
	d_allBandData=NULL;
	us_allBandData=NULL;
	m_PointCoodinateX=NULL;
	m_PointCoodinateY=NULL;
	saveMemF=NULL;
	saveMemD=NULL;
	saveMemUs=NULL;

}

bool NNtrain::getAllParameter()
{
	QFile file("./FilesGenerate/config_nodata.tmp");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return "false";
	file.readLine();
	while (!file.atEnd()) 
	{
		QString str=file.readLine();
		QStringList strlist=str.split(",");
		mvLanduseType.push_back(strlist[0].toUInt());
		mvCountType.push_back(strlist[1].toUInt());
	}
	file.close();
	remove("./FilesGenerate/config_nodata.tmp");

	QFile filem("./FilesGenerate/logFileTrain.log");
	if (!filem.open(QIODevice::ReadOnly | QIODevice::Text))
		return "false";
	int numofdf;
	int countRight=0;
	bool bRlt;
	while (!filem.atEnd()) 
	{
		QString str=filem.readLine().trimmed();
		if (str==tr("[NoData Value]").trimmed())
		{
			QString tmp=filem.readLine().trimmed();
			if (tmp==tr("No NoData Value").trimmed())
			{
				isNoDataExit=false;
			}
			else
			{
				isNoDataExit=true;
				noDataValue=tmp.toDouble();
			}
			countRight++;
		}
		if (str==tr("[Path of land use data]").trimmed())
		{
			pathoflanduse=filem.readLine();
			bRlt=imageOpenConver2uchar(pathoflanduse.toStdString().c_str());
			countRight++;
		}
		if (str==tr("[Path of saving data]").trimmed())
		{
			pathofsimresult=filem.readLine();
			countRight++;
		}
		if (str==tr("[Number of driving data]").trimmed())
		{
			QString numStr=filem.readLine();
			numofdf=numStr.toInt();
			countRight++;
		}
		if (str==tr("[Path of driving data]").trimmed())
		{
			for (int ii=0;ii<numofdf;ii++)
			{
				QString tmp=filem.readLine().trimmed();
				pathofdivingfactor.push_back(tmp);
				bRlt=imageOpen(pathofdivingfactor[pathofdivingfactor.length()-1].toStdString().c_str());
			}
			countRight++;
		}
		if (str==tr("[Data type]").trimmed())
		{
			datatype=filem.readLine().trimmed();
			countRight++;
		}
		if (str==tr("[Normalization type]").trimmed())
		{
			QString tmp=filem.readLine().trimmed();
			if (tmp==tr("Normalization").trimmed())
			{
				isNomalized=true;
			}
			else
			{
				isNomalized=false;
			}
			countRight++;
		}
		if (str==tr("[Sample type]").trimmed())
		{
			QString tmp=filem.readLine().trimmed();
			if (tmp==tr("Uniform Sampling").trimmed())
			{
				isUnifomSam=true;
			}
			else
			{
				isUnifomSam=false;
			}
			countRight++;
		}
		if (str==tr("[Percentage of Random Points]").trimmed())
		{
			QString numStr=filem.readLine();
			samplingRate=numStr.toDouble();
			countRight++;
		}
		if (str==tr("[Hidden layer]").trimmed())
		{
			QString numStr=filem.readLine();
			numHiddenLayer=numStr.toInt();
			countRight++;
		}
	}
	filem.close();

	if (countRight==10)
	{
		return true;
	}
	else
	{
		return false;
	}
}



bool NNtrain::imageOpen(QString filename)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	TiffDataRead* pread = new TiffDataRead;

	divingList.push_back(pread);

	string stdfilenamestr=filename.trimmed().toStdString(); 

	if (!divingList.at(divingList.length()-1)->loadFrom(stdfilenamestr.c_str()))
	{
		qDebug()<<"load error!"<<endl;
		return false;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	return true;
}

bool NNtrain::imageOpenConver2uchar(QString filename)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	TiffDataRead* pread = new TiffDataRead;

	imgList.push_back(pread);

	string stdfilenamestr=filename.trimmed().toStdString(); 

	if (!imgList.at(imgList.length()-1)->loadFrom(stdfilenamestr.c_str()))
	{
		qDebug()<<"load error!"<<endl;
		return false;
	}
	else
	{
		imgList.at(imgList.length()-1)->convert2uchar();
		cout<<"convert success!"<<endl;
	}

	return true;
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

	bandCount=0;

	for (int ii=0;ii<nData;ii++)
	{
		bandCount+=divingList.at(ii)->bandnum();

		QFileInfo fi = QFileInfo(pathofdivingfactor[ii]);

		if (divingList[ii]->bandnum()==1)
		{
			QString str = fi.fileName();  
			bandName.append(str);
		}
		else
		{
			QString str = fi.fileName(); 

			for (int kk=0;kk<divingList[ii]->bandnum();kk++)
			{  
				QString str1=str+"_band"+QString::number(kk+1);
				bandName.append(str1);
			}
		}
	}


	if (bandCount>0)
	{
		if (datatype.trimmed()==tr("Float").trimmed())
		{
			f_allBandData=new float[nWidth*nHeight*bandCount];
		}
		if (datatype.trimmed()==tr("Double").trimmed())
		{
			d_allBandData=new double[nWidth*nHeight*bandCount];
		}
		if (datatype.trimmed()==tr("Unsigned short").trimmed())
		{
			us_allBandData=new unsigned short[nWidth*nHeight*bandCount];
		}
			
		currentPos=0;
		//get data

		double* minmax1=new double[2];

		bool bRlt = false;
		for (int ii=0;ii<divingList.size();ii++)
		{
			divingList.at(ii)->loadData();
			currentBand=divingList.at(ii)->bandnum();

			for (int kk=0;kk<currentBand;kk++)
			{
				divingList.at(ii)->poDataset()->GetRasterBand(1+kk)->ComputeRasterMinMax(1,minmax1);

				float nodatavalue_factor=divingList[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
				double nodatavalue_factord=divingList[ii]->poDataset()->GetRasterBand(1)->GetNoDataValue();
				if (minmax1[0]==nodatavalue_factor||minmax1[0]==nodatavalue_factord)
				{
					minmax1[0]=0;
				}

				minmaxsave<<minmax1[0]<<minmax1[1]<<divingList.at(ii)->poDataset()->GetRasterBand(1+kk)->GetNoDataValue();
			}

			switch(divingList.at(ii)->datatype())
			{
				case GDT_Byte:
					bRlt = dataCopyConvert<unsigned char>(divingList.at(ii)->imgData());
					break;
				case GDT_UInt16:
					bRlt = dataCopyConvert<unsigned short>(divingList.at(ii)->imgData());
					break;
				case GDT_Int16:
					bRlt = dataCopyConvert<short>(divingList.at(ii)->imgData());
					break;
				case GDT_UInt32:
					bRlt = dataCopyConvert<unsigned int>(divingList.at(ii)->imgData());
					break;
				case GDT_Int32:
					bRlt = dataCopyConvert<int>(divingList.at(ii)->imgData());
					break;
				case GDT_Float32:
					bRlt = dataCopyConvert<float>(divingList.at(ii)->imgData());
					break;
				case GDT_Float64:
					bRlt = dataCopyConvert<double>(divingList.at(ii)->imgData());
					break;
				default:
					cout<<"CGDALRead::loadFrom : unknown data type!"<<endl;

			}

			currentPos+=nWidth*nHeight*currentBand;

			divingList.at(ii)->close();
		}

		delete[] minmax1;


		if (f_allBandData!=NULL||d_allBandData!=NULL||us_allBandData!=NULL)
		{
			nnTrain();
			if (datatype.trimmed()==tr("Float").trimmed())
			{
				if (f_allBandData!=NULL)
				{
					delete[] f_allBandData;
					f_allBandData=NULL;
				}
			}
			if (datatype.trimmed()==tr("Double").trimmed())
			{
				if (d_allBandData!=NULL)
				{
					delete[] d_allBandData;
					d_allBandData=NULL;
				}
			}
			if (datatype.trimmed()==tr("Unsigned short").trimmed())
			{
				if (us_allBandData!=NULL)
				{
					delete[] us_allBandData;
					us_allBandData=NULL;
				}
			}
		}
	}

	if (pathofsimresult.trimmed().size()>0&&imgList.size()>0&&divingList.size()>0&&(saveMemD!=NULL||saveMemF!=NULL||saveMemUs!=NULL))
	{
		if (datatype.trimmed()==tr("Float").trimmed())
		{
			size_t i,j,k;

			TiffDataWrite pwrite;
			bool brlt = pwrite.init(pathofsimresult.trimmed().toStdString().c_str(), imgList.at(0)->rows(), imgList.at(0)->cols(), mvLanduseType.size(), \
				imgList.at(0)->geotransform(), imgList.at(0)->projectionRef(), GDT_Float32, -1);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				sendParameter(tr("Save Error: ")+pathofsimresult.trimmed().toStdString().c_str());
				return ;
			}
			float _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (saveMemF!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							size_t datalen;

							datalen=k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j;

							_val = saveMemF[datalen];

							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] saveMemF;
				saveMemF=NULL;
			}
		}
		if (datatype.trimmed()==tr("Double").trimmed())
		{
			size_t i,j,k;
			/// <生成tiff文件>
			TiffDataWrite pwrite;
			bool brlt = pwrite.init(pathofsimresult.trimmed().toStdString().c_str(), imgList.at(0)->rows(), imgList.at(0)->cols(), mvLanduseType.size(), \
				imgList.at(0)->geotransform(), imgList.at(0)->projectionRef(), GDT_Float64, -1);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				return ;
			}
			double _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (saveMemD!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							size_t datalen;
							datalen=k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j;
							_val = saveMemD[datalen];
							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] saveMemD;
				saveMemD=NULL;
			}
		}
		if (datatype.trimmed()==tr("Unsigned short").trimmed())
		{
			size_t i,j,k;

			TiffDataWrite pwrite;
			bool brlt = pwrite.init(pathofsimresult.trimmed().toStdString().c_str(), imgList.at(0)->rows(), imgList.at(0)->cols(), mvLanduseType.size(), \
				imgList.at(0)->geotransform(), imgList.at(0)->projectionRef(), GDT_UInt16, 0);
			if (!brlt)
			{
				cout<<"write init error!"<<endl;
				return ;
			}
			unsigned short _val = 0;
			//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
			if (saveMemUs!=NULL)
			{
				for (i=0; i<pwrite.rows(); i++)
				{
					for (j=0; j<pwrite.cols(); j++)
					{
						for (k=0; k<pwrite.bandnum(); k++)
						{
							size_t datalen;
							datalen=k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j;
							_val = saveMemUs[datalen];
							pwrite.write(i, j, k, &_val);
						}
					}
				}
				cout<<"write success!"<<endl;
				pwrite.close();
				delete[] saveMemUs;
				saveMemD=NULL;
			}
		}
	}
	imgList.at(0)->close();
}

template<class TT> bool NNtrain::dataCopyConvert(unsigned char* buffer)
{
	size_t _sizeofTT;
	_sizeofTT=sizeof(TT);
	TT _temp;
	if (datatype.trimmed()==tr("Float").trimmed())
	{
		float data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			size_t ii;
			for (ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*_sizeofTT);
				data_temp=(float)_temp;
				f_allBandData[currentPos+ii]=data_temp;
			}
		}
	}
	if (datatype.trimmed()==tr("Double").trimmed())
	{
		size_t _sizeofTT;
		_sizeofTT=sizeof(TT);
		double data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			size_t ii;
			for (ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*_sizeofTT);
				data_temp=(double)_temp;
				d_allBandData[currentPos+ii]=data_temp;
			}
		}
	}
	if (datatype.trimmed()==tr("Unsigned short").trimmed())
	{
		size_t _sizeofTT;
		_sizeofTT=sizeof(TT);
		unsigned short data_temp;
		if (nWidth>0&&nHeight>0&&currentBand>0)
		{
			size_t ii;
			for (ii=0;ii<nWidth*nHeight*currentBand;ii++)
			{
				_temp=*(TT*)(buffer+ii*_sizeofTT);
				data_temp=(unsigned short)(_temp*65534+1);
				us_allBandData[currentPos+ii]=data_temp;
			}
		}
	}
	return true;
}


void NNtrain::nnTrain()
{
	if (isNomalized==true)
	{
		QString status = QString("Normalizing data, please wait...");
		sendParameter(status);
		normalizationdata();
	}
	else
	{
		QString status_s = QString("Set random points, please wait...");
		sendParameter(status_s);
	}
	

	if (isUnifomSam==true)
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

	real_2d_array trainarr;
	size_t jj;
	double _filter;

	trainarr.setlength(numof,(bandCount+1));
	if (datatype.trimmed()==tr("Float").trimmed())
	{
		size_t i;
		size_t j;
		size_t datalen;

		for (i=0;i<bandCount;i++)
		{
			jj=0;
			for (j=0;j<numof;j++)
			{
				datalen=i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j];

				double _filter=f_allBandData[datalen];

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
	if (datatype.trimmed()==tr("Double").trimmed())
	{
		size_t i;
		size_t j;
		size_t datalen;

		for (i=0;i<bandCount;i++)
		{
			jj=0;
			for (j=0;j<numof;j++)
			{
				datalen=i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j];

				_filter=d_allBandData[datalen];

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

	if (datatype.trimmed()==tr("Unsigned short").trimmed())
	{
		size_t i;
		size_t j;
		size_t datalen;

		for (i=0;i<bandCount;i++)
		{
			jj=0;
			for (j=0;j<numof;j++)
			{
				datalen=i*nWidth*nHeight+m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j];

				_filter=us_allBandData[datalen];

				trainarr[jj][i]=(((double)_filter)-1)/65534.0;

				jj++;
			}
		}
	}

	jj=0;
	size_t j;
	size_t datalen;

	for (j=0;j<numof;j++)
	{
		datalen=m_PointCoodinateX[j]*nWidth+m_PointCoodinateY[j];

		trainarr[jj][bandCount]=(double)imgList[0]->imgData()[datalen]-1;
		jj++;
	}


	delete[] m_PointCoodinateX;
	delete[] m_PointCoodinateY;

	ofstream file;
	file.open( "./FilesGenerate/NetworkInput.csv", ios::out );
	string line;

	for (int ii=0;ii<bandName.size();ii++)
	{
		QString str=bandName.at(ii);
		string _str;
		_str=str.toStdString();
		line=line+_str+", ";
	}
	file << line+"class" << endl;

	double* sa=new double[bandCount+1];

	size_t ii;
	for (ii=0;ii<numof;ii++)
	{
		for (int jj=0;jj<(bandCount+1);jj++)
		{
			sa[jj]=trainarr[ii][jj];
		}
		line=putCount2File(sa,(bandCount+1));
		file <<line.substr(0,line.length()-1)<< endl;
	}
	file.close();

	QString status = QString("Start training, please wait...");
	sendParameter(status);

	//initial net
	mlptrainer trn;
	multilayerperceptron network;
	mlpreport rep;

	//7 features,3 classes
	mlpcreatetrainercls(bandCount, mvLanduseType.size(), trn);
	//7 features, 3 classes, and 10 hidden
	mlpcreatec1(bandCount,numHiddenLayer, mvLanduseType.size(), network);
	//num5 training data
	mlpsetdataset(trn, trainarr, numof);



	//training 5 times
	double TimeStart=GetTickCount();
	mlptrainnetwork(trn, network, 1, rep);
	double TimeEnd=GetTickCount();
	double TimeUsed=(TimeEnd-TimeStart)/1000;
	//output
	QString status0 = QString("Run time: %1 s").arg(TimeUsed);
	sendParameter(status0);
// 	//training parameters
	QString status1 = QString("Precision evaluation: ");
	sendParameter(status1);
// 	
// 	QString status2 = QString("relclserror = %1").arg(rep.relclserror);
// 	sendParameter(status2);
// // 	
// 	QString status3 = QString("avgce = %1").arg(rep.avgce);
// 	sendParameter(status3);
// // 
	QString status4 = QString("RMSE = %1").arg(rep.rmserror);
	sendParameter(status4);
// // 
	QString status5 = QString("Average error = %1").arg(rep.avgerror);
	//sendParameter(status5);
// 
	QString status6 = QString("Average relative error = %1").arg(rep.avgrelerror);
	//sendParameter(status6);

// 	QString status7 = QString("ngrad =  %1").arg(rep.ngrad);
// 	sendParameter(status7);
// 
// 	QString status8 = QString("nhess =  %1").arg(rep.nhess);
// 	sendParameter(status8);
// 
// 	QString status9 = QString("ncholesky =  %1").arg(rep.ncholesky);
// 	sendParameter(status9);


	trainarr.setlength(0,0);

	QString status_F = QString("Waiting for prediction...");
	sendParameter(status_F);
	//predict classes
	real_1d_array prearr;
	prearr.setlength(bandCount);
	//output array
	real_1d_array dst;
	//3 bands picture
	isNoDataExit=true;
	if (datatype.trimmed()==tr("Float").trimmed())
	{
		size_t _a=mvLanduseType.size();
		saveMemF=new float[nWidth*nHeight*_a];

		size_t i;
		size_t j;
		size_t k;

		for (i=0;i<nHeight;i++)
		{
			for (j=0;j<nWidth;j++)
			{
				if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[i*nWidth+j]&&imgList[0]->imgData()[i*nWidth+j]!=0))
				{
					for (k=0;k<bandCount;k++)
					{
						prearr[k]=f_allBandData[k*nHeight*nWidth+i*nWidth+j];
					}
					mlpprocess(network, prearr, dst);
					size_t ii;
					for(ii=0;ii<dst.length();ii++)
					{
						saveMemF[nWidth*nHeight*ii+i*nWidth+j]=(float)dst[ii];
					}
				}
				else
				{
					size_t ii;
					for(ii=0;ii<mvLanduseType.size();ii++)
					{
						saveMemF[nWidth*nHeight*ii+i*nWidth+j]=-1;
					}
				}
			}
		}
	}
	if (datatype.trimmed()==tr("Double").trimmed())
	{
		size_t _a=mvLanduseType.size();
		saveMemD=new double[nWidth*nHeight*_a];

		size_t i;
		size_t j;
		size_t k;

		for (i=0;i<nHeight;i++)
		{
			for (j=0;j<nWidth;j++)
			{
				if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[i*nWidth+j]&&imgList[0]->imgData()[i*nWidth+j]!=0))
				{
					for (k=0;k<bandCount;k++)
					{
						prearr[k]=d_allBandData[k*nHeight*nWidth+i*nWidth+j];
					}
					mlpprocess(network, prearr, dst);

					size_t ii;
					for(ii=0;ii<dst.length();ii++)
					{
						saveMemD[nWidth*nHeight*ii+i*nWidth+j]=(double)dst[ii];
					}
				}
				else
				{
					size_t ii;
					for(ii=0;ii<mvLanduseType.size();ii++)
					{
						saveMemD[nWidth*nHeight*ii+i*nWidth+j]=-1;
					}
				}
			}
		}
	}
	if (datatype.trimmed()==tr("Unsigned short").trimmed())
	{
		size_t _a=mvLanduseType.size();
		saveMemUs=new unsigned short[nWidth*nHeight*_a];

		size_t i;
		size_t j;
		size_t k;

		for (i=0;i<nHeight;i++)
		{
			for (j=0;j<nWidth;j++)
			{
				if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[i*nWidth+j]&&imgList[0]->imgData()[i*nWidth+j]!=0))
				{
					for (k=0;k<bandCount;k++)
					{
						prearr[k]=((double)us_allBandData[k*nHeight*nWidth+i*nWidth+j]-1)/65534.0;
					}
					mlpprocess(network, prearr, dst);

					size_t ii;
					for(ii=0;ii<dst.length();ii++)
					{
						saveMemUs[nWidth*nHeight*ii+i*nWidth+j]=(unsigned short)(dst[ii]*65534+1);
					}
				}
				else
				{
					size_t ii;
					for(ii=0;ii<mvLanduseType.size();ii++)
					{
						saveMemUs[nWidth*nHeight*ii+i*nWidth+j]=0;
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

	size_t kk,ii;

	for (kk=0;kk<bandCount;kk++)
	{
		for (ii=0;ii<nWidth*nHeight;ii++)
		{
			tempNormData[ii]=f_allBandData[kk*nWidth*nHeight+ii];
		}
		
		QString fname;

		fname=QString::number(kk);

		fname=fname+".tif";

		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		char **papszMetadata = poDriver->GetMetadata();
		GDALDataset* poDataset2=poDriver->Create(fname.toStdString().c_str(),nWidth,nHeight,1,GDT_Float32,papszMetadata);

 		poDataset2->SetGeoTransform(imgList.at(0)->geotransform());
 		poDataset2->SetProjection(imgList.at(0)->projectionRef());
		poDataset2->GetRasterBand(1)->SetNoDataValue(-1);

		CPLErr err = poDataset2->RasterIO(GF_Write, 0, 0,nWidth,nHeight,tempNormData,nWidth,nHeight,GDT_Float32, 1, 0, 0, 0, 0);
		GDALClose(poDataset2);

		if (err==CE_None)
		{
			cout<<"Write data successed!"<<endl;

		}
	}
	delete[] tempNormData;
	tempNormData=NULL;

}

bool NNtrain::normalizationdata()
{
	size_t i, j, k=0;
	double temp;
	double max1;
	double min1;
	double nodata;

	if (datatype.trimmed()==tr("Float").trimmed())
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
					if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[j*nWidth+i]&&imgList[0]->imgData()[j*nWidth+i]!=0))
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

	if (datatype.trimmed()==tr("Double").trimmed())
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
					if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[j*nWidth+i]&&imgList[0]->imgData()[j*nWidth+i]!=0))
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

	if (datatype.trimmed()==tr("Unsigned short").trimmed())
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
					if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[j*nWidth+i]&&imgList[0]->imgData()[j*nWidth+i]!=0))
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


bool NNtrain::setRandomPoint(bool _stra)
{

	srand((unsigned)time(NULL));

	numof=0;

	for (int ii=0;ii<mvCountType.size();ii++)
	{
		numof+=mvCountType[ii];
	}

	qSort(mvCountType.begin(), mvCountType.end());

	numof=numof*samplingRate*0.001;

	if (numof/mvCountType.size()>mvCountType[0]&&_stra==false)
	{
		QString status = QString("The sampling points is too much! %1").arg(numof);
		sendParameter(status);
		return false;
	}
	m_PointCoodinateX=new int[numof];
	m_PointCoodinateY=new int[numof];
	double raPointX,raPointY;
	bool label=false;
	size_t i=0,j=0,xc,yc;
	int al_num=0,bl_num=0;
	QList<int> l_num;
	for (int ii=0;ii<mvLanduseType.size();ii++)
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
			if (isNoDataExit==false||(noDataValue!=imgList[0]->imgData()[xc*nWidth+yc]&&imgList[0]->imgData()[xc*nWidth+yc]!=0))
			{
				for (j=0;j<al_num;j++) 
				{
					if (m_PointCoodinateX[j]==xc&&m_PointCoodinateY[j]==yc)
					{
						label=false;
						break;
					}
				}
				if (label==true&&imgList[0]->imgData()[xc*nWidth+yc]>0)
				{
					m_PointCoodinateX[i]=xc;
					m_PointCoodinateY[i]=yc;
					al_num=i;
					i++;
				}
			}
		}

		for (i=0;i<numof;i++)
		{
			for (int _ii=0;_ii<mvLanduseType.size();_ii++)
			{
				if ((int)imgList[0]->imgData()[xc*nWidth+yc]==mvLanduseType.at(_ii))
				{
					l_num[_ii]+=1;
				}
			}
		}
	}
	else
	{
		vector<Coor> coorSavior;

		int nType=mvLanduseType.size();

		int numofeach=ceil((double)(numof*1.0/nType));

		int _label;

		size_t nCurrent=0;


		for (int kk=0;kk<nType;kk++)
		{
			size_t ii;
			for (ii=0;ii<nHeight;ii++)
			{
				size_t jj;
				for (jj=0;jj<nWidth;jj++)
				{
					_label=mvLanduseType.at(kk);

					if (_label==imgList[0]->imgData()[ii*nWidth+jj])
					{
						coorPushback(coorSavior,ii,jj);
					}
				}
			}

			random_shuffle(coorSavior.begin(), coorSavior.end());

			for(ii=0;ii<numofeach;ii++)
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
	}

	return true;
}

size_t NNtrain::randomFunction( size_t _lon )
{

	double dourp=(double)rand()/(double)RAND_MAX;

	size_t inrp=floor((_lon-1)*dourp);

	return inrp;
}

