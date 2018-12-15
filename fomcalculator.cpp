#include "fomcalculator.h"
#include <QMessageBox>
#include <QtCore>
#include <vector>
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

FoMcalculator::FoMcalculator(QObject *parent)
	: QObject(parent)
{

}

FoMcalculator::~FoMcalculator()
{

}

FoMcalculator::FoMcalculator(QObject *parent,vector<fomReader>& _image,int _mode)
{
	imageSerial=_image;
	mode=_mode;
}

bool FoMcalculator::reRoadImages()
{
	if (imageSerial.size()<3)
	{
		return false;
	}
	if (imageSerial[0]._pread->rows()-imageSerial[1]._pread->rows()!=0||imageSerial[1]._pread->rows()-imageSerial[2]._pread->rows()!=0)
	{
		return false;
	}
	if (imageSerial[0]._pread->cols()-imageSerial[1]._pread->cols()!=0||imageSerial[1]._pread->cols()-imageSerial[2]._pread->cols()!=0)
	{
		return false;
	}
	nodata=imageSerial[0]._pread->poDataset()->GetRasterBand(1)->GetNoDataValue();
	//maxmum=imageSerial[0]._pread->poDataset()->GetRasterBand(1)->GetMaximum();
	//minmum=imageSerial[0]._pread->poDataset()->GetRasterBand(1)->GetMinimum();

	double minmax[2];
	imageSerial[0]._pread->poDataset()->GetRasterBand(1)->ComputeRasterMinMax(false,minmax);
	minmum=minmax[0];
	maxmum=minmax[1];

	itrows=imageSerial[0]._pread->rows();
	itcols=imageSerial[0]._pread->cols();

	if (nodata==maxmum||nodata==minmum)
	{
		return false;
	}

	for (int ii=0;ii<imageSerial.size();ii++)
	{
		unsigned char* _tmp=new unsigned char[(size_t)itrows*itcols];
		string path;
		path=imageSerial[ii]._path;
		if (!imageSerial[ii]._pread->loadFrom(path.c_str()))
		{
			cout<<"load error!"<<endl;
		}
		else
		{
			cout<<"load success!"<<endl;
		}
		size_t jj;
		for (jj=0;jj<itrows*itcols;jj++)
		{
			_tmp[jj]=convert2uchar(imageSerial[ii]._pread,jj);
		}
		convertImage.push_back(_tmp);
		//imageSerial[ii]._pread->close();
	}
	return true;
}

bool FoMcalculator::measureFoM()
{
	// 统计总数量

	float A =0;

	float B =0;

	float C =0;

	float D =0;

	float denominator=0;

	size_t ii;
	for (ii=0;ii<(size_t)itrows*itcols;ii++)
	{
		if (convertImage[0][ii]!=0&&(convertImage[0][ii]<=maxmum&&convertImage[0][ii]>=minmum))
		{
			if (convertImage[1][ii]==convertImage[2][ii]&&convertImage[1][ii]==convertImage[0][ii])
			{
				continue;
			}
			else
			{
				if (convertImage[0][ii]==convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii])
				{
					A++;
				}
				if (convertImage[1][ii]==convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii])
				{
					B++;
				}
				if (convertImage[1][ii]!=convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii]&&convertImage[0][ii]!=convertImage[2][ii])
				{
					C++;
				}
				if (convertImage[1][ii]==convertImage[0][ii]&&convertImage[2][ii]!=convertImage[0][ii])
				{
					D++;
				}
			}
		}
	}

	float FoM=B/(A+B+C+D);

	QFile file("./FilesGenerate/FoM.csv");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);

	out<<"A=";
	QString header=","+QString::number((int)A);
	out<<header<<endl;

	out<<"B=";
	QString header1=","+QString::number((int)B);
	out<<header1<<endl;

	out<<"C=";
	QString header2=","+QString::number((int)C);
	out<<header2<<endl;

	out<<"D=";
	QString header3=","+QString::number((int)D);
	out<<header3<<endl;
	
	out<<"[Figure of Merit]=B/(A+B+C+D)"<<endl;
	QString header4="FoM=,"+QString::number(B/(A+B+C+D));
	out<<header4<<endl;

	out<<"[Producer's Accuracy]=B/(A+B+C)"<<endl;
	QString header5="Producer's Accuracy=, "+QString::number(B/(A+B+C));
	out<<header5<<endl;

	out<<"[User's Accuracy]=B/(B+C+D)"<<endl;
	QString header6="User's Accuracy=,"+QString::number(B/(B+C+D));
	out<<header6<<endl;

	file.close();

	for (int ii=0;ii<convertImage.size();ii++)
	{
		delete[] convertImage[ii];
		 convertImage[ii]=NULL;
	}

	return true;

}

template<class TT> unsigned char FoMcalculator::convert(TiffDataRead* tImage,int ii)
{
	TT temp=*(TT*)(tImage->imgData()+ii*sizeof(TT));

	if (temp==(TT)tImage->poDataset()->GetRasterBand(1)->GetNoDataValue())
	{
		return (unsigned char)0;
	}

	return (unsigned char)temp;
}

unsigned char FoMcalculator::convert2uchar(TiffDataRead* tImage,int ii)
{
	unsigned char uRestrictValue;

	switch(tImage->datatype())
	{
	case GDT_Byte:
		uRestrictValue = convert<unsigned char>(tImage,ii);
		break;
	case GDT_UInt16:
		uRestrictValue = convert<unsigned short>(tImage,ii);
		break;
	case GDT_Int16:
		uRestrictValue = convert<short>(tImage,ii);
		break;
	case GDT_UInt32:
		uRestrictValue = convert<unsigned int>(tImage,ii);
		break;
	case GDT_Int32:
		uRestrictValue = convert<int>(tImage,ii);
		break;
	case GDT_Float32:
		uRestrictValue = convert<float>(tImage,ii);
		break;
	case GDT_Float64:
		uRestrictValue = convert<double>(tImage,ii);
		break;
	default:
		return 0;
	}

	return uRestrictValue;
}


void FoMcalculator::calculateFoMprocess()
{
	reRoadImages();
	if (this->mode==1)
	{
		measureFoM();
	}
	else
	{
		measureKappaChangePart();
	}
	
}

bool FoMcalculator::measureKappaChangePart()
{
	// 统计总数量

//	float A =0;
//
//	float B =0;
//
//	float C =0;
//
//	float D =0;
//
//	float denominator=0;
//
//	size_t ii;
//	for (ii=0;ii<(size_t)itrows*itcols;ii++)
//	{
//		if (convertImage[0][ii]!=0&&(convertImage[0][ii]<=maxmum&&convertImage[0][ii]>=minmum))
//		{
//			if (convertImage[1][ii]==convertImage[2][ii]&&convertImage[1][ii]==convertImage[0][ii])
//			{
//				continue;
//			}
//			else
//			{
//				if (convertImage[0][ii]==convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii])
//				{
//					A++;  //M
//				}
//				if (convertImage[1][ii]==convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii])
//				{
//					B++; // H
//				}
//				if (convertImage[1][ii]!=convertImage[2][ii]&&convertImage[1][ii]!=convertImage[0][ii])
//				{
//					C++; //?
//				}
//				if (convertImage[1][ii]==convertImage[0][ii]&&convertImage[2][ii]!=convertImage[0][ii])
//				{
//					D++; //F
//				}
//			}
//		}
//	}
//
//
//
//	float FoM=B/(A+B+D);
//
//	QFile file("FoM(ESTOQUE).csv");
//	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//		return false;
//
//	QTextStream out(&file);
//
//	out<<"H=";
//	QString header1=","+QString::number((int)B);
//	out<<header1<<endl;
//
//	out<<"M=";
//	QString header=","+QString::number((int)A);
//	out<<header<<endl;
//
//
//// 	out<<"C=";
//// 	QString header2=","+QString::number((int)C);
//// 	out<<header2<<endl;
//
//	out<<"F=";
//	QString header3=","+QString::number((int)D);
//	out<<header3<<endl;
//
//	out<<"[Figure of Merit]=H/(H+M+F)"<<endl;
//	QString header4="FoM=,"+QString::number(B/(A+B+D));
//	out<<header4<<endl;
//
//	out<<"[HOC]=H/(H+M)"<<endl;
//	QString header5="=, "+QString::number(B/(A+B));
//	out<<header5<<endl;
//
//	out<<"[MOC]=M/(H+M)"<<endl;
//	QString header6="=,"+QString::number(A/(B+A));
//	out<<header6<<endl;
//
//	out<<"[FOC]=F/(H+M)"<<endl;
//	QString header7="=,"+QString::number(D/(B+A));
//	out<<header7<<endl;
//
//	file.close();
//
//	for (int ii=0;ii<convertImage.size();ii++)
//	{
//		delete[] convertImage[ii];
//		convertImage[ii]=NULL;
//	}

	vector<int> vCountTurthExitValue;
	vector<int> vCountCompdExitValue;
	vector<int> vExitValue;

	for (int ii=minmum;ii<=maxmum;ii++)
	{
		vCountTurthExitValue.push_back(0);
		vCountCompdExitValue.push_back(0);
		vExitValue.push_back(ii);
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

	int mnNumofSam=0;
	bool isfindt;
	bool isfindc;

	for (size_t ii=0;ii<(size_t)itrows*itcols;ii++)
	{
		if (convertImage[0][ii]!=0&&(convertImage[0][ii]<=maxmum&&convertImage[0][ii]>=minmum))
		{
			if (convertImage[1][ii]==convertImage[2][ii]&&convertImage[1][ii]==convertImage[0][ii])
			{
				continue;
			}
			else
			{
				isfindt=false;
				isfindc=false;

				for (int jj=0;jj<vExitValue.size();jj++)
				{
					if(vExitValue[jj]==convertImage[1][ii])
					{
						vCountTurthExitValue[jj]++;
						isfindt=true;
					}

					if(vExitValue[jj]==convertImage[2][ii])
					{
						vCountCompdExitValue[jj]++;
						isfindc=true;
					}

					if (isfindc==true&&isfindt==true)
					{
						break;
					}

				}

				if (convertImage[1][ii]==convertImage[2][ii]||convertImage[2][ii]==0)
				{
					pixelstatistic[convertImage[1][ii]-1][convertImage[1][ii]-1]++;
				}
				else
				{
					pixelstatistic[convertImage[2][ii]-1][convertImage[1][ii]-1]++;
				}

			}
		}
	}



	QFile file("Kappa_changepart.csv");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

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

	return true;
}

